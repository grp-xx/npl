use clap::{ArgGroup, Parser};
use etherparse::IpNumber;
use std::collections::HashMap;
use std::io::Write;
use std::net::SocketAddrV4;
use std::path::PathBuf;
use std::time::Duration;
use utils::log::verbose_log;
use utils::net::FlowIdV4;

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
#[derive(Debug, Clone, Copy)]
enum ConnectionState {
    Open,
    Fin,
    Closed,
}

#[derive(Debug, Clone, Copy)]
struct TcpConnection {
    flow_id: FlowIdV4,
    state: ConnectionState,
    num_packets: u64,
    bytes_transferred: u64,
    initiation_time: Duration,
    final_time: Duration,
}

fn packet_timestamp_to_duration(pcap_hdr: &pcap::PacketHeader) -> Duration {
    let ts = pcap_hdr.ts;
    Duration::new(ts.tv_sec as u64, (ts.tv_usec as u32) * 1_000)
}

impl TcpConnection {
    fn new(flow_id: FlowIdV4, pcap_hdr: &pcap::PacketHeader) -> Self {
        TcpConnection {
            flow_id,
            state: ConnectionState::Open,
            num_packets: 1,
            bytes_transferred: 0,
            initiation_time: packet_timestamp_to_duration(&pcap_hdr),
            final_time: packet_timestamp_to_duration(&pcap_hdr),
        }
    }

    fn update(&mut self, newstate: ConnectionState, segment_size: u64) {
        self.state = newstate;
        self.num_packets += 1;
        self.bytes_transferred += segment_size as u64;
    }

    fn close(&mut self, pcap_hdr: &pcap::PacketHeader, segment_size: u64) {
        self.state = ConnectionState::Closed;
        self.num_packets += 1;
        self.bytes_transferred += segment_size as u64;
        self.final_time = packet_timestamp_to_duration(&pcap_hdr);
    }
}

#[derive(Debug, Clone, Copy)]
struct TcpConnStats {
    flow_id: FlowIdV4,
    num_packets: u64,
    bytes_transferred: u64,
    duration: Duration,
}

impl TcpConnStats {
    fn from_connection(conn: &TcpConnection) -> Self {
        TcpConnStats {
            flow_id: conn.flow_id,
            num_packets: conn.num_packets,
            bytes_transferred: conn.bytes_transferred,
            duration: conn.final_time - conn.initiation_time,
        }
    }
}

#[derive(Debug, Clone, Copy)]
enum PacketAction {
    Opened,
    Updated,
    Closed,
    Ignored,
}

fn main() {
    let cli = Cli::parse();

    match (cli.interface, cli.file) {
        (Some(interface), None) => live_capture(cli.verbose, &interface),
        (None, Some(file)) => read_from_file(cli.verbose, &file),
        _ => unreachable!(), // This should never happen due to the ArgGroup
    }
}

fn live_capture(verbose: u8, interface: &str) {
    // Placeholder for live capture logic
    verbose_log(
        verbose,
        1,
        format!("Starting live capture on interface: {}", interface),
    );

    let mut cap = pcap::Capture::from_device(interface)
        .expect("Failed to find the specified interface")
        .promisc(true)
        .immediate_mode(true)
        .open()
        .expect("Failed to open the specified interface");
    match cap.filter("ip and tcp", true) {
        Ok(()) => verbose_log(
            verbose,
            2,
            "Installed capture filter: ip and tcp".to_string(),
        ),
        Err(err) => verbose_log(
            verbose,
            1,
            format!("Failed to install capture filter 'ip and tcp': {err}"),
        ),
    }
    let mut tcp_conn_map = HashMap::<FlowIdV4, TcpConnection>::new();
    let (tx, rx) = std::sync::mpsc::channel::<TcpConnection>();

    let t = std::thread::spawn(move || {
        let mut connection_vector: Vec<TcpConnStats> = Vec::new();
        let now = std::time::Instant::now();

        for conn in rx.iter() {
            connection_vector.push(TcpConnStats::from_connection(&conn));
            let elapsed = now.elapsed();
            if elapsed >= Duration::from_secs(10) {
                connection_vector.sort_by(|a, b| b.duration.cmp(&a.duration));
                print_top_connections(&mut connection_vector);
            }
        }
    });

    let mut packet_count = 0;
    let mut opened_connections = 0;
    let mut closed_connections = 0;
    let mut ignored_packets = 0;
    while let Ok(frame) = cap.next_packet() {
        packet_count += 1;
        match packet_handler(verbose, &mut tcp_conn_map, tx.clone(), frame) {
            PacketAction::Opened => opened_connections += 1,
            PacketAction::Closed => closed_connections += 1,
            PacketAction::Ignored => ignored_packets += 1,
            PacketAction::Updated => (),
        }
    }
    drop(tx);
    t.join().unwrap();
    verbose_log(
        verbose,
        1,
        format!(
            "Live capture stopped: packets={packet_count}, opened={opened_connections}, closed={closed_connections}, active={}, ignored={ignored_packets}",
            tcp_conn_map.len()
        ),
    );
}

fn read_from_file(verbose: u8, file: &PathBuf) {
    verbose_log(
        verbose,
        1,
        format!("Reading packets from file: {}", file.display()),
    );

    let mut cap = pcap::Capture::from_file(file).expect("Failed to open pcap file");
    match cap.filter("ip and tcp", true) {
        Ok(()) => verbose_log(
            verbose,
            2,
            "Installed capture filter: ip and tcp".to_string(),
        ),
        Err(err) => verbose_log(
            verbose,
            1,
            format!("Failed to install capture filter 'ip and tcp': {err}"),
        ),
    }

    let mut tcp_conn_map = HashMap::<FlowIdV4, TcpConnection>::new();
    let (tx, rx) = std::sync::mpsc::channel::<TcpConnection>();
    let t = std::thread::spawn(move || {
        let mut connection_vector: Vec<TcpConnStats> = rx
            .iter()
            .map(|conn| TcpConnStats::from_connection(&conn))
            .collect();

        verbose_log(
            verbose,
            1,
            format!(
                "Finished collecting closed connections: {}",
                connection_vector.len()
            ),
        );

        print_top_connections(&mut connection_vector);
    });

    let mut packet_count = 0;
    let mut opened_connections = 0;
    let mut closed_connections = 0;
    let mut ignored_packets = 0;
    while let Ok(frame) = cap.next_packet() {
        packet_count += 1;
        match packet_handler(verbose, &mut tcp_conn_map, tx.clone(), frame) {
            PacketAction::Opened => opened_connections += 1,
            PacketAction::Closed => closed_connections += 1,
            PacketAction::Ignored => ignored_packets += 1,
            PacketAction::Updated => (),
        }
    }
    drop(tx);
    t.join().unwrap();
    verbose_log(
        verbose,
        1,
        format!(
            "Finished reading {}: packets={packet_count}, opened={opened_connections}, closed={closed_connections}, active={}, ignored={ignored_packets}",
            file.display(),
            tcp_conn_map.len()
        ),
    );
}

fn extract_flow_id(data: &[u8]) -> Option<FlowIdV4> {
    let iphdr = etherparse::Ipv4HeaderSlice::from_slice(data).ok()?;
    let ipl = (iphdr.ihl() * 4) as usize;
    match iphdr.protocol() {
        IpNumber::TCP => {
            let tcphdr = etherparse::TcpHeaderSlice::from_slice(&data[ipl..]).ok()?;
            let s1 = SocketAddrV4::new(iphdr.source_addr(), tcphdr.source_port());
            let s2 = SocketAddrV4::new(iphdr.destination_addr(), tcphdr.destination_port());
            Some(FlowIdV4::new(&s1, &s2, iphdr.protocol().into()))
        }
        _ => None, // Skip non-TCP packets (they should not be present due to the filter, but we check just in case)
    }
}

fn packet_handler(
    verbose: u8,
    tcp_conn_map: &mut HashMap<FlowIdV4, TcpConnection>,
    tx: std::sync::mpsc::Sender<TcpConnection>,
    frame: pcap::Packet,
) -> PacketAction {
    let iphdr = etherparse::Ipv4HeaderSlice::from_slice(&frame.data[14..]).unwrap();
    let ipl = (iphdr.ihl() * 4) as usize;
    let tcp = etherparse::TcpSlice::from_slice(&frame.data[14 + ipl..]).unwrap();

    let flow_id = match extract_flow_id(&frame.data[14..]) {
        Some(flow_id) => flow_id,
        None => {
            verbose_log(
                verbose,
                2,
                "Ignoring packet because no TCP flow id could be extracted".to_string(),
            );
            return PacketAction::Ignored;
        }
    };
    let payload_len = tcp.payload().len() as u64;

    verbose_log(
        verbose,
        3,
        format!(
            "TCP packet: flow={flow_id}, flags={}, payload={payload_len} bytes",
            tcp_flags(&tcp)
        ),
    );

    match (tcp.syn(), tcp.ack(), tcp.fin(), tcp.rst()) {
        (true, false, false, false) => {
            let conn = TcpConnection::new(flow_id, &frame.header);
            let replaced = tcp_conn_map.insert(flow_id, conn).is_some();
            verbose_log(
                verbose,
                2,
                format!("Opened TCP connection: flow={flow_id}, replaced_existing={replaced}"),
            );
            PacketAction::Opened
        }

        (false, _, true, false) => match tcp_conn_map.get_mut(&flow_id) {
            Some(conn) => match conn.state {
                ConnectionState::Open => {
                    conn.update(ConnectionState::Fin, payload_len);
                    verbose_log(
                        verbose,
                        2,
                        format!(
                            "Observed first FIN: flow={flow_id}, state={:?}, packets={}, bytes={}",
                            conn.state, conn.num_packets, conn.bytes_transferred
                        ),
                    );
                    PacketAction::Updated
                }
                ConnectionState::Fin => {
                    conn.close(&frame.header, payload_len);
                    verbose_log(
                        verbose,
                        1,
                        format!(
                            "Closed TCP connection after FIN: flow={flow_id}, packets={}, bytes={}, duration={:?}",
                            conn.num_packets,
                            conn.bytes_transferred,
                            conn.final_time - conn.initiation_time
                        ),
                    );
                    if tx.send(*conn).is_err() {
                        verbose_log(
                            verbose,
                            1,
                            format!("Could not report closed connection: flow={flow_id}"),
                        );
                    }
                    PacketAction::Closed
                }
                ConnectionState::Closed => {
                    verbose_log(
                        verbose,
                        2,
                        format!("Ignoring FIN for already closed connection: flow={flow_id}"),
                    );
                    PacketAction::Ignored
                }
            },
            None => {
                verbose_log(
                    verbose,
                    2,
                    format!("Ignoring FIN for unknown connection: flow={flow_id}"),
                );
                PacketAction::Ignored
            }
        },

        (false, _, _, true) => match tcp_conn_map.get_mut(&flow_id) {
            Some(conn) => {
                conn.close(&frame.header, payload_len);
                verbose_log(
                    verbose,
                    1,
                    format!(
                        "Closed TCP connection after RST: flow={flow_id}, packets={}, bytes={}, duration={:?}",
                        conn.num_packets,
                        conn.bytes_transferred,
                        conn.final_time - conn.initiation_time
                    ),
                );
                if tx.send(*conn).is_err() {
                    verbose_log(
                        verbose,
                        1,
                        format!("Could not report reset connection: flow={flow_id}"),
                    );
                }
                PacketAction::Closed
            }
            None => {
                verbose_log(
                    verbose,
                    2,
                    format!("Ignoring RST for unknown connection: flow={flow_id}"),
                );
                PacketAction::Ignored
            }
        },
        _ => match tcp_conn_map.get_mut(&flow_id) {
            Some(conn) => {
                conn.update(conn.state, payload_len);
                verbose_log(
                    verbose,
                    3,
                    format!(
                        "Updated TCP connection: flow={flow_id}, state={:?}, packets={}, bytes={}",
                        conn.state, conn.num_packets, conn.bytes_transferred
                    ),
                );
                PacketAction::Updated
            }
            None => {
                verbose_log(
                    verbose,
                    3,
                    format!(
                        "Ignoring non-opening packet for unknown connection: flow={flow_id}, flags={}",
                        tcp_flags(&tcp)
                    ),
                );
                PacketAction::Ignored
            }
        },
    }
}

fn tcp_flags(tcp: &etherparse::TcpSlice<'_>) -> String {
    let mut flags = Vec::new();
    if tcp.fin() {
        flags.push("FIN");
    }
    if tcp.syn() {
        flags.push("SYN");
    }
    if tcp.rst() {
        flags.push("RST");
    }
    if tcp.psh() {
        flags.push("PSH");
    }
    if tcp.ack() {
        flags.push("ACK");
    }
    if tcp.urg() {
        flags.push("URG");
    }
    if tcp.ece() {
        flags.push("ECE");
    }
    if tcp.cwr() {
        flags.push("CWR");
    }

    if flags.is_empty() {
        "NONE".to_string()
    } else {
        flags.join("|")
    }
}

fn print_top_connections(connection_vector: &mut Vec<TcpConnStats>) {
    print!("\x1b[?25l"); // hide cursor
    print!("\x1b[2J\x1b[H"); // Clear the terminal
    println!("Top 10 longest TCP connections:");
    println!(
        "--------------------------------------------------------------------------------------------------------"
    );

    connection_vector.sort_by(|a, b| b.duration.cmp(&a.duration));
    connection_vector.iter().take(10).for_each(|c| {
        println!(
            "{:<21} -- Packets: {:<8} Bytes: {:<8} Duration: {:<16?}",
            c.flow_id, c.num_packets, c.bytes_transferred, c.duration
        );
    });
    std::io::stdout().flush().unwrap();
}
