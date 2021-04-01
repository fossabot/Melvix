// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef NET_H
#define NET_H

#include <def.h>
#include <socket.h>

#define htonl(l)                                                                                   \
	((((l)&0xff) << 24) | (((l)&0xff00) << 8) | (((l)&0xff0000) >> 8) |                        \
	 (((l)&0xff000000) >> 24))
#define htons(s) ((((s)&0xff) << 8) | (((s)&0xff00) >> 8))
#define ntohl(l) htonl((l))
#define ntohs(s) htons((s))
#define ip(a, b, c, d)                                                                             \
	((((a)&0xff) << 24) | (((b)&0xff) << 16) | (((c)&0xff) << 8) | (((d)&0xff) << 0))

#define ETHERNET_TYPE_IP4 0x0800
#define ETHERNET_TYPE_IP6 0x86dd
#define ETHERNET_TYPE_ARP 0x0806

#define IP_PROT_ICMP 0x01
#define IP_PROT_TCP 0x06
#define IP_PROT_UDP 0x11

#define TCP_FLAG_FIN (1 << 0)
#define TCP_FLAG_SYN (1 << 1)
#define TCP_FLAG_RST (1 << 2)
#define TCP_FLAG_PSH (1 << 3)
#define TCP_FLAG_ACK (1 << 4)
#define TCP_FLAG_URG (1 << 5)
#define TCP_FLAG_ECE (1 << 6)
#define TCP_FLAG_CWR (1 << 7)
#define TCP_FLAG_NS (1 << 8)

#define ARP_REQUEST 1
#define ARP_REPLY 2

#define DHCP_REQUEST 1
#define DHCP_REPLY 2
#define DHCP_TRANSACTION_IDENTIFIER 0x18122002

#define HARDWARE_TYPE_ETHERNET 0x01

// Hardcoded ports - TODO!
#define DHCP_PORT 68
#define DNS_PORT 50053

// Protocol structs

struct ethernet_packet {
	u8 dst[6];
	u8 src[6];
	u16 type;
	u8 data[];
} PACKED;

struct arp_packet {
	u16 hardware_type;
	u16 protocol;
	u8 hardware_addr_len;
	u8 protocol_addr_len;
	u16 opcode;
	u8 src_mac[6];
	u32 src_protocol_addr;
	u8 dst_mac[6];
	u32 dst_protocol_addr;
} PACKED;

struct ip_packet {
	u8 version_ihl;
	u8 dscp_ecn;
	u16 length;
	u16 id;
	u16 flags_fragment;
	u8 ttl;
	u8 protocol;
	u16 checksum;
	u32 src;
	u32 dst;
	u8 data[];
} PACKED;

struct dhcp_packet {
	u8 op;
	u8 hardware_type;
	u8 mac_len;
	u8 hops;
	u32 xid;
	u16 seconds;
	u16 flags;
	u32 client_ip;
	u32 your_ip;
	u32 server_ip;
	u32 gateway_ip;
	u8 client_mac[6];
	u8 reserved[10];
	u8 server_name[64];
	u8 file[128];
	u8 options[64];
} PACKED;

struct dns_packet {
	u16 qid;
	u16 flags;
	u16 questions;
	u16 answers;
	u16 authorities;
	u16 additional;
	u8 data[];
} PACKED;

struct udp_packet {
	u16 src_port;
	u16 dst_port;
	u16 length;
	u16 checksum;
	u8 data[];
} PACKED;

struct tcp_packet {
	u16 src_port;
	u16 dst_port;
	u32 seq_number;
	u32 ack_number;
	u16 flags;
	u16 window_size;
	u16 checksum;
	u16 urgent;
	u8 data[];
} PACKED;

struct tcp_pseudo_header {
	u32 src;
	u32 dst;
	u8 zeros;
	u8 protocol;
	u16 tcp_len;
	u8 tcp_packet[];
};

struct icmp_packet {
	u8 type;
	u8 version;
	u16 checksum;
	u16 identifier;
	u16 sequence;
} PACKED;

// Other structs

struct arp_table_entry {
	u32 ip_addr;
	u64 mac_addr;
};

void ethernet_handle_packet(struct ethernet_packet *packet, int len) NONNULL;

struct socket *net_open(enum socket_type type);
int net_close(struct socket *socket) NONNULL;
int net_connect(struct socket *socket, u32 ip_addr, u16 dst_port) NONNULL;
void net_send(struct socket *socket, void *data, u32 len) NONNULL;
int net_receive(struct socket *socket, void *buf, u32 len) NONNULL;

int net_installed(void);
void net_install(void);

#endif
