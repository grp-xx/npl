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
//            verbose_log(
//                cli.verbose,
//                1,
//                format!("Reading from network interface: {}", interface),
//            );
            live_capture(cli.verbose, &interface);
        }
        (None, Some(file)) => {
//            verbose_log(
//                cli.verbose,
//                1,
//                format!("Reading from file: {}", file.display()),
//            );
            read_from_file(cli.verbose, &file);
        }
        _ => unreachable!(), // This should never happen due to the ArgGroup
    } 



}

fn live_capture(verbose: u8,interface: &str) {
    // Placeholder for live capture logic
    verbose_log(verbose, 2, format!("Starting live capture on interface: {}", interface));
}

fn read_from_file(verbose: u8,file: &PathBuf) {
    // Placeholder for file reading logic
    verbose_log(verbose, 2, format!("Starting to read from file: {}", file.display()));
}

fn verbose_log(verbose: u8, level: u8, message: String) {
    if verbose >= level {
        eprintln!("{message}");
    }
}

