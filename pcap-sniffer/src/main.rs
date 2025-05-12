use pcap::{Capture, Device};


fn main() {
    let interfaces = Device::list().unwrap();
    for i in interfaces.iter() {
        println!("{}",i.name);
    }

//    let mydev = "en8";
//    let mut cap = Capture::from_device(mydev)
//    .expect("Can't open device")
//    .immediate_mode(true)
//    .promisc(true)
//    .snaplen(64)
//    .open()
//    .expect("Can't open Pcap device");

    let mut cap = Capture::from_file("../Traces/smallFlows.pcap")
    .expect("Can't open pacp file");

    while let Ok(pkt) = cap.next_packet() {
//         println!("Header {:?}",pkt.header);
        let ethertype = &pkt.data[12..14];
        if ethertype == [0x08,0x00] {
            println!("Packet: {:0x?}",&pkt.data[14..]);
        }
    }
}
