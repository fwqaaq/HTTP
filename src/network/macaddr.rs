use std::fmt::{self, Debug, Display};
use std::str::FromStr;
#[derive(Clone, Copy, PartialEq, Eq)]
pub struct MacAddr([u8; 6]);

impl MacAddr {
    pub const BROADCAST: MacAddr = MacAddr([0xff; 6]);
    pub const ZERO: MacAddr = MacAddr([0; 6]);
}

impl Display for MacAddr {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_fmt(format_args!(
            "{:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}",
            self.0[0], self.0[1], self.0[2], self.0[3], self.0[4], self.0[5]
        ))
    }
}

impl Debug for MacAddr {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        fmt::Display::fmt(&self, f)
    }
}

impl From<[u8; 6]> for MacAddr {
    fn from(mac: [u8; 6]) -> Self {
        MacAddr(mac)
    }
}

impl From<MacAddr> for [u8; 6] {
    fn from(mac: MacAddr) -> Self {
        mac.0
    }
}

impl FromStr for MacAddr {
    type Err = &'static str;

    fn from_str(s: &str) -> std::result::Result<Self, Self::Err> {
        let mut mac = [0u8; 6];
        for (i, byte) in s.split(':').enumerate() {
            if i >= 6 {
                return Err("Invalid MAC address");
            }
            mac[i] = u8::from_str_radix(byte, 16).map_err(|_| "Invalid MAC address")?;
        }
        Ok(MacAddr(mac))
    }
}

#[cfg(test)]
mod test {
    use super::*;
    #[test]
    fn test_mac_to_str() {
        assert_eq!(
            MacAddr([0x16, 0x38, 0x45, 0x54, 0x33, 0xab]).to_string(),
            "16:38:45:54:33:ab"
        )
    }

    #[test]
    fn test_str_to_mac() {
        assert_eq!(
            MacAddr::from_str("16:38:45:54:33:ab").expect("Invalid MAC address"),
            MacAddr([0x16, 0x38, 0x45, 0x54, 0x33, 0xab])
        )
    }
}
