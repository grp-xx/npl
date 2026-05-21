use clap::{ArgGroup, Parser};
use std::path::PathBuf;
use std::io::Write;
use utils::log::verbose_log;


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
    let mut cap = pcap::Capture::from_device(interface)
        .unwrap()
        .promisc(true)
        .immediate_mode(true)
        .snaplen(64)
        .open()
        .unwrap();
    let (tx,rx) = std::sync::mpsc::channel::<u8>();
    let _ = std::thread::spawn(move || stats(rx, verbose));
    
    cap.for_each(None, |packet| {
        verbose_log(verbose, 3, format!("Processing packet"));
        // Here you would add your actual packet processing logic using proto_array
        tx.send(packet.data[23]).unwrap();
    }).unwrap();

}

fn read_from_file(verbose: u8,file: &PathBuf) {
    // Placeholder for file reading logic
    verbose_log(verbose, 2, format!("Starting to read from file: {}", file.display()));
}


fn stats(rx: std::sync::mpsc::Receiver<u8>, verbose: u8) {
    let mut proto_map = std::collections::HashMap::<u8, u64>::new();
    let mut now = std::time::Instant::now();
    
    
    for proto in rx
    {
        proto_map.entry(proto).and_modify(|c| *c+=1).or_insert(1);
        
        if now.elapsed().as_secs() >= 3 {
            let mut v: Vec<_> = proto_map.iter().filter(|(_, count)| **count > 0).collect();
            v.sort_by(|a, b| b.1.cmp(a.1));
            // Top 5 protocols
            print!("\x1b[2J\x1b[H");
            print!("\x1b[s");
            std::io::stdout().flush().unwrap();
            for i in 0..5 {
                print!("\x1b[u");
                if let Some((proto, count)) = v.get(i) {
                    verbose_log(verbose, 1, format!("Top {}: Proto: {}, Count: {}", i + 1, proto, count));
                }
            } 
            
            now = std::time::Instant::now();
        }
    }    
}



