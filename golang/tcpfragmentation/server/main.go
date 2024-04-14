package main

import (
	"bufio"
	"bytes"
	"encoding/binary"
	"fmt"
	"io"
	"net"
)

func main() {
	listen, err := net.Listen("tcp", ":8080")
	if err != nil {
		fmt.Println("Listen error:", err)
	}
	fmt.Println("Server started on port localhost:8080")

	for {
		conn, err := listen.Accept()
		if err != nil {
			fmt.Println("Accept error:", err)
			break
		}
		go handler(conn)
	}
}

func SplitFunc(data []byte, atEOF bool) (advance int, token []byte, err error) {
	if atEOF && len(data) == 0 {
		return 0, nil, nil
	}

	var l int16

	if len(data) >= 6 && binary.BigEndian.Uint32(data[:4]) == 0x12345678 {
		binary.Read(bytes.NewReader(data[4:6]), binary.BigEndian, &l)
		if int(l)+6 <= len(data) {
			return int(l) + 6, data[:6+l], nil
		}
	}
	return
}

func handler(conn net.Conn) {
	defer conn.Close()
	defer fmt.Println("Connection closed")

	fmt.Println("New Connection from:", conn.RemoteAddr())
	var buf [65542]byte
	res := bytes.NewBuffer(nil)
	for {
		n, err := conn.Read(buf[0:])
		res.Write(buf[0:n])
		if err != nil {
			if err == io.EOF {
				continue
			} else {
				fmt.Println("Read error:", err)
				break
			}
		}
		scanner := bufio.NewScanner(res)
		scanner.Split(SplitFunc)
		for scanner.Scan() {
			fmt.Println("Received data:", string(scanner.Bytes()[6:]))
		}
	}
}
