package main

import (
	"context"
	"errors"
	"net"
	"net/http"
)

var (
	ErrorBlockedHost = errors.New("Host Blocked By Policy")
)

var blockedNet = []*net.IPNet{
	&net.IPNet{
		IP:   net.ParseIP("10.0.0.0"),
		Mask: net.CIDRMask(8, 32),
	},
	&net.IPNet{
		IP:   net.ParseIP("169.254.0.0"),
		Mask: net.CIDRMask(16, 32),
	},
}

func isBadIP(ip net.IP) bool {
	for _, n := range blockedNet {
		if n.Contains(ip) {
			return true
		}
	}
	return false
}

func SafeDialContext(ctx context.Context, network, address string) (net.Conn, error) {
	host, _, err := net.SplitHostPort(address)
	if err != nil {
		return nil, err
	}
	if ips, err := net.LookupIP(host); err != nil {
		return nil, err
	} else {
		for _, ip := range ips {
			if isBadIP(ip) {
				return nil, ErrorBlockedHost
			}
		}
	}
	var d net.Dialer
	return d.DialContext(ctx, network, address)
}

func SafeDial(network, address string) (net.Conn, error) {
	return SafeDialContext(context.TODO(), network, address)
}

func GetSafeHTTPClient() *http.Client {
	t := http.DefaultTransport.(*http.Transport)
	t = t.Clone()
	t.DialContext = SafeDialContext
	return &http.Client{
		Transport: t,
	}
}
