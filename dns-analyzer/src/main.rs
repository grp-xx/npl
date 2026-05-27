use chrono::Local;
use clap::Parser;
use pcap::{Capture, Device};
use simple_dns::{QTYPE, TYPE};
use std::collections::HashMap;
use std::time::Instant;
use std::io::Write;

struct DnsQuery {
    qname: String,
    qtype: String,
}

#[derive(Parser, Debug)]
#[command(name = "DNS Analyzer")]
#[command(author = "Your Name")]
#[command(version = "1.0")]
#[command(about = "Analyzes DNS queries in live or offline mode", long_about = None)]
struct Args {
    /// Interface for live packet capture (conflicts with --pcap)
    #[arg(short, long, conflicts_with = "pcap_file")]
    interface: Option<String>,

    /// PCAP file for offline analysis (conflicts with --interface)
    #[arg(short = 'f', long, conflicts_with = "interface")]
    pcap_file: Option<String>,

    /// Interval (in seconds) _between stats output
    #[arg(short = 'n', long, default_value = "5")]
    interval: u64,
}

fn main() {
    let args = Args::parse(); // uses `clap` to fill the struct

    println!("Interface: {:?}", args.interface);
    println!("pcap file: {:?}", args.pcap_file);
    println!("Interval: {:?}", args.interval);

    if let Some(interface) = args.interface {
        println!("[*] Reading live packets from interface: {}", interface);
        start_live_capture(interface, args.interval);
    } else if let Some(pcap_file) = args.pcap_file {
        println!("[*] Reading packets from file: {}", pcap_file);
        start_offline_analysis(pcap_file);
    } else {
        eprintln!("Please specify either an interface or a pcap file");
    }
}

// Function to start live packet capture
fn start_live_capture(interface: String, interval: u64) {
    let device = Device::list()
        .unwrap()
        .into_iter()
        .find(|d| d.name == interface)
        .expect("Device not found");

    let mut cap = Capture::from_device(device)
        .unwrap()
        .promisc(true)
        .immediate_mode(true)
        .snaplen(65535) // Capture full packets
        .open()
        .unwrap();

    cap.filter("udp dst port 53", true).unwrap();

    let (tx, rx) = std::sync::mpsc::channel::<DnsQuery>();

    let mut now = Instant::now();

    let _ = std::thread::spawn(move || {
        let mut types: HashMap<String, u64> = HashMap::new();
        let mut names: HashMap<String, u64> = HashMap::new();
        for q in rx {
            //            println! {"Flow_Id: {} --- {}",f.ips, f.ipd};
            *types.entry(q.qtype).or_insert(0) += 1;
            *names.entry(q.qname).or_insert(0) += 1;
            if now.elapsed() > std::time::Duration::from_secs(interval) {
                //    if now.elapsed().as_secs() > 5 {
                let mut vt: Vec<_> = types.iter().collect();
                let mut vn: Vec<_> = names.iter().collect();
                vt.sort_by(|a, b| b.1.cmp(a.1));
                vn.sort_by(|a, b| b.1.cmp(a.1));
                print!("\x1b[2J\x1b[H"); // Clear the terminal
                println!("=== DNS Stats at {} ===", Local::now());
                println!("");
                println!("Query Type Statistics:");
                for i in vt.iter() {
                    println!("{}:   {}", i.0, i.1);
                }
                println!("");
                println!("Top 10 DNS Names:");
                for i in vn.iter().take(10) {
                    println!("{}:   {}", i.0, i.1);
                }
                print!("\x1b[?25l"); // hide cursor}
                now = Instant::now();
            }
        }
    });

    loop {
        match cap.next_packet() {
            Ok(packet) => {
                let ipl = &packet.data[14] & 0x0F; // Get the IP header length;
                let dns_start = 14 + (ipl as usize * 4) + 8; // Calculate the start of the DNS section
                if let Ok(parsed_packet) = simple_dns::Packet::parse(&packet.data[dns_start..]) {
                    // println!("Captured packet: {:?}", parsed_packet);
                    for question in parsed_packet.questions {
                        // println!("Question: {:?}", question);
                        let qtype = match question.qtype {
                            QTYPE::TYPE(TYPE::A) => String::from("A"),
                            QTYPE::TYPE(TYPE::CNAME) => String::from("CNAME"),
                            QTYPE::TYPE(TYPE::MX) => String::from("MX"),
                            QTYPE::TYPE(TYPE::NS) => String::from("NS"),
                            _ => String::from("Other"),
                        };
                        // println!("Type: {}", qtype);
                        // println!("Name: {}", question.qname);
                        let query = DnsQuery {
                            qname: question.qname.to_string(),
                            qtype: qtype,
                        };
                        tx.send(query).unwrap();
                    }
                } else {
                    eprintln!("Failed to parse packet data as DNS");
                }
            }
            Err(e) => eprintln!("Error reading packet: {}", e),
        }
    }
}

fn start_offline_analysis(pcap_file: String) {
    let mut cap = Capture::from_file(pcap_file).expect("Failed to open pcap file");
    cap.filter("udp dst port 53", true)
        .expect("Failed to set filter");

    let mut types: HashMap<String, u64> = HashMap::new();
    let mut names: HashMap<String, u64> = HashMap::new();

    while let Ok(packet) = cap.next_packet() {
        let ipl = &packet.data[14] & 0x0F; // Get the IP header length;
        let dns_start = 14 + (ipl as usize * 4) + 8; // Calculate the start of the DNS section
        if let Ok(parsed_packet) = simple_dns::Packet::parse(&packet.data[dns_start..]) {
            for question in parsed_packet.questions {
                let qtype = match question.qtype {
                    QTYPE::TYPE(TYPE::A) => String::from("A"),
                    QTYPE::TYPE(TYPE::CNAME) => String::from("CNAME"),
                    QTYPE::TYPE(TYPE::MX) => String::from("MX"),
                    QTYPE::TYPE(TYPE::NS) => String::from("NS"),
                    _ => String::from("Other"),
                };
                // println!("Type: {}", qtype);
                // println!("Name: {}", question.qname);
                let query = DnsQuery {
                    qname: question.qname.to_string(),
                    qtype: qtype,
                };
                *types.entry(query.qtype).or_insert(0) += 1;
                *names.entry(query.qname).or_insert(0) += 1;
            }
        } else {
            eprintln!("Failed to parse packet data as DNS");
        }
    }

    let mut vt: Vec<_> = types.iter().collect();
    let mut vn: Vec<_> = names.iter().collect();
    vt.sort_by(|a, b| b.1.cmp(a.1));
    vn.sort_by(|a, b| b.1.cmp(a.1));
    std::io::stdout().flush().unwrap(); // Flush the output to ensure it appears immediately
    println!("=== DNS Stats at {} ===", Local::now());
    println!("");
    println!("Query Type Statistics:");
    for i in vt.iter() {
        println!("{}:   {}", i.0, i.1);
    }
    println!("");
    println!("Top 10 DNS Names:");
    for i in vn.iter().take(10) {
        println!("{}:   {}", i.0, i.1);
    }
    println!("");
    println!("");
    println!("");
}
