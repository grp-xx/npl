use etherparse::{NetHeaders::Ipv4, PacketHeaders, SlicedPacket};
use pcap::{Active, Capture};

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
                //                let sliced_frame = SlicedPacket::from_ethernet(&frame);
                let frame_hdr = PacketHeaders::from_ethernet_slice(&frame);
                // println!("Network Layer: {:?}", frame_hdr.unwrap().net);
                let ip = frame_hdr.unwrap().net.unwrap();
                match ip {
                    Ipv4(hdr,_) => {
                        println!("Source IP: {:?}", hdr.source);
                        println!("Destination IP: {:?}", hdr.destination);
                        println!("Protocol: {:?}", hdr.protocol);
                    }
                    _ => {}
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
