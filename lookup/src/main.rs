use std::net::ToSocketAddrs;
fn main() {
    let input: Vec<_> = std::env::args().collect();
    if input.len() != 2 {
        println!("Usage: {} <hostname> ", &input[0]);
        return;
    }

    let hostname = &input[1];
    let saddr = format!("{}:{}",hostname,0);

    let addresses = saddr.to_socket_addrs().unwrap();

    for addr in addresses {
        println!("{}",addr.ip());
    }
}
