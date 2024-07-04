# TCP

> When we want to build a TCP server in the Rust, JavaScript and other high-level languages, we can use the standard lib to build the server easily.

1. First, we need to create a TCP listener, and bind it to a port.
2. Then, we can accept the incoming connections, and handle the connections in a new thread.
   * In Deno/Rust, get the **incoming connections**(stream) from the `accept()` method of the listener.
   * Beacuse there are many streams in the connections, Rust/Deno provides another method to get the **connection stream**.
      * Rust: `for stream in listener.incoming() {}`
      * Deno: `for await (const stream of listener) {}`
3. Finally, we can *write*/*read* the data from the connection `stream`.(TCP connection is **persistent connection**.)

For example:

```ts
const listener = Deno.listen({
  hostname: '0.0.0.0',
  port: 8080,
  transport: 'tcp',
})
const encoder = new TextEncoder()

async function main() {
  for (;;) {
    const stream = await listener.accept()
    handleStream(stream)
  }
}

async function handleStream(stream: Deno.Conn) {
  stream.write(encoder.encode('Hello World\n'))
  const buffer = new Uint8Array(1024)
  while (true) {
    const n = await stream.read(buffer)
    if (n === null) break
    stream.write(buffer.slice(0, n))
  }
  console.log('connection closed', stream.rid)
  stream.close()
}

main()
```

In Rust with `incoming`: <https://github.com/fwqaaq/HTTP/blob/78b5fa8bb2c0cdb37127dec78a6842e0b47d1fb2/src/small_chat.rs#L102>

## Socket

>Socket is an abstract concept in computer network that represents the endpoint in a network communication between two nodes. In network programming, a socket is a fundamental building block used for sending and receiving data in a network.

In most cases, we never pay attention to the socket, but without exception, the TCP implementation in most language is based on the socket.

* TCP Sockets: These sockets are used for establishing reliable, ordered, and bidirectional.
* UDP Sockets: Compared to TCP sockets, UDP sockets provide a simpler by exchanging data in the form of independent packets(called datagrams). And UDP sockets are not reliable and ordered.

> How to create a TCP server with socket in C?

1. **Creating a Socket**: Create a socket based on the required communication type (TCP, UDP) and protocol.
2. **Binding the Socket** (optional, typically for servers): Bind the socket to a network address and port.
3. **Listening for Connections** (TCP only, typically for servers): In TCP, the server socket listens for connection requests from clients.
4. **Accepting Connections** (TCP only, typically for servers): The server accepts connection requests from clients to establish communication.
5. **Connecting to a Socket** (typically for clients): The client socket attempts to connect to the server's socket.
6. **Sending and Receiving Data**: Once a connection is established, data can be sent and received through the socket.
7. **Closing the Socket**: After communication is complete, the socket is closed to free up resources.

link: [HTTP example](../c_network/src/tcpserver.c)
