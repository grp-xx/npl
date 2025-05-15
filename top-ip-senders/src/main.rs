use std::{
    collections::{HashMap, hash_map::Entry},
    net::Ipv4Addr,
    time::Instant,
};

use etherparse::{NetHeaders::Ipv4, PacketHeaders, SlicedPacket};
use pcap::{Active, Capture};

fn main() {
    let input: Vec<String> = std::env::args().collect();
    if std::env::args().len() < 3 {
        println!("Usage: {} [-i device | -f filename]", &input[0]);
        return;
    }

    match input[1].as_str() {
        "-i" => {
            let mut cap = Capture::from_device(&input[2][..])
                .unwrap()
                .immediate_mode(true)
                .promisc(true)
                .snaplen(5000)
                .open()
                .unwrap();

            cap.filter("ip", true).unwrap();

            let (tx, rx) = std::sync::mpsc::channel::<[u8; 4]>();
            let _ = std::thread::spawn(move || {
                let mut IPsenders: HashMap<Ipv4Addr, u32> = HashMap::new();
                let mut now = Instant::now();
                for ip in rx {
                    // println!("IP source: {:?}", ip);
                    let ips = Ipv4Addr::new(ip[0], ip[1], ip[2], ip[3]);
                    *IPsenders.entry(ips).or_insert(0) += 1;

                    if now.elapsed() > std::time::Duration::from_secs(5) {
                        let mut v: Vec<_> = IPsenders.iter().collect();
                        v.sort_by(|a, b| b.1.cmp(a.1));

                        println!(" Top 5 IP Senders");
                        println!("-----------------");
                        for i in 0..5 {
                            match v.get(i) {
                                Some(t) => println!("{}:{}", t.0, t.1),
                                None => {}
                            }
                        }
                        now = Instant::now();
                    }
                }
            });
            cap.for_each(None, |frame| {
                //                let sliced_frame = SlicedPacket::from_ethernet(&frame);
                let frame_hdr = PacketHeaders::from_ethernet_slice(&frame);
                // println!("Network Layer: {:?}", frame_hdr.unwrap().net);
                let ip = frame_hdr.unwrap().net.unwrap();
                match ip {
                    Ipv4(hdr, _) => {
                        let ips = hdr.source;
                        //               println!("IP inviato {:?}", ips);
                        tx.send(ips).unwrap();
                    }
                    _ => {}
                }
            })
            .unwrap();
        }
        "-f" => {
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
