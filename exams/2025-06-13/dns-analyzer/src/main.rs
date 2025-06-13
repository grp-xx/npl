use chrono::Local;
use clap::Parser;
// use pcap::{Capture, Device};
// use simple_dns::{QTYPE, TYPE};
// use std::collections::HashMap;
// use std::time::Instant;

// Command-line arguments structure

#[derive(Parser, Debug)]
#[command(name = "DNS Analyzer")]
#[command(author = "Your Name")]
#[command(version = "1.0")]
#[command(about = "Analyzes DNS queries in live or offline mode", long_about = None)]
struct Args {
    /// Interface for live packet capture (conflicts with --pcap)
    #[arg(short, long, conflicts_with = "pcap_file")]
    interface: Option<String>,

    /// PCAP file for offline analysis (conflicts with --interface)
    #[arg(short = 'f', long, conflicts_with = "interface")]
    pcap_file: Option<String>,

    /// Interval (in seconds) _between stats output
    #[arg(short = 'n', long, default_value = "5")]
    interval: u64,
}

fn main() {
    let args = Args::parse(); // uses `clap` to fill the struct

    println!("Now is {}", Local::now());
    println!("Interface: {:?}", args.interface);
    println!("pcap file: {:?}", args.pcap_file);
    println!("Interval: {:?}", args.interval);

    if let Some(interface) = args.interface {
        println!("[*] Reading live packets from interface: {}", interface);
    //        live capture processing logic
    } else if let Some(pcap_file) = args.pcap_file {
        println!("[*] Reading packets from file: {}", pcap_file);
    //        offline processing logic;
    } else {
        eprintln!("Please specify either an interface or a pcap file");
    }
}
