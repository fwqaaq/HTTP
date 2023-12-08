pub const ETH_ALEN: usize = 6;

pub const ETH_P_IPV4: u16 = 0x0800;
pub const ETH_P_ARP: u16 = 0x0806;
pub const ETH_P_RARP: u16 = 0x8035;
pub const ETH_P_IPV6: u16 = 0x86DD;

mod arp;
mod ether;
mod macaddr;
