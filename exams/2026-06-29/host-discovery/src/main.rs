use clap::Parser;
use ipnet::Ipv4Net;

const DEFAULT_PORT: u16 = 54321;

#[derive(Parser, Debug)]
#[command(name = "host-discovery")]
#[command(about = "Simple LAN host discovery via UDP probes + ICMP Port Unreachable")]
struct Args {
    /// Subnet to scan, example:
    #[arg(short, long)]
    net: Ipv4Net,

    /// Device to sniff for ICMP packets, ex: eth0, wlan0, en0
    #[arg(short, long)]
    iface: String,

    /// UDP port likely closed on hosts, default: 54321
    #[arg(short, long, default_value_t = DEFAULT_PORT)]
    port: u16,

    /// Capture timeout in milliseconds
    #[arg(short, long, default_value_t = 5000)]
    timeout_ms: u64,

    /// Delay between UDP probes in microseconds
    #[arg(long, default_value_t = 10)]
    delay_us: u64,

    /// Reverse DNS optional
    #[arg(long, default_value_t = false)]
    rdns: bool,
    
    /// Verbose mode (-v, -vv, -vvv, etc.)
    #[arg(short, long, action = clap::ArgAction::Count)]
    verbose: u8,
}

fn main() {
    let cli = Args::parse();

    println!("Starting host discovery on subnet: {}", cli.net);

}

