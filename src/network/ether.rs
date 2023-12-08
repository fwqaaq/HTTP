use crate::network::macaddr::MacAddr;
use crate::network::{ETH_ALEN, ETH_P_ARP, ETH_P_IPV4, ETH_P_IPV6, ETH_P_RARP};
use std::fmt::Debug;

#[repr(C, packed)]
#[derive(Copy, Clone)]
pub struct EtherHeader {
    pub ether_dhost: [u8; ETH_ALEN], // 目标 MAC 地址
    pub ether_shost: [u8; ETH_ALEN], // 源 MAC 地址
    pub ether_type: u16,             // 指定上层协议类型（例如，IP）
}

impl Debug for EtherHeader {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let r#type = match self.ether_type {
            ETH_P_IPV4 => "IPv4",
            ETH_P_ARP => "ARP",
            ETH_P_RARP => "RARP",
            ETH_P_IPV6 => "IPv6",
            _ => "Unknown",
        };
        f.debug_struct("EtherHeader")
            .field("ether_dhost", &MacAddr::from(self.ether_dhost))
            .field("ether_shost", &MacAddr::from(self.ether_shost))
            .field(
                "ether_type",
                &format_args!("{} ({})", u16::from_be(self.ether_type), r#type),
            )
            .finish()
    }
}

#[derive(Debug, Clone)]
pub struct EtherClient {}
