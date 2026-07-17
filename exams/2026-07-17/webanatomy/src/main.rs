use clap::Parser;
use std::io::Read;
use std::net::{Ipv4Addr, SocketAddr, ToSocketAddrs};
use std::time::Instant;
use url::Url;

#[derive(Parser, Debug)]
#[command(name = "webanatomy")]
struct Args {
    /// URL to request, for example https://example.com/
    url: String,

    /// Network interface to capture on, for example en0 or eth0
    #[arg(short, long)]
    interface: String,

    /// Maximum capture duration, in seconds
    #[arg(short, long, default_value_t = 5)]
    timeout: u64,
}

fn main() {
    let args = Args::parse();

    // Part 1: parse the URL and determine the remote endpoint.
    let url = Url::parse(&args.url).expect("invalid URL");
    if url.scheme() != "http" && url.scheme() != "https" {
        panic!("unsupported URL scheme: {}", url.scheme());
    }

    let host = url.host_str().expect("URL has no host").to_string();
    let port = url
        .port_or_known_default()
        .expect("cannot determine remote port");

    // Part 2: resolve the host and prepare the BPF filter for packet capture.
    let ips = resolve_ipv4s(&host, port);
    let bpf = build_bpf_filter(&ips, port);

    // Part 3: perform the application-level HTTP/HTTPS request.
    let request_start = Instant::now();
    let (http_status, app_bytes) = http_get(&args.url);
    let request_time = request_start.elapsed();

    print_report(
        &url,
        &host,
        port,
        &ips,
        &args.interface,
        args.timeout,
        &bpf,
        http_status,
        app_bytes,
        request_time.as_secs_f64() * 1000.0,
    );

    // Student task:
    // Open a pcap capture on args.interface, install the BPF filter above,
    // capture TCP packets while the request runs, parse Ethernet/IPv4/TCP,
    // and count packets/bytes in both directions.
}

fn resolve_ipv4s(host: &str, port: u16) -> Vec<Ipv4Addr> {
    let mut ips = Vec::new();
    let addrs = (host, port).to_socket_addrs().expect("cannot resolve host");

    for addr in addrs {
        if let SocketAddr::V4(v4) = addr {
            let ip = *v4.ip();
            if !ips.contains(&ip) {
                ips.push(ip);
            }
        }
    }

    if ips.is_empty() {
        panic!("no IPv4 address found for {host}");
    }

    ips
}

fn build_bpf_filter(ips: &[Ipv4Addr], port: u16) -> String {
    let hosts = ips
        .iter()
        .map(|ip| format!("host {ip}"))
        .collect::<Vec<_>>()
        .join(" or ");

    format!("({hosts}) and tcp and port {port}")
}

fn http_get(url: &str) -> (u16, usize) {
    // HTTP error statuses are still valid responses: read their body too.
    match ureq::get(url).call() {
        Ok(response) => read_response(response),
        Err(ureq::Error::Status(_, response)) => read_response(response),
        Err(e) => panic!("request failed: {e}"),
    }
}

fn read_response(response: ureq::Response) -> (u16, usize) {
    let status = response.status();
    let mut body = Vec::new();
    response
        .into_reader()
        .read_to_end(&mut body)
        .expect("cannot read response body");

    (status, body.len())
}

fn print_report(
    url: &Url,
    host: &str,
    port: u16,
    ips: &[Ipv4Addr],
    interface: &str,
    timeout: u64,
    bpf: &str,
    http_status: u16,
    app_bytes: usize,
    request_time_ms: f64,
) {
    println!("webanatomy - starter");
    println!("URL: {url}");
    println!("Host: {host}");
    println!("Remote port: {port}");
    println!("Resolved IPv4 addresses: {ips:?}");

    println!("\nCapture setup");
    println!("Interface: {interface}");
    println!("Timeout: {timeout} s");
    println!("BPF filter: {bpf}");

    println!("\nApplication");
    println!("HTTP status: {http_status}");
    println!("Bytes received by GET: {app_bytes}");
    println!("Request time: {request_time_ms:.2} ms");

}
