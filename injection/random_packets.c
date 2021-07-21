/*
 * (c) 2008-2011 Daniel Halperin <dhalperi@cs.washington.edu>
 */
#include <linux/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <tx80211.h>
#include <tx80211_packet.h>

#include "util.h"

static void init_lorcon();

struct lorcon_packet
{
	__le16	fc;
	__le16	dur;
	u_char	addr1[6];
	u_char	addr2[6];
	u_char	addr3[6];
	__le16	seq;
	u_char	payload[0];
} __attribute__ ((packed));

struct tx80211	tx;
struct tx80211_packet	tx_packet;
struct tx80211_packet	tx_packet_31;
struct tx80211_packet	tx_packet_33;
uint8_t *payload_buffer_29;
uint8_t *payload_buffer_31;
uint8_t *payload_buffer_33;

#define PAYLOAD_SIZE	2000000

static inline void payload_memcpy_29(uint8_t *dest, uint32_t length,
		uint32_t offset)
{
	uint32_t i;
	for (i = 0; i < length; ++i) {
		dest[i] = payload_buffer_29[(offset + i) % PAYLOAD_SIZE];
		//printf("%zu  ",dest[i]);
	}
	//printf("\n");
}
static inline void payload_memcpy_31(uint8_t *dest, uint32_t length,
                uint32_t offset)
{
        uint32_t i;
        for (i = 0; i < length; ++i) {
                dest[i] = payload_buffer_31[(offset + i) % PAYLOAD_SIZE];
                //printf("%zu  ",dest[i]);
        }
        //printf("\n");
}
static inline void payload_memcpy_33(uint8_t *dest, uint32_t length,
                uint32_t offset)
{
        uint32_t i;
        for (i = 0; i < length; ++i) {
                dest[i] = payload_buffer_33[(offset + i) % PAYLOAD_SIZE];
                //printf("%zu  ",dest[i]);
        }
        //printf("\n");
}



int main(int argc, char** argv)
{
	uint32_t num_packets;
	uint32_t packet_size;
	struct lorcon_packet *packet;
	struct lorcon_packet *packet_31;
	struct lorcon_packet *packet_33;
	uint32_t i;
	int32_t ret;
	uint32_t mode;
	uint32_t delay_us;
	struct timespec start, now,before;
	int32_t diff;

	/* Parse arguments */
	if (argc > 5) {
		printf("Usage: random_packets <number> <length> <mode: 0=my MAC, 1=injection MAC> <delay in us>\n");
		return 1;
	}
	if (argc < 5 || (1 != sscanf(argv[4], "%u", &delay_us))) {
		delay_us = 0;
	}
	if (argc < 4 || (1 != sscanf(argv[3], "%u", &mode))) {
		mode = 0;
		printf("Usage: random_packets <number> <length> <mode: 0=my MAC, 1=injection MAC> <delay in us>\n");
	} else if (mode > 1) {
		printf("Usage: random_packets <number> <length> <mode: 0=my MAC, 1=injection MAC> <delay in us>\n");
		return 1;
	}
	if (argc < 3 || (1 != sscanf(argv[2], "%u", &packet_size)))
		packet_size = 2200;
	if (argc < 2 || (1 != sscanf(argv[1], "%u", &num_packets)))
		num_packets = 10000;

	/* Generate packet payloads */
	printf("Generating packet payloads \n");
	payload_buffer_29 = malloc(PAYLOAD_SIZE);
	payload_buffer_31 = malloc(PAYLOAD_SIZE);
	payload_buffer_33 = malloc(PAYLOAD_SIZE);
	if (payload_buffer_29 == NULL) {
		perror("malloc payload buffer");
		exit(1);
	}
	generate_payloads_29(payload_buffer_29, PAYLOAD_SIZE);
	generate_payloads_31(payload_buffer_31, PAYLOAD_SIZE);
	generate_payloads_33(payload_buffer_33, PAYLOAD_SIZE);

	/* Setup the interface for lorcon */
	printf("Initializing LORCON\n");
	init_lorcon();

	/* Allocate packet */
	packet = malloc(sizeof(*packet) + packet_size);
	packet_31 = malloc(sizeof(*packet) + packet_size+2);
	packet_33 = malloc(sizeof(*packet) + packet_size+4);
	if (!packet) {
		perror("malloc packet");
		exit(1);
	}
	packet->fc = (0x08 /* Data frame */
				| (0x0 << 8) /* Not To-DS */);
	packet_31->fc = (0x08 /* Data frame */
				| (0x0 << 8) /* Not To-DS */);
	packet_33->fc = (0x08 /* Data frame */
                                | (0x0 << 8) /* Not To-DS */);

	packet->dur = 0xffff;
	packet_31->dur = 0xffff;
	packet_33->dur = 0xffff;
	if (mode == 0) {
		memcpy(packet->addr1, "\x00\x16\xea\x12\x34\x56", 6);
		get_mac_address(packet->addr2, "mon0");
		memcpy(packet->addr3, "\x00\x16\xea\x12\x34\x56", 6);
	} else if (mode == 1) {
		memcpy(packet->addr1, "\x00\x16\xea\x12\x34\x56", 6);
		memcpy(packet_31->addr1, "\x00\x16\xea\x12\x34\x56", 6);
		memcpy(packet_33->addr1, "\x00\x16\xea\x12\x34\x56", 6);
		memcpy(packet->addr2, "\x00\x16\xea\x12\x34\x56", 6);
		memcpy(packet_31->addr2, "\x00\x16\xea\x12\x34\x56", 6);
		memcpy(packet_33->addr2, "\x00\x16\xea\x12\x34\x56", 6);
//		memcpy(packet->addr3, "\xff\xff\xff\xff\xff\xff", 6);
		get_mac_address(packet->addr3, "mon0");
		get_mac_address(packet_31->addr3, "mon0");
		get_mac_address(packet_33->addr3, "mon0");
	}
	packet->seq = 0;
	packet_31->seq = 0;
	packet_33->seq = 0;
	tx_packet.packet = (uint8_t *)packet;
	tx_packet_31.packet = (uint8_t *)packet_31;
	tx_packet_33.packet = (uint8_t *)packet_33;
	tx_packet.plen = sizeof(*packet) + packet_size;
	tx_packet_31.plen = sizeof(*packet_31) + packet_size+2;
	tx_packet_33.plen = sizeof(*packet_33) + packet_size+4;
	int num_robot = 3;
	/* Send packets */
	printf("Sending %u packets of size %u (. every thousand)\n", num_packets, packet_size);
	if (delay_us) {
		/* Get start time */
		clock_gettime(CLOCK_MONOTONIC, &start);
	}
	for (i = 0; i < num_packets; ++i) {
		clock_gettime(CLOCK_MONOTONIC, &before);
		payload_memcpy_29(packet->payload, packet_size,
				(i*packet_size) % PAYLOAD_SIZE);
		payload_memcpy_31(packet_31->payload, packet_size+2,
                                (i*(packet_size+2)) % PAYLOAD_SIZE);
		payload_memcpy_33(packet_33->payload, packet_size+4,
                                (i*(packet_size+4)) % PAYLOAD_SIZE);

		if (delay_us) {
			clock_gettime(CLOCK_MONOTONIC, &now);
			diff = (now.tv_sec - before.tv_sec) * 1000000 +
			       (now.tv_nsec - before.tv_nsec + 500) / 1000;
			//diff = delay_us*(i+0.33) - diff;
			//printf("%d\n",diff);
			if (diff > 0 && diff < (delay_us/num_robot))
				usleep(delay_us/num_robot-diff);
		}

		ret = tx80211_txpacket(&tx, &tx_packet);
		//printf("Sent first\n");

		 clock_gettime(CLOCK_MONOTONIC, &before);
		 if (delay_us) {
                        clock_gettime(CLOCK_MONOTONIC, &now);
                        diff = (now.tv_sec - before.tv_sec) * 1000000 +
                               (now.tv_nsec - before.tv_nsec + 500) / 1000;
                        //diff = delay_us*(i+0.66) - diff;
                        if (diff > 0 && diff < (delay_us/num_robot))
                                usleep(delay_us/num_robot-diff);
                }
                ret = tx80211_txpacket(&tx, &tx_packet_31);
		//printf("Sent second\n");

		clock_gettime(CLOCK_MONOTONIC, &before);
		if (delay_us) {
                        clock_gettime(CLOCK_MONOTONIC, &now);
                        diff = (now.tv_sec - before.tv_sec) * 1000000 +
                               (now.tv_nsec - before.tv_nsec + 500) / 1000;
                      //  diff = delay_us*(i+1) - diff;
                        if (diff > 0 && diff < (delay_us/num_robot))
                                usleep(delay_us/num_robot-diff);
                }
                ret = tx80211_txpacket(&tx, &tx_packet_33);
               // printf("Sent third\n");

		if (ret < 0) {
			fprintf(stderr, "Unable to transmit packet: %s\n",
					tx.errstr);
			exit(1);
		}

		if (((i+1) % 1000) == 0) {
			printf(".");
			fflush(stdout);
		}
		if (((i+1) % 50000) == 0) {
			printf("%dk\n", (i+1)/1000);
			fflush(stdout);
		}
	}

	return 0;
}

static void init_lorcon()
{
	/* Parameters for LORCON */
	int drivertype = tx80211_resolvecard("iwlwifi");

	/* Initialize LORCON tx struct */
	if (tx80211_init(&tx, "mon0", drivertype) < 0) {
		fprintf(stderr, "Error initializing LORCON: %s\n",
				tx80211_geterrstr(&tx));
		exit(1);
	}
	if (tx80211_open(&tx) < 0 ) {
		fprintf(stderr, "Error opening LORCON interface\n");
		exit(1);
	}

	/* Set up rate selection packet */
	tx80211_initpacket(&tx_packet);
}

