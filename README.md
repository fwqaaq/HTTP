# Study network coding

Ethernet II header is 14 bytes long:

* 6 bytes for the `destination` MAC address
* 6 bytes for the `source` MAC address
* 2 bytes for the `type` field

| type | flag |
| ---- | ---- |
| IPv4 | 0x0800|
| IPv6 | 0x86DD|
| ARP | 0x0806|
| RARP | 0x8035|
| MPLS | 0x8847(unicast) 0x8848(multicast)|
| PPPoE Discovery Stage | 0x8863|
| PPPoE Session Stage | 0x8864|
| IEEE 802.1Q VLAN tagging | 0x8100|
| IEEE 802.1X | 0x888E|

## ARP 协议

![ARP](./utils/arp.png)

## IP 协议

![IP](./utils/ip.png)

### ICMP 协议

参见：<https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol>

构建 Ping 工具：[Echo Request](https://en.wikipedia.org/wiki/Ping_(networking_utility)#Echo_request)、[Echo Reply](https://en.wikipedia.org/wiki/Ping_(networking_utility)#Echo_reply)

## HTTP Proxy with the hyper library

## HTTPS Server with the hyper library

```bash
openssl req -x509 -newkey rsa:4096 -nodes -sha256 -subj /CN=localhost -keyout ssl/private.pem -out ssl/cert.pem 
```

## smart_chat

Got inspired by this [project](https://github.com/antirez/smallchat)
