use std::net::{UdpSocket, SocketAddr};
// use std::net::ToSocketAddrs;
use clap::{Parser};


#[derive(Parser, Debug)]
struct Cli {
    /// Server host
    #[arg(long, default_value = "127.0.0.1")]
    host: String,
    /// Server port
    #[arg(short, long, default_value_t = 10000)]
    port: u16,
    /// News message to broadcast 
    #[arg(short, long, default_value = "Breaking news: Rust is awesome!")]
    news: String,
    /// Repeat interval in seconds 
    #[arg(short, long, default_value_t = 2)]
    interval: u16,
    /// Verbose mode (-v, -vv, -vvv, etc.)
    #[arg(short, long, action = clap::ArgAction::Count)]
    verbose: u8,
}

fn main() {
    let cli = Cli::parse();
    let sock_addr: SocketAddr = format!("{}:{}",cli.host, 0).parse().unwrap();

    let srv_sock = UdpSocket::bind(sock_addr).expect("Failed to bind socket");
    srv_sock.set_broadcast(true).expect("Failed to enable broadcast");
    verbose_log(cli.verbose, 1, format!("Server broadcasting news on {}", srv_sock.local_addr().unwrap()));

    let bcast_addr = SocketAddr::from(([255, 255, 255, 255], cli.port));

    let message = format!("{}\n", cli.news);
    loop {
        match srv_sock.send_to(message.as_bytes(), bcast_addr) {
            Ok(bytes_sent) => verbose_log(cli.verbose, 2, format!("Sent {} bytes: {}", bytes_sent, cli.news)),
            Err(e) => eprintln!("Failed to send news: {}", e),
        }
        std::thread::sleep(std::time::Duration::from_secs(cli.interval as u64));
    }



}




fn verbose_log(verbose: u8, level: u8, message: String) {
    if verbose >= level {
        eprintln!("{message}");
    }
}