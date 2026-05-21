use pnet::datalink::{self, NetworkInterface};

pub fn get_interface(name: &str) -> NetworkInterface {
    datalink::interfaces()
    .into_iter()
    .find(|iface| iface.name == name)
    .expect("Interface not found")
}
