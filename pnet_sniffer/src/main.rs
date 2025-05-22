use pnet::datalink::{self, Channel::Ethernet};
use pnet::datalink::{Config, FanoutOption, FanoutType, NetworkInterface};
use pnet::packet::Packet;
use pnet::packet::ethernet::{EtherTypes, EthernetPacket};
use pnet::packet::ipv4::Ipv4Packet;

fn set_interface(iface: &str) -> Option<NetworkInterface> {
    datalink::interfaces().into_iter().find(|i| i.name == iface)
}

fn main() {
    let ifaces: Vec<_> = std::env::args().skip(1).collect();
    // let devices = datalink::interfaces();
    let iface = &ifaces[0];

    let num_threads = 4;
    let mut t = vec![];

    for i in 0..num_threads {
        let iface = iface.clone();
        t.push(std::thread::spawn(move || {
            let interface = match set_interface(&iface) {
                Some(interface) => interface,
                None => panic!("Device {} not found", iface),
            };

            let mut configuration = Config::default();
            let f_out: FanoutOption = FanoutOption {
                group_id: 1234,
                fanout_type: FanoutType::HASH,
                defrag: false,
                rollover: true,
            };

            configuration.linux_fanout = Some(f_out);

            // println!("Default Configuration: {:?}", configuration);

            let (_tx, mut rx) = match datalink::channel(&interface, configuration) {
                Ok(Ethernet(tx, rx)) => (tx, rx),
                Ok(_) => panic!("Unsupported channel"),
                Err(e) => panic!("{}", e),
            };

            loop {
                match rx.next() {
                    Ok(frame) => match EthernetPacket::new(frame) {
                        Some(eth_hdr) => match eth_hdr.get_ethertype() {
                            EtherTypes::Ipv4 => {
                                let ip = Ipv4Packet::new(eth_hdr.payload());
                                if let Some(ip_hdr) = ip {
                                    println!(
                                        "Thread {}: {} --> {}",
                                        i,
                                        ip_hdr.get_source(),
                                        ip_hdr.get_destination()
                                    );
                                };
                            }
                            _ => {}
                        },
                        None => {}
                    },
                    Err(_) => {}
                }
            }
        }));
    }

    for i in t {
        let _ = i.join();
    }
}
