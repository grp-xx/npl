use pcap::{Active, Capture};
use pnet_packet::{ethernet::{EtherType, EtherTypes, EthernetPacket}, ipv4::Ipv4Packet, Packet};

fn main() {
    let input: Vec<String> = std::env::args().collect();
    if std::env::args().len() < 3 {
        println!("Usage: {} [i device | f filename]", &input[0]);
        return;
    }

    match input[1].as_str() {
        "i" => {
            let mut cap = Capture::from_device(&input[2][..])
                .unwrap()
                .immediate_mode(true)
                .promisc(true)
                .snaplen(5000)
                .open()
                .unwrap();

            cap.filter("ip", true).unwrap();

            cap.for_each(Some(1), |frame| {
                let eth = EthernetPacket::new(frame.data).unwrap();
                println!("Ethernet hdr: {:?}", eth);
                match eth.get_ethertype() {
                    EtherTypes::Ipv4 => {
                        let ip = Ipv4Packet::new(eth.payload()).unwrap();
                        println!("IP source: {}",ip.get_source());
                        println!("IP destinatio: {}",ip.get_destination());
                        println!("Transport Protocol: {}",ip.get_next_level_protocol());
                    },
                    _ => {},
                }
            })
            .unwrap();
        }
        "f" => {
            let mut cap = Capture::from_file(&input[2][..]).unwrap();

            cap.for_each(Some(1), |frame| {
                println!("Got packet");
            })
            .unwrap();
        }
        _ => {
            println!("Usage: {} [i device | f filename]", &input[0]);
            return;
        }
    };
}
