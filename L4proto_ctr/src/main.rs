use clap::{ArgGroup, Parser};
use std::path::PathBuf;

#[derive(Parser, Debug)]
#[command(group(
    ArgGroup::new("input")
        .required(true)
        .args(["interface", "file"])
))]
struct Cli {
    /// Network interface to read from
    #[arg(short = 'i', long)]
    interface: Option<String>,

    /// File to read from
    #[arg(short = 'f', long)]
    file: Option<PathBuf>,
    
    /// Verbose mode (-v, -vv, -vvv, etc.)
    #[arg(short, long, action = clap::ArgAction::Count)]
    verbose: u8,
}



fn main() {

    let cli = Cli::parse();

    match (cli.interface, cli.file) {
        (Some(interface), None) => {
            verbose_log(
                cli.verbose,
                1,
                format!("Reading from network interface: {}", interface),
            );
        }
        (None, Some(file)) => {
            verbose_log(
                cli.verbose,
                1,
                format!("Reading from file: {}", file.display()),
            );
        }
        _ => unreachable!(), // This should never happen due to the ArgGroup
    } 



}



fn verbose_log(verbose: u8, level: u8, message: String) {
    if verbose >= level {
        eprintln!("{message}");
    }
}

