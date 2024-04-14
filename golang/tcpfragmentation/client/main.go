package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"net"
	"time"
)

func main() {
	data := []byte("[Hello, World!]")
	magicNum := make([]byte, 4)
	binary.BigEndian.PutUint32(magicNum, 0x12345678)
	dataLenArray := make([]byte, 2)
	binary.BigEndian.PutUint16(dataLenArray, uint16(len(data)))
	packageBuf := bytes.NewBuffer(magicNum)
	packageBuf.Write(dataLenArray)
	packageBuf.Write(data)
	conn, err := net.DialTimeout("tcp", "localhost:8080", time.Second*30)
	if err != nil {
		fmt.Println("Connection error:", err)
		return
	}
	defer conn.Close()
	for i := 0; i < 100; i++ {
		_, err := conn.Write(packageBuf.Bytes())
		if err != nil {
			fmt.Println("Write to server failed:", err)
			break
		}
	}
}
