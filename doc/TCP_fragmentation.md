# TCP packet fragmentation

>[!TIP]
>First, why do we need to set Maximum Transmission Unit (MTU) to 1492 bytes?

A standard Ethernet frame is a maximum size of `1518` bytes. The **source MAC** address(6B), **destination MAC** address(6B), **type** field(2B), and **FCS** (Frame Check Sequence, 4B) altogether consist of `18` bytes. But why do we need to set the MTU to `1492` bytes instead of `1500` bytes? Because the Ethernet is going over PPPOE (DSL) which has an overhead(extra routing information) of `8` bytes. So, `1500 - 8 = 1492` bytes.

In general though the TCP is **not a packet protocol**. It is a **stream protocol** and has no requirement to provide you with a 1:1 relationship to the physical packets sent or received.(Maybe there are enough messages to maximize the size of IP packet for the given MTH).

So if you send two packets(i.e. call `send` twice), the receiver might only receive 1 packet (the receiving TCP stack might combine them into one packet).

Please keep in mind that TCP is a stream protocol instead of a packet protocol. You get a stream of data, not packets.

So you need to implement your own packet protocol on top of TCP if you want to send packets. For example, you can use a fixed-size magic number to indicate the start of a packet, and a fixed-size length field to indicate the length of the packet. Follow the example below: (Please see the [detailed code](../golang/tcpfragmentation/client/main.go))

```go
data := []byte("[Hello, World!]")
magicNum := make([]byte, 4)
binary.BigEndian.PutUint32(magicNum, 0x12345678)
dataLenArray := make([]byte, 2)
binary.BigEndian.PutUint16(dataLenArray, uint16(len(data)))
packageBuf := bytes.NewBuffer(magicNum)
packageBuf.Write(dataLenArray)
packageBuf.Write(data)
```

See: <https://stackoverflow.com/questions/756765/when-will-a-tcp-network-packet-be-fragmented-at-the-application-layer>
