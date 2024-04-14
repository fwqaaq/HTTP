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

## ARP protocol

![ARP](./utils/arp.png)

## IP protocol

![IP](./utils/ip.png)

### ICMP protocol

Reference: <https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol>

Build Ping: [Echo Request](https://en.wikipedia.org/wiki/Ping_(networking_utility)#Echo_request), [Echo Reply](https://en.wikipedia.org/wiki/Ping_(networking_utility)#Echo_reply)

## HTTP/HTTPS Proxy with the hyper library

> [!NOTE]
> Because HTTPS is encrypted, we can only see the CONNECT method with HTTPS connections in HTTP proxies, and we cannot see the real request. And the server cannot use HTTPS, it is generally HTTPS will be upgraded to the CONNECT method, and then use the HTTP protocol to communicate.

Using HTTPS server requires special client support, generally IOS and other devices do not support it.

MIMT -> [Go Proxy](./mitm/main.go), but it is still TCP connecting with the client, and server message is like this: TCP -> HTTP -> TLS (Proxy Request). Only maybe attacker can see the real request for MIMT.

## HTTPS

> [!WARNING]
> If you use `0.0.0.0` as server address, and `localhost` has port conflict with it, then it will not report the error that the port have already been used (no error at all).

```bash
openssl req -x509 -newkey rsa:4096 -nodes -sha256 -subj /CN=localhost -keyout ssl/private.pem -out ssl/cert.pem 
```

## smart_chat

Got inspired by this [project](https://github.com/antirez/smallchat)

## What is TCP packet fragmentation?

Please see the [this document](./doc/TCP_fragmentation.md)
