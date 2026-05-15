use std::net::IpAddr;
use pcap::Capture;

fn main() {

    let mut cap = Capture::from_device("en0").unwrap()
        .promisc(true)
        .snaplen(64)
       // .timeout(3000)
        .immediate_mode(true)
        .open().unwrap();

//    let mut cap = Capture::from_file("../Traces/smallFlows.pcap")
//                                        .unwrap();
//    println!("Link type: {:?}", cap.get_datalink());
    cap.filter("ip", true).unwrap();
//    while let Ok(packet) = cap.next_packet() {
////        println!("EtherType {:02x?}",&packet.data[12..14]);
//        let src_ip = IpAddr::from([packet.data[26], packet.data[27], packet.data[28], packet.data[29]]);
//        let dst_ip = IpAddr::from([packet.data[30], packet.data[31], packet.data[32], packet.data[33]]);
//        let l4_proto = packet.data[23];
//
//        println!("{} --> {} - {}",
//                 src_ip,
//                 dst_ip,
//                 l4_proto);
//
//    }


    cap.for_each(Some(100),|packet| pkt_handler(packet)).unwrap();

    println!("Received {} packets", cap.stats().unwrap().received);
    println!("Dropped {} packets", cap.stats().unwrap().dropped);
    println!("If dropped {} packets", cap.stats().unwrap().if_dropped);

    
}

fn pkt_handler(packet: pcap::Packet) {
    let src_ip = IpAddr::from([packet.data[26], packet.data[27], packet.data[28], packet.data[29]]);
    let dst_ip = IpAddr::from([packet.data[30], packet.data[31], packet.data[32], packet.data[33]]);
    let l4_proto = packet.data[23];    
    println!("Packet timestamp: {}.{}", packet.header.ts.tv_sec, packet.header.ts.tv_usec);
    println!("Packet snaplen: {}", packet.header.caplen);
    println!("Packet length: {}", packet.header.len);   
    println!("{} --> {} - {}",
             src_ip,
             dst_ip,
             l4_proto);
}
