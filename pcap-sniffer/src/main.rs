use pcap::{BpfProgram, Capture, Device};

fn main() {
    let interfaces = Device::list().unwrap();
    for i in interfaces.iter() {
        println!("{}", i.name);
    }

    let mydev = "en8";
    let mut cap = Capture::from_device(mydev)
        .expect("Can't open device")
        .immediate_mode(true)
        .promisc(true)
        .snaplen(64)
        .open()
        .expect("Can't open Pcap device");

    cap.filter("ip", true).unwrap();
    //    let mut cap = Capture::from_file("../Traces/smallFlows.pcap").expect("Can't open pacp file");
    //    println!("LinkType: {:?}", cap.get_datalink());
    //    while let Ok(frame) = cap.next_packet() {
    //        //     println!("Header {:?}",&pkt.header);
    //        let ethertype = &frame.data[12..14];
    //        if ethertype == [0x08, 0x00] {
    //            println!("Packet: {:?}", &frame.data[14..]);
    //            println!("Src Addr: {:?}", &frame.data[26..30]);
    //            println!("Dst Addr: {:?}", &frame.data[30..34]);
    //        }
    //    }
    //

    let pkt_handler = |frame: pcap::Packet<'_>| {
        let ethertype = &frame.data[12..14];
        if ethertype == [0x08, 0x00] {
            //     println!("Packet: {:?}", &frame.data[..]);
            println!("_______________________");
            println!(
                "Timestamp: {}.{}",
                frame.header.ts.tv_sec, frame.header.ts.tv_usec
            );
            println!("Src Addr: {:?}", &frame.data[26..30]);
            println!("Dst Addr: {:?}", &frame.data[30..34]);
        }
    };

    // Mediante callback
    //
    // cap.for_each(Some(10), pkt_handler).unwrap();
    cap.for_each(Some(30), pkt_handler).unwrap();
    println!("Stats: {:?}", cap.stats().unwrap());
}
