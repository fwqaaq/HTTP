use std::net::IpAddr;
use std::thread::sleep;
use std::time::Instant;
use std::{net::Ipv4Addr, time::Duration};

use pnet::packet::icmp::{echo_request::MutableEchoRequestPacket, IcmpCode, IcmpPacket, IcmpTypes};
use pnet::packet::{self, ip::IpNextHeaderProtocol, Packet};
use pnet::transport::{
    icmp_packet_iter, transport_channel, TransportChannelType, TransportProtocol,
};

const TARGET: [u8; 4] = [114, 114, 114, 114];
const ICMP_BUFFER_SIZE: usize = 64;

fn main() {
    let (mut tx, mut rx) = match transport_channel(
        4096,
        TransportChannelType::Layer4(TransportProtocol::Ipv4(IpNextHeaderProtocol(1))),
    ) {
        Ok((tx, rx)) => (tx, rx),
        Err(e) => panic!(
            "An error occurred when creating the transport channel: {}",
            e
        ),
    };

    let mut iter = icmp_packet_iter(&mut rx);

    loop {
        let mut buffer = [0u8; ICMP_BUFFER_SIZE]; // Only ICMP length(8 bytes header + 56 bytes payload)
        let packet = create_icmp_packet(&mut buffer);
        let timer = Instant::now();
        tx.send_to(packet, IpAddr::V4(Ipv4Addr::from(TARGET)))
            .expect("Failed to send packet");
        let (ty, ip, elapsed) = match iter.next() {
            Ok((packet, ip)) if packet.get_icmp_type() == IcmpTypes::EchoReply => {
                (packet.get_icmp_type(), ip, timer.elapsed())
            }
            _ => continue,
        };
        sleep(Duration::from_secs(1));
        println!("ICMP TYPE: {:?} from {:?}, TIME: {:?}", ty, ip, elapsed);
    }
}

fn create_icmp_packet(header: &mut [u8]) -> MutableEchoRequestPacket {
    let mut packet = MutableEchoRequestPacket::new(header).expect("Failed to create ICMP packet.");
    packet.set_icmp_type(IcmpTypes::EchoRequest); // type -> 8
    packet.set_icmp_code(IcmpCode(0)); // code -> 0
    packet.set_sequence_number(1); // sequence number -> 1
    let pid = std::process::id() as u16;
    packet.set_identifier(pid); // identifier -> pid
    packet.set_payload(b"hello");
    let check_sum = packet::icmp::checksum(
        &IcmpPacket::new(packet.packet()).expect("Failed to convert to IcmpPacket"),
    );
    packet.set_checksum(check_sum);
    packet
}
