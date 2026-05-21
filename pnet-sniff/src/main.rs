use pnet::datalink::{self, Config, Channel::Ethernet};
use pnet::packet::ethernet::{EthernetPacket, EtherType};
use pnet::packet::{Packet,ipv4::Ipv4Packet};
use clap::Parser;

#[derive(Parser, Debug)]

struct Cli {
    /// Network interface to read from
    #[arg(short = 'i', long)]
    interface: Option<String>,
    
    /// Number of packets to capture
    #[arg(short = 'n', long, default_value = None)]
    numpkts: Option<usize>,

    /// Verbose mode (-v, -vv, -vvv, etc.)
    #[arg(short, long, action = clap::ArgAction::Count)]
    verbose: u8,
}
fn main() {

    let cli = Cli::parse();
    let nic = cli.interface.unwrap();
    // let interfaces = datalink::interfaces();

    let interface = datalink::interfaces()
                    .into_iter()
                    .find(|iface| iface.name == nic)
                    .expect("Interface unavailable");




    println!("Hello Interface: {:?}", interface);
    let config = Config::default();

    let (_, mut rx) = match datalink::channel(&interface, config) {
        Ok(Ethernet(tx, rx)) => (tx,rx),
        Ok(_) => panic!("unhandled channel type"),
        Err(e) => panic!("unable to create channel: {e}"),
    };

    loop {
        match rx.next() {
            Ok(frame) => {
                // println!("Received frame: {:?}", frame);
                let ethhdr = EthernetPacket::new(frame).unwrap();
                println!("Ethernet Header: {:?}", ethhdr);
                // println!("Payload: {:?}", ethhdr.payload());
                
                match ethhdr.get_ethertype() {
                    EtherType(0x0800) => {
                        if let Some(ipv4hdr) = Ipv4Packet::new(ethhdr.payload()) { 
                            println!("IPv4 packet detected!");
                            println!("{} -> {}", ipv4hdr.get_source(), ipv4hdr.get_destination());
                        }
                    }
                    EtherType(0x86DD) => println!("IPv6 packet detected!"),
                    _ => println!("Other EtherType: {:?}", ethhdr.get_ethertype()),
                }
            }
            Err(e) => {
                println!("An error occurred while reading: {e}");
            }
        }
    }

}
