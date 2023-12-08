// use libc::{ARPOP_REPLY, ARPOP_REPLY};
use super::{macaddr::MacAddr, ETH_ALEN, ETH_P_ARP, ETH_P_IPV4, ETH_P_IPV6, ETH_P_RARP};
use std::{fmt::Debug, net::Ipv4Addr};

#[repr(C, packed)] // 禁止填充，紧密排列
#[derive(Clone)]
pub struct ArpHeader {
    pub arp_hrd: u16,            // 指定网络协议类型（例如，以太网）
    pub arp_pro: u16,            // 指定协议地址类型（例如，IPv4）
    pub arp_hln: u8,             // 指定硬件地址长度（例如，6）
    pub arp_pln: u8,             // 指定协议地址长度（例如，4）
    pub arp_op: u16,             // 指定操作类型（例如，ARP 请求或 ARP 回复）
    pub arp_sha: [u8; ETH_ALEN], // 指定发送方硬件地址
    pub arp_spa: [u8; 4],        // 指定发送方协议地址
    pub arp_tha: [u8; ETH_ALEN], // 指定目标硬件地址
    pub arp_tpa: [u8; 4],        // 指定目标协议地址
}

// arp 硬件类型
const ARP_HARDWARE: [&str; 33] = [
    "Reserved",
    "Ethernet (10Mb)",
    "Experimental Ethernet (3Mb)",
    "Amateur Radio AX.25",
    "Proteon ProNET Token Ring",
    "Chaos",
    "IEEE 802 Networks",
    "ARCNET",
    "Hyperchannel",
    "Lanstar",
    "Autonet Short Address",
    "LocalTalk",
    "LocalNet (IBM PCNet or SYTEK LocalNET)",
    "Ultra link",
    "SMDS",
    "Frame Relay",
    "Asynchronous Transmission Mode (ATM)",
    "HDLC",
    "Fibre Channel",
    "Asynchronous Transmission Mode (ATM)",
    "Serial Line",
    "Asynchronous Transmission Mode (ATM)",
    "MIL-STD-188-220",
    "Metricom",
    "IEEE 1394.1995",
    "MAPOS",
    "Twinaxial",
    "EUI-64",
    "HIPARP",
    "IP and ARP over ISO 7816-3",
    "ARPSec",
    "IPsec tunnel",
    "InfiniBand (TM)",
];

const ARP_OP: [&str; 11] = [
    "Reserved",
    "ARP Request",
    "ARP Reply",
    "RARP Request", // RARP 是 ARP 的逆操作，较少使用（DHCP 提供了更好的方式）
    "RARP Reply",
    "DRARP Request",
    "DRARP Reply",
    "DRARP Error",
    "InARP Request",
    "InARP Reply",
    "ARP NAK",
];

impl Debug for ArpHeader {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let hrd = *ARP_HARDWARE
            .get(u16::from_be(self.arp_hrd) as usize)
            .unwrap_or(&"Unknown");
        let pro: &str = match u16::from_be(self.arp_pro) {
            ETH_P_IPV4 => "IPv4",
            ETH_P_ARP => "ARP",
            ETH_P_RARP => "RARP",
            ETH_P_IPV6 => "IPv6",
            _ => "Unknown",
        };
        let op = *ARP_OP
            .get(u16::from_be(self.arp_op) as usize)
            .unwrap_or(&"Unknown");
        f.debug_struct("ArpHeader")
            .field(
                "arp_hrd",
                &format_args!("{} ({})", u16::from_be(self.arp_hrd), hrd),
            )
            .field(
                "arp_pro",
                &format_args!("{} ({})", u16::from_be(self.arp_pro), pro),
            )
            .field("arp_hln", &self.arp_hln)
            .field("arp_pln", &self.arp_pln)
            .field(
                "arp_op",
                &format_args!("{} ({})", u16::from_be(self.arp_op), op),
            )
            .field("arp_sha", &MacAddr::from(self.arp_sha))
            .field("arp_spa", &Ipv4Addr::from(self.arp_spa))
            .field("arp_tha", &MacAddr::from(self.arp_tha))
            .field("arp_tpa", &Ipv4Addr::from(self.arp_tpa))
            .finish()
    }
}
