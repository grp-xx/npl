use std::net::ToSocketAddrs;
use clap::Parser;


#[derive(Parser, Debug)]
struct Cli {
    /// Host Name
    host: String, // Note: This is a positional argument, so it doesn't need a flag like --host
    /// Host Name
    #[arg(short = '4', long)]
    ipv4: bool,
    /// Host Name
    #[arg(short = '6', long)]
    ipv6: bool,
    /// Verbose mode (-v, -vv, -vvv, etc.)
    #[arg(short, long, action = clap::ArgAction::Count)]
    verbose: u8,
}

fn main() {
    let cli = Cli::parse();
    verbose_log(cli.verbose, 1, format!("Looking up DNS for host: {}", cli.host));
    let socket = format!("{}:0", cli.host); // Port 0 is used just for DNS lookup
    match socket.to_socket_addrs() {
        Ok(ips) => {
            for ip in ips {
                match (cli.ipv4, cli.ipv6) {
                    (true, false) if ip.is_ipv4() => (), // Note the guard conditions to filter based on user input
                    (false, true) if ip.is_ipv6() => (),
                    (true, true) => (),
                    (false, false) => (),
                    _ => continue,
                }
                println!("{} has IP address: {}", cli.host, ip.ip());
            }
        },
        Err(e) => eprintln!("Failed to lookup DNS for {}: {}", cli.host, e),
    }
}

fn verbose_log(verbose: u8, level: u8, message: String) {
    if verbose >= level {
        eprintln!("{message}");
    }
}