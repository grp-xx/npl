use pnet::datalink::{self, Channel::Ethernet};
use pnet::datalink::{Config, EtherType, NetworkInterface};
use pnet::packet::Packet;
use pnet::packet::ethernet::{EtherTypes, EthernetPacket};
use pnet::packet::ipv4::Ipv4Packet;

fn set_interface(iface: &str) -> Option<NetworkInterface> {
    datalink::interfaces().into_iter().find(|i| i.name == iface)
}

fn main() {
    let ifaces: Vec<_> = std::env::args().skip(1).collect();
    // let devices = datalink::interfaces();

    let interface = match set_interface(&ifaces[0]) {
        Some(interface) => interface,
        None => panic!("Device {} not found", &ifaces[0]),
    };

    let configuration = Config::default();

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
                            println!("{:?}", ip_hdr);
                        };
                    }
                    _ => {}
                },
                None => {}
            },
            Err(_) => {}
        }
    }

    //    for i in devices.iter() {
    //        println!("{}",i.name);
    //    }
}
