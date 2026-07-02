use clap::Parser;
use ipnet::Ipv4Net;
use pcap::{Capture};
use utils::{log::verbose_log, log::bold, net::get_ipv4_addr_from_udp_socket};
use std::{
    collections::HashMap,
    net::{Ipv4Addr, SocketAddrV4, UdpSocket},
    sync::{Arc, Mutex},
    thread,
    time::{Duration, Instant},
};


const DEFAULT_PORT: u16 = 54321; // Default port for UDP probes

#[derive(Parser, Debug)]
#[command(name = "host-discovery")]
#[command(about = "Simple LAN host discovery via UDP probes + ICMP Port Unreachable")]
struct Args {
    /// Subnet to scan, example:
    #[arg(short, long)]
    net: Ipv4Net,

    /// Device to sniff for ICMP packets, ex: eth0, wlan0, en0
    #[arg(short, long)]
    iface: String,

    /// UDP port likely closed on hosts, default: 54321
    #[arg(short, long, default_value_t = DEFAULT_PORT)]
    port: u16,

    /// Capture timeout in seconds
    #[arg(short, long, default_value_t = 5)]
    timeout_s: u64,

    /// Delay between UDP probes in microseconds
    #[arg(long, default_value_t = 100)]
    delay_us: u64,

    /// Reverse DNS optional
    #[arg(long, default_value_t = false)]
    rdns: bool,
    
    /// Verbose mode (-v, -vv, -vvv, etc.)
    #[arg(short, long, action = clap::ArgAction::Count)]
    verbose: u8,
}

struct HostEntry {
    _sent: bool,
    received: bool,
    _ip4addr: Ipv4Addr,
    mac: Option<[u8; 6]>,
    name: Option<String>,
}

struct Addresses {
    ip4addr: Ipv4Addr,
    mac: [u8; 6],
}

fn main() {
    let cli = Args::parse();

    verbose_log(cli.verbose, 0, format!("Scanning subnet: {}", cli.net));

    let targets: Vec<Ipv4Addr> = cli.net.hosts().collect();
    
    let missing_targets = Arc::new(Mutex::new(targets.len()));
    
    let mut hosts:  HashMap<Ipv4Addr, HostEntry> = HashMap::new();

    let sock = match UdpSocket::bind("0.0.0.0:0") {
        Ok(s) => s,
        Err(e) => {
            eprintln!("Failed to bind socket: {}", e);
            return;
        }
    };




    // Start RX thread to listen for ICMP responses
    let sock_clone = sock.try_clone().expect("Failed to clone socket");
    let (tx, rx) = std::sync::mpsc::channel::<Addresses>();
    let missing_targets_clone = Arc::clone(&missing_targets);
    let rx_thread = thread::spawn(move || {
        let mut cap = match Capture::from_device(cli.iface.as_str()) {
            Ok(dev) => dev.promisc(true).timeout(cli.timeout_s as i32).snaplen(65535).open().unwrap(),
            Err(e) => {
                eprintln!("Failed to open capture device {}: {}", cli.iface, e);
                return;
            }
        };

        let sending_ip = get_ipv4_addr_from_udp_socket(&sock_clone).unwrap();
        verbose_log(cli.verbose, 2, format!("Listening for ICMP responses on interface {}", cli.iface));
        verbose_log(cli.verbose, 2, format!("Sending probes from IP {}", sending_ip));
        cap.filter(&format!("ip and dst host {} and icmp", sending_ip), true).unwrap();
        let mut cap = cap.setnonblock().unwrap();

        let start_time = Instant::now();

        loop {
            if *missing_targets.lock().unwrap() == 0 {
                drop(tx);
                break;
            } 
            if start_time.elapsed() > Duration::from_secs(cli.timeout_s) {
                drop(tx);
                break;
            }
            match cap.next_packet() {
                Ok(frame) => {
                    let src_mac = frame.data[6..12].to_owned();
                    let _dst_mac = frame.data[0..6].to_owned();
                    let ipl = frame.data[14] & 0x0F; // IP header length in 32-bit words
                    let ip_src_addr = Ipv4Addr::new(frame.data[26], frame.data[27], frame.data[28], frame.data[29]);
                    let icmp_offset = 14 + (ipl as usize * 4); // Offset to the ICMP header
                    let icmp_type = frame.data[icmp_offset];
                    let icmp_code = frame.data[icmp_offset + 1];
                    verbose_log(cli.verbose, 2, format!("ICMP Type: {}, Code: {}", icmp_type, icmp_code));
                    if icmp_type == 3 && icmp_code == 3 { // Port Unreachable 
                        verbose_log(cli.verbose, 3, format!("ICMP Response for IP {}", ip_src_addr));      
                        let addr = Addresses {
                            ip4addr: ip_src_addr,
                            mac: [src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5]],
                        };
                        *missing_targets.lock().unwrap() -= 1; // Decrement the count of IP targets
                        tx.send(addr).expect("Failed to send address through channel");
                    } else {
                        verbose_log(cli.verbose, 3, format!("No matching ICMP response to port"));
                    }
                },
                Err(_) => {
                    continue;
                }
            }
        }



    });




    
    for ip in targets {
        std::thread::sleep(Duration::from_micros(cli.delay_us));
        match sock.send_to(&[0u8; 1], SocketAddrV4::new(ip, cli.port)) {
            Ok(_) => {
                hosts.insert(ip, HostEntry {
                    _sent: true,
                    received: false,
                    _ip4addr: ip,
                    mac: None,
                    name: None,
                });
            }
            Err(e) => {
                eprintln!("Failed to send probe to {}: {}", ip, e);
                *missing_targets_clone.lock().unwrap() -= 1; // Decrement the count of IP targets

            },
        }
    }

    println!("-------------------------------------------------------------------------------------------------------------------");
    for addr in rx {
        if let Some(host) = hosts.get_mut(&addr.ip4addr) {
            host.received = true;
            host.mac = Some(addr.mac);
            if cli.rdns {
                match dns_lookup::lookup_addr(&std::net::IpAddr::from(addr.ip4addr)) {
                    Ok(name) => {
                        host.name = Some(name);
                    }
                    Err(_) => {
                        host.name = None;
                    }
                }
            }
        }
        let mac = format!(
            "{:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}",
            addr.mac[0],
            addr.mac[1],
            addr.mac[2],
            addr.mac[3],
            addr.mac[4],
            addr.mac[5]
        );

        if cli.rdns {
            let name = hosts
                .get(&addr.ip4addr)
                .and_then(|h| h.name.clone())
                .unwrap_or_else(|| "Unknown".to_string());
            println!("{}\t{}\t{}\t{}\t{}\t{}", bold("Host: "), addr.ip4addr, bold("MAC: "), mac, bold("Name: "), name);
        } else {
            println!("{}\t{}\t{}\t{}", bold("Host: "), addr.ip4addr, bold("MAC: "), mac);
        }

    } 

    rx_thread.join().expect("RX thread panicked");

}

