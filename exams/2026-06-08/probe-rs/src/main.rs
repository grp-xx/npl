use clap::Parser;
use std::{collections::HashMap, net::UdpSocket, time::Duration};
use utils::{log::verbose_log, net::hostname_to_ipv4, pcap::packet_timestamp_to_duration};

const PORT: u16 = 33434; // Default port for traceroute-like probes

struct Probe {
    id: u16,
    departure: Duration,
    arrival: Option<Duration>,
    rtt: Option<Duration>,
}


#[derive(Parser, Debug)]
struct Cli {
    /// Target host to probe
//    #[arg(long)]
    host: String,

    /// Number of probes to send
    #[arg(short = 'n', long, default_value_t = 5)]
    numprobes: u16,
    
    /// Network interface to read from
    #[arg(short = 'i', long)]
    interface: Option<String>,

    /// Verbose mode (-v, -vv, -vvv, etc.)
    #[arg(short, long, action = clap::ArgAction::Count)]
    verbose: u8,
}
fn main() {
    let cli = Cli::parse();
    let sock = match UdpSocket::bind("0.0.0.0:0") {
        Ok(s) => s,
        Err(e) => {
            eprintln!("Failed to bind socket: {}", e);
            return;
        }
    };

    let ip4s = match hostname_to_ipv4(&cli.host) {
        Ok(ips) => ips,
        Err(e) => {
            eprintln!("Failed to resolve hostname {}: {}", cli.host, e);
            return;
        }
    };
    for ip in ip4s.iter() {
        verbose_log(cli.verbose, 2, format!("Resolved {} to {}", cli.host, ip));
    }

    let host = cli.host.clone();
    let ip = ip4s[0]; // Use the first resolved IP address for probing


    let ip = ip.clone();
    let rx_thread = std::thread::spawn(move || {
        // Here you would implement the logic to listen for ICMP responses and print them out.
        // This is just a placeholder to show where that logic would go.
//        std::thread::sleep(std::time::Duration::from_millis(500)); // Sleep briefly to allow probes to be sent before starting to listen
        let mut cap = pcap::Capture::from_device(cli.interface.as_deref().unwrap_or("any"))
            .unwrap()
            .promisc(true)
//            .immediate_mode(true)
            .timeout(1000)
            .snaplen(65535)
            .open()
            .unwrap();

        let program = format!("(dst host {} and udp and dst portrange {}-{}) or (icmp and src host {})", ip, PORT, PORT+cli.numprobes-1,ip);
        cap.filter(&program, true).unwrap();
        let mut cap = cap.setnonblock().unwrap();

        let start_time = std::time::Instant::now();
        let timeout = Duration::from_secs(cli.numprobes as u64 * 2); // Set a timeout based on the number of probes sent

        let mut probes_map: HashMap<u16, Probe> = HashMap::new(); // This will store the probes and their departure times
        let mut responses = 0;
        loop {
                if responses >= cli.numprobes {
                    verbose_log(cli.verbose, 2, "Received all expected responses, terminating".to_string());
                    break;
                }
                if start_time.elapsed() > timeout {
                    verbose_log(cli.verbose, 2, "Timeout expires, terminating".to_string());
                    break;
                }
                match cap.next_packet() {
                    Ok(packet) => {
                        verbose_log(cli.verbose, 2, format!("Received packet: {:?}", packet));
                        match packet.data[23] {
                            17 => {
                                let ipl = packet.data[14] & 0x0F; // IP header length in 32-bit words
                                let port_offset = 14 + (ipl as usize * 4) + 2; // Calculate the offset to the UDP header
                                let dst_port = u16::from_be_bytes([packet.data[port_offset], packet.data[port_offset + 1]]);
                                verbose_log(cli.verbose, 1, format!("UDP Probe Detected to port {}", dst_port));
                                let probe = Probe {
                                    id: dst_port - PORT,
                                    departure: packet_timestamp_to_duration(&packet.header),
                                    arrival: None,
                                    rtt: None, // You would calculate this based on departure and arrival times
                                };
                                probes_map.insert(dst_port, probe);
                            },
                            1 => {
                                responses += 1; 
                                verbose_log(cli.verbose, 1, format!("ICMP Detected"));
                                // Handle ICMP response, extract the original destination port from the embedded IP header and match it to the probe
                                let ipl = packet.data[14] & 0x0F; // IP header length in 32-bit words
                                let icmp_offset = 14 + (ipl as usize * 4); // Calculate the offset to the ICMP header
                                let icmp_type = packet.data[icmp_offset];
                                let icmp_code = packet.data[icmp_offset + 1];
                                verbose_log(cli.verbose, 2, format!("ICMP Type: {}, Code: {}", icmp_type, icmp_code));
                                if icmp_type == 3 && icmp_code == 3 { // Port Unreachable 
                                    let embedded_ipl = packet.data[icmp_offset + 8] & 0x0F; // IP header length of the embedded packet
                                    let embedded_port_offset = icmp_offset + 8 + (embedded_ipl as usize * 4) + 2; // Calculate the offset to the UDP header in the embedded packet
                                    let embedded_dst_port = u16::from_be_bytes([packet.data[embedded_port_offset], packet.data[embedded_port_offset + 1]]);
                                    verbose_log(cli.verbose, 1, format!("ICMP Response for port {}", embedded_dst_port));
                                    if let Some(probe) = probes_map.get_mut(&embedded_dst_port) {
                                        probe.arrival = Some(packet_timestamp_to_duration(&packet.header));
                                        probe.rtt = Some(probe.arrival.unwrap() - probe.departure);
                                    } else {
                                        verbose_log(cli.verbose, 1, format!("No matching probe found for ICMP response to port {}", embedded_dst_port));
                                    }
                                }
                            },
                            _ => break,
                        }
                    },
                    Err(_) => continue, // No packet received, continue to the next iteration
                }
            }

            verbose_log(cli.verbose, 0, "UDP probing report".to_string());
            verbose_log(cli.verbose, 0, "--------------------------".to_string());
            verbose_log(cli.verbose, 0, format!("Destination: {}", cli.host));
            verbose_log(cli.verbose, 0, format!("Probes sent: {}", cli.numprobes));
            verbose_log(cli.verbose, 0, format!("ICMP port unreachable responses: {}", responses));
            verbose_log(cli.verbose, 0, format!("Lost replies: {}", cli.numprobes-responses));
            verbose_log(cli.verbose, 0, "\n".to_string());
            verbose_log(cli.verbose, 0, "Per-probe results:".to_string());
            verbose_log(cli.verbose, 0, format!("ID     RTT"));
            verbose_log(cli.verbose, 0, "--------------------------".to_string());

            let mut probes_vec: Vec<(&u16, &Probe)> = probes_map.iter().collect();
            probes_vec.sort_by_key(|(id, _probe)| *id);
            for (_port, probe) in probes_vec {
                if let Some(_arrival) = probe.arrival {
                    verbose_log(cli.verbose, 0, format!("{}      {:?}", probe.id, probe.rtt.unwrap()));
                } 
            }
    });

    for port in PORT..(PORT + cli.numprobes) {
        verbose_log(cli.verbose, 0, format!("Probing {} on port {}", ip, port));
        std::thread::sleep(std::time::Duration::from_millis(500)); // Sleep briefly between sending probes to avoid overwhelming the network
        
        // Here you would implement the actual probing logic, such as sending UDP packets
        // and listening for ICMP responses. This is just a placeholder to show where that logic would go.
        let _ = sock.send_to(&[0], (host.as_str(), port));
        std::thread::sleep(std::time::Duration::from_millis(500)); // Sleep between probes
    }

    rx_thread.join().unwrap();


}
