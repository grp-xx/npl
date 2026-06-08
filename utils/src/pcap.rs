use std::time::Duration;

pub fn packet_timestamp_to_duration(pcap_hdr: &pcap::PacketHeader) -> Duration {
    let ts = pcap_hdr.ts;
    Duration::new(ts.tv_sec as u64, (ts.tv_usec as u32) * 1_000)
}