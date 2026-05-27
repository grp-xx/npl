use clap::{ArgGroup, Parser};
use etherparse::IpNumber;
use utils::net::FlowIdV4;
use std::collections::HashMap;
use std::net::SocketAddrV4;
use std::path::PathBuf;
use std::io::Write;
use utils::log::verbose_log;


#[derive(Parser, Debug)]
#[command(group(
    ArgGroup::new("input")
        .required(true)
        .args(["interface", "file"])
))]
struct Cli {
    /// Network interface to read from
    #[arg(short = 'i', long)]
    interface: Option<String>,

    /// File to read from
    #[arg(short = 'f', long)]
    file: Option<PathBuf>,
    
    /// Verbose mode (-v, -vv, -vvv, etc.)
    #[arg(short, long, action = clap::ArgAction::Count)]
    verbose: u8,
}

fn main() {

    let cli = Cli::parse();

    match (cli.interface, cli.file) {
        (Some(interface), None) => {
//            verbose_log(
//                cli.verbose,
//                1,
//                format!("Reading from network interface: {}", interface),
//            );
            live_capture(cli.verbose, &interface);
        }
        (None, Some(file)) => {
//            verbose_log(
//                cli.verbose,
//                1,
//                format!("Reading from file: {}", file.display()),
//            );
            read_from_file(cli.verbose, &file);
        }
        _ => unreachable!(), // This should never happen due to the ArgGroup
    } 



}


fn live_capture(verbose: u8,interface: &str) {
    // Placeholder for live capture logic
    verbose_log(verbose, 2, format!("Starting live capture on interface: {}", interface));
    let device = pcap::Device::list()
        .expect("Failed to list devices")
        .into_iter()
        .find(|d| d.name == interface)
        .expect("Device not found");
    let mut cap = pcap::Capture::from_device(device)
        .expect("Failed to create capture from device")
        .promisc(true)
        .immediate_mode(true)
        .snaplen(64) // Capture full packets
        .open()
        .expect("Failed to open capture");
    let _ = cap.filter("ip and (tcp or udp)",true);

    let (tx, rx) = std::sync::mpsc::channel();
        std::thread::spawn(move || {
            verbose_log(verbose, 3, "Started Counting thread".to_string());
            let mut now = std::time::Instant::now();
            let mut flow_count: HashMap<FlowIdV4, (u64,u64)> = HashMap::new();
            for (fid, bytes) in rx {
                let entry = flow_count.entry(fid).or_insert((0,0));
                entry.0 += 1;
                entry.1 += bytes; // Placeholder for byte count
            
                let elapsed = now.elapsed();
                if elapsed > std::time::Duration::from_secs(5) {
                    print_stats(&flow_count);
                    now = std::time::Instant::now(); // Reset the timer
                }
            }
        });

    while let Ok(frame) = cap.next_packet() {
        if let Some(fid) = extract_flow_id(&frame.data[14..]) {
            verbose_log(verbose, 3, format!("Extracted Flow ID: {}", fid));
            tx.send((fid, frame.header.len as u64)).expect("Failed to send flow ID"); 
        }
    }
}


fn read_from_file(verbose: u8,file: &PathBuf) {
    // Placeholder for file reading logic
    verbose_log(verbose, 2, format!("Starting to read from file: {}", file.display()));
    let mut cap = pcap::Capture::from_file(file).expect("Failed to open pcap file");
    let _ = cap.filter("ip and (tcp or udp)",true);
    let mut flow_count: HashMap<FlowIdV4, (u64,u64)> = HashMap::new();
    while let Ok(frame) = cap.next_packet() {
        let fid = extract_flow_id(&frame.data[14..]);
        if let Some(x) = fid {
            verbose_log(verbose, 3, format!("Extracted Flow ID: {}", x));

            let entry = flow_count.entry(x).or_insert((0,0));
            entry.0 += 1;
            entry.1 += frame.header.len as u64;
        }
    }
    print_stats(&flow_count);
}



fn extract_flow_id(data: &[u8]) -> Option<FlowIdV4> {
    let iphdr = etherparse::Ipv4HeaderSlice::from_slice(data).ok()?;
    let ipl  = (iphdr.ihl() * 4) as usize;
    match iphdr.protocol() {
        IpNumber::TCP => {
            let tcphdr = etherparse::TcpHeaderSlice::from_slice(&data[ipl..]).ok()?;
            let s1 = SocketAddrV4::new(iphdr.source_addr(), tcphdr.source_port());
            let s2 = SocketAddrV4::new(iphdr.destination_addr(), tcphdr.destination_port());
            Some(FlowIdV4::new(&s1, &s2, iphdr.protocol().into()))
        },
        IpNumber::UDP => {
            let udphdr = etherparse::UdpHeaderSlice::from_slice(&data[ipl..]).ok()?;
            let s1 = SocketAddrV4::new(iphdr.source_addr(), udphdr.source_port());
            let s2 = SocketAddrV4::new(iphdr.destination_addr(), udphdr.destination_port());
            Some(FlowIdV4::new(&s1, &s2, iphdr.protocol().into()))
        },
        _ => None, // Skip non-TCP/UDP packets
    }
}

fn print_stats(stat_map: &HashMap<FlowIdV4, (u64,u64)>) {
    let mut v: Vec<_> = stat_map.iter().collect();
    v.sort_by(|&x, &y| ((y.1).1).cmp(&(x.1).1));
    
    std::io::stdout().flush().unwrap(); // Flush the output to ensure it appears immediately
    print!("\x1b[2J\x1b[H"); // Clear the terminal
    print!("\x1b[?25l"); // hide cursor}
    println!("=== Top 10 Flows ===");
    for i in v.iter().take(10) {
        println!("{} - Packets: {}, Bytes: {}", i.0, (i.1).0, (i.1).1);
    }
}
