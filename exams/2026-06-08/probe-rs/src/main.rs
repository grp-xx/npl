use clap::Parser;

#[derive(Parser, Debug)]
struct Cli {
    /// Target host to probe
//    #[arg(long)]
    host: String,

    /// Number of probes to send
    #[arg(short = 'n', long, default_value_t = 5)]
    numprobes: u16,
    
    /// Network interface to read from
    #[arg(short = 'i', long)]
    interface: Option<String>,

    /// Verbose mode (-v, -vv, -vvv, etc.)
    #[arg(short, long, action = clap::ArgAction::Count)]
    verbose: u8,
}
fn main() {
//    let cli = Cli::parse();
    println!("Hello, welcome to the 2026-06-07 exam!");
    println!("Good luck!!!");

}
