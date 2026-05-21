use pnet::datalink::{self, Config, Channel::Ethernet};
use pnet::packet::ethernet::{EthernetPacket, EtherType};
use pnet::packet::{Packet,ipv4::Ipv4Packet};
use clap::Parser;
use utils::log::verbose_log;

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

    let interface= utils::pnet::get_interface(&nic);



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
                            utils::log::verbose_log(cli.verbose,
                                2,
                                format!("IPv4 packet detected: {} -> {}", ipv4hdr.get_source(), ipv4hdr.get_destination())); 
                        }
                    }
                    EtherType(0x86DD) => println!("IPv6 packet detected!"),
                    _ => utils::log::verbose_log(cli.verbose, 
                        2, 
                        format!("Other EtherType: {:?}", ethhdr.get_ethertype())),
                }
            }
            Err(e) => {
                println!("An error occurred while reading: {e}");
            }
        }
    }

}
