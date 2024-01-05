package main

import (
	"bufio"
	"crypto/rand"
	"crypto/rsa"
	"crypto/tls"
	"crypto/x509"
	"crypto/x509/pkix"
	"encoding/json"
	"encoding/pem"
	"errors"
	"fmt"
	"io"
	"math/big"
	"net"
	"net/http"
	"os"
	"strings"
	"sync"
	"time"
)

const (
	cert = "/Users/feiwu/Project/rust/HTTP/ssl/cert.pem"
	key  = "/Users/feiwu/Project/rust/HTTP/ssl/private.pem"
)

var dohList = []string{
	"https://1.1.1.1/dns-query",
	"https://1.0.0.1/dns-query",
	"https://8.8.8.8/resolve",
	"https://8.8.4.4/resolve",
	"https://223.6.6.6/resolve",
	"https://223.5.5.5/resolve",
	"https://1.12.12.12/resolve",
	"https://120.53.53.53/resolve",
}

var certCache sync.Map
var rootCert *x509.Certificate
var caPrivate any

type Answer struct {
	Name string `json:"name"`
	Type int    `json:"type"`
	TTL  int    `json:"TTL"`
	Data string `json:"data"`
}

type Data struct {
	Answer []Answer `json:"answer"`
}

func init() {
	// Read CA certificate for signing
	certBytes, err := os.ReadFile(cert)
	if err != nil {
		panic("[ERROR] Failed to read CA certificate: " + err.Error())
	}
	certBlock, _ := pem.Decode(certBytes)
	rootCert, err = x509.ParseCertificate(certBlock.Bytes)
	if err != nil {
		panic("[ERROR] Failed to parse CA certificate: " + err.Error())
	}
	keyBytes, err := os.ReadFile(key)
	if err != nil {
		panic("[ERROR] Failed to read CA private key: " + err.Error())
	}
	keyBlock, _ := pem.Decode(keyBytes)
	caPrivate, err = x509.ParsePKCS8PrivateKey(keyBlock.Bytes)
	if err != nil {
		panic("[ERROR] Failed to assert private key type")
	}
}

func dnsQuery(doh, domain, recordType string) ([]string, error) {
	req, _ := http.NewRequest("GET", fmt.Sprintf("%s?name=%s&type=%s", doh, domain, recordType), nil)
	req.Header.Set("Accept", "application/dns-json")
	res, err := http.DefaultClient.Do(req)
	if err != nil {
		return nil, err
	}

	body, err := io.ReadAll(res.Body)
	if err != nil {
		return nil, err
	}

	var data Data
	if json.Unmarshal(body, &data) != nil {
		return nil, err
	}

	answer := data.Answer
	result := []string{}
	for _, a := range answer {
		if a.Type == 28 || a.Type == 1 {
			result = append(result, a.Data)
		}
	}
	if len(result) == 0 {
		return result, fmt.Errorf("[ERROR] Don't have %s record", domain)
	}

	return result, nil
}

// Build a connection to the target host
func connFast(host, port string) (net.Conn, error) {
	conns := make(chan net.Conn, 4)

	var ipUsed sync.Map
	for _, doh := range dohList {
		for _, t := range []string{"AAAA", "A"} {
			go func(doh, t string) {
				ips, err := dnsQuery(doh, host, t)
				if err != nil {
					fmt.Printf("[ERROR] Failed to query dns: %v \n", err)
					return
				}
				for _, ip := range ips {
					if _, ok := ipUsed.LoadOrStore(ip, true); ok {
						continue
					}
					go func(ip string) {
						conn, err := net.DialTimeout("tcp", net.JoinHostPort(ip, port), 20*time.Second)
						if err != nil {
							fmt.Printf("[ERROR] Failed to connect to target: %v \n", err)
							return
						}
						conns <- conn
					}(ip)
				}
			}(doh, t)
		}
	}
	conn := <-conns
	fmt.Printf("[INFO] Connect: %v -> %v \n", host, conn.RemoteAddr().String())
	return conn, nil
}

func getCertificate(info *tls.ClientHelloInfo) (*tls.Certificate, error) {
	// sni flag
	target := info.ServerName
	if target == "" {
		err := errors.New("[ERROR] Don't have sni")
		return nil, err
	}

	// get cert from cache
	if cert, ok := certCache.Load(target); ok {
		return cert.(*tls.Certificate), nil
	}

	// generate cert
	private, _ := rsa.GenerateKey(rand.Reader, 2048)
	serialNumber, _ := rand.Int(rand.Reader, new(big.Int).Lsh(big.NewInt(1), 128))
	template := &x509.Certificate{
		Subject: pkix.Name{
			CommonName: "localhost",
		},
		SerialNumber:          serialNumber,                        // serial number
		NotBefore:             time.Now().Add(time.Hour * 24 * -7), // start time
		NotAfter:              time.Now().Add(time.Hour * 24 * 72), // end time
		ExtKeyUsage:           []x509.ExtKeyUsage{x509.ExtKeyUsageServerAuth},
		BasicConstraintsValid: true,
		DNSNames:              []string{target},
	}
	derBytes, err := x509.CreateCertificate(rand.Reader, template, rootCert, &private.PublicKey, caPrivate)
	if err != nil {
		panic("[ERROR] Failed to create certificate: " + err.Error())
	}
	cert := &tls.Certificate{
		Certificate: [][]byte{derBytes},
		PrivateKey:  private,
	}
	certCache.Store(target, cert)
	return cert, nil
}

func proxy(conn net.Conn) {
	req, err := http.ReadRequest(bufio.NewReader(conn))
	req.Header.Del("Proxy-Connection")
	if err != nil {
		panic("[ERROR] Failed to read request: " + err.Error())
	}

	// Connect to the target host
	address := req.URL.Host
	if req.Method == "CONNECT" {
		// Tell to the client that the connection was established
		fmt.Fprintf(conn, "HTTP/1.1 200 Connection established\r\n\r\n")
		host, port, err := net.SplitHostPort(address)
		if err != nil {
			panic("[ERROR] Failed to split host and port: " + err.Error())
		}

		// Transparent proxy for HTTPS(TCP Tunnel)
		if port != "443" || net.ParseIP(host) != nil {
			targetConn, err := net.Dial("tcp", address)
			if err != nil {
				fmt.Printf("[ERROR] Failed to connect to target for HTTPS: %v \n", err)
				return
			}
			go io.Copy(targetConn, conn)
			go io.Copy(conn, targetConn)
			return
		}

		fmt.Printf("[INFO] HOST: %s\n", host)
		// MIMT Proxy in here
		config := &tls.Config{
			InsecureSkipVerify: true,
			VerifyPeerCertificate: func(rawCerts [][]byte, _ [][]*x509.Certificate) error {
				// bypass tls verification and manually do it
				certs := make([]*x509.Certificate, len(rawCerts))
				for i, asn1Data := range rawCerts {
					cert, _ := x509.ParseCertificate(asn1Data)
					certs[i] = cert
				}
				opts := x509.VerifyOptions{
					DNSName:       host,
					Intermediates: x509.NewCertPool(),
				}
				for _, cert := range certs[1:] {
					opts.Intermediates.AddCert(cert)
				}
				_, err := certs[0].Verify(opts)
				if err != nil {
					fmt.Printf("[WARN] err: %v\n", err)
				}
				return err
			},
		}
		targetConn, err := connFast(host, "443")
		if err != nil {
			fmt.Printf("[WARN] err: %v\n", err)
			return
		}
		// Build tls connection with client
		targetTlsConn := tls.Client(targetConn, config)
		proxyTlsConn := tls.Server(conn, &tls.Config{GetCertificate: getCertificate})

		go io.Copy(targetTlsConn, proxyTlsConn)
		go io.Copy(proxyTlsConn, targetTlsConn)
		return
	}
	// handle HTTP
	if strings.IndexByte(address, ':') == -1 {
		address += ":80"
	}

	targetConn, err := net.Dial("tcp", address)
	if err != nil {
		fmt.Printf("[ERROR] Failed to connect to target for HTTP: " + err.Error())
		return
	}
	if err := req.Write(targetConn); err != nil {
		fmt.Printf("[ERROR] Failed to write request: " + err.Error())
		return
	}

	go io.Copy(targetConn, conn)
	go io.Copy(conn, targetConn)
}

func main() {
	// tcp server
	listener, err := net.Listen("tcp", ":9000")
	fmt.Printf("[INFO] listen on localhost:9000\n")
	if err != nil {
		panic("[ERROR] Failed to listen: " + err.Error())
	}
	for {
		conn, err := listener.Accept()
		if err != nil {
			panic("[ERROR] Failed to accept: " + err.Error())
		}
		go proxy(conn)
	}
}
