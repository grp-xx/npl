use clap::Parser;
use std::process;
// use serde::{Deserialize, Serialize};
use std::net::SocketAddr;
// use std::net::UdpSocket;
// use std::time::Duration;
use std::time::{SystemTime, UNIX_EPOCH};


// Clap fills this structure from the command line.
// Exactly one between --server and --client must be present.
#[derive(Parser)]
#[command(name = "clockprobe")]
struct Cli {
    #[arg(
        short = 's',
        long,
        value_name = "BIND_ADDRESS",
        conflicts_with = "client",
        required_unless_present = "client"
    )]
    server: Option<SocketAddr>,

    #[arg(
        short = 'c',
        long,
        value_name = "SERVER_ADDRESS",
        conflicts_with = "server",
        required_unless_present = "server"
    )]
    client: Option<SocketAddr>,

    #[arg(short = 'v', long)]
    verbose: bool,
}

// Return the current Unix time in microseconds.
// A clock error is fatal because no measurement would be meaningful.
fn _now_us() -> i64 {
    match SystemTime::now().duration_since(UNIX_EPOCH) {
        Ok(duration) => duration.as_micros() as i64,
        Err(error) => {
            eprintln!("cannot read the system clock: {error}");
            process::exit(1);
        }
    }
}

fn main() {
    let _cli = Cli::parse();

}
