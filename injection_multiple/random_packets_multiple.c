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
typedef struct lorcon_packet lorcon_packet_t;

struct tx80211	tx;
struct tx80211_packet	tx_packet;
struct tx80211_packet	tx_packet_31;
struct tx80211_packet	tx_packet_33;
struct tx80211_packet	tx_packet_35;
uint8_t *payload_buffer_29;
uint8_t *payload_buffer_31;
uint8_t *payload_buffer_33;
uint8_t *payload_buffer_35;
uint8_t **payload_buffer_array;
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
static inline void payload_memcpy_35(uint8_t *dest, uint32_t length,
                uint32_t offset)
{
        uint32_t i;
        for (i = 0; i < length; ++i) {
                dest[i] = payload_buffer_35[(offset + i) % PAYLOAD_SIZE];
                //printf("%zu  ",dest[i]);
        }
        //printf("\n");
}static inline void payload_memcpy_frame_count(uint32_t robot_id,uint8_t *dest, uint32_t length,
                uint32_t offset)
{
        uint32_t i;
        for (i = 0; i < length; ++i) {
                dest[i] = payload_buffer_array[robot_id][(offset + i) % PAYLOAD_SIZE];
                // printf("%zu  ",dest[i]);
        }
        //printf("\n");
}


int main(int argc, char** argv)
{
	uint32_t num_packets;
	uint32_t packet_size;
	struct lorcon_packet *packet;
	// struct lorcon_packet *packet_31;
	// struct lorcon_packet *packet_33;
	// struct lorcon_packet *packet_35;

	uint32_t i;
	uint32_t robot_index;
	int32_t ret;
	uint32_t mode;
	uint32_t delay_us;
	struct timespec start, now,before;
	int32_t diff;
	int32_t sleep_interval;
	int num_robot;
	
	/* Parse arguments */
	if (argc > 6) {
		printf("Usage: random_packets <number> <starting_length> <mode: 0=my MAC, 1=injection MAC> <delay in us>  <num_robot>\n");
		return 1;
	}
	if (argc < 5 || (1 != sscanf(argv[4], "%u", &delay_us))) {
		delay_us = 0;
	}
	if (argc < 4 || (1 != sscanf(argv[3], "%u", &mode))) {
		mode = 0;
		printf("Usage: random_packets <number> <starting_length> <mode: 0=my MAC, 1=injection MAC> <delay in us>  <num_robot>\n");
	} else if (mode > 1) {
		printf("Usage: random_packets <number> <starting_length> <mode: 0=my MAC, 1=injection MAC> <delay in us> <num_robot>\n");
		return 1;
	}
	if (argc < 3 || (1 != sscanf(argv[2], "%u", &packet_size)))
		packet_size = 29;
	if (argc < 2 || (1 != sscanf(argv[1], "%u", &num_packets)))
		num_packets = 10000;
	if (argc < 6 || (1 != sscanf(argv[5], "%u", &num_robot))) 
		num_robot = 4;
	printf("Total Robot %d, ",num_robot);
	payload_buffer_array = malloc(sizeof(uint8_t*)*num_robot);
	tx80211_packet_t tx_packet_array[num_robot];

	printf("Generating packet payloads \n");
	for( robot_index = 0; robot_index < num_robot; robot_index++) {
		payload_buffer_array[robot_index] = malloc(PAYLOAD_SIZE);
	}
	/* Generate packet payloads */
	// payload_buffer_29 = malloc(PAYLOAD_SIZE); // RX1, TX1
	// payload_buffer_31 = malloc(PAYLOAD_SIZE); // TX2
	// payload_buffer_33 = malloc(PAYLOAD_SIZE);
	// payload_buffer_35 = malloc(PAYLOAD_SIZE);
	for( robot_index = 0; robot_index < num_robot; robot_index++) {
	// if (payload_buffer_29 == NULL||payload_buffer_31 == NULL||payload_buffer_33 == NULL||payload_buffer_35 == NULL) {
		if(payload_buffer_array[robot_index] == NULL){
			perror("malloc payload buffer");
			exit(1);
			}
	}

	for( robot_index = 0; robot_index < num_robot; robot_index++) {
		generate_payloads_timeframe(payload_buffer_array[robot_index],PAYLOAD_SIZE,packet_size+robot_index*2);
	}
	// generate_payloads_timeframe(payload_buffer_29, PAYLOAD_SIZE,packet_size);
	// generate_payloads_timeframe(payload_buffer_31, PAYLOAD_SIZE,packet_size+2);
	// generate_payloads_timeframe(payload_buffer_33, PAYLOAD_SIZE,packet_size+4);
	// generate_payloads_timeframe(payload_buffer_35, PAYLOAD_SIZE,packet_size+6);
	/* Setup the interface for lorcon */
	printf("Initializing LORCON\n");
	init_lorcon();
	lorcon_packet_t **packet_array = malloc(sizeof(lorcon_packet_t*)*num_robot);
	/* Allocate packet */
	for(robot_index = 0; robot_index < num_robot; robot_index++){
		packet_array[robot_index] = malloc(sizeof(*packet) + packet_size+robot_index*2);
		packet_array[robot_index]->fc = (0x08 /* Data frame */
				| (0x0 << 8) /* Not To-DS */);
		packet_array[robot_index]->dur = 0xffff;
	}
	// packet = malloc(sizeof(*packet) + packet_size);
	// packet_31 = malloc(sizeof(*packet) + packet_size+2);
	// packet_33 = malloc(sizeof(*packet) + packet_size+4);
	// packet_35 = malloc(sizeof(*packet) + packet_size+6);
	// if (!packet) {
	// 	perror("malloc packet");
	// 	exit(1);
	// }
	// packet->fc = (0x08 /* Data frame */
				// | (0x0 << 8) /* Not To-DS */);
	// packet_31->fc = (0x08 /* Data frame */
	// 			| (0x0 << 8) /* Not To-DS */);
	// packet_33->fc = (0x08 /* Data frame */
    //                             | (0x0 << 8) /* Not To-DS */);
	// packet_35->fc = (0x08 /* Data frame */
    //                             | (0x0 << 8) /* Not To-DS */);

	// packet->dur = 0xffff;
	// packet_31->dur = 0xffff;
	// packet_33->dur = 0xffff;
	// packet_35->dur = 0xffff;
	if (mode == 0) {
		// memcpy(packet->addr1, "\x00\x16\xea\x12\x34\x56", 6);
		// get_mac_address(packet->addr2, "mon0");
		// memcpy(packet->addr3, "\x00\x16\xea\x12\x34\x56", 6);
	} else if (mode == 1) {
		for(robot_index = 0; robot_index < num_robot; robot_index++){
			memcpy(packet_array[robot_index]->addr1, "\x00\x16\xea\x12\x34\x56", 6);
			memcpy(packet_array[robot_index]->addr2, "\x00\x16\xea\x12\x34\x56", 6);
			get_mac_address(packet_array[robot_index]->addr3, "mon0");
			packet_array[robot_index]->seq = 0;
		}
				// memcpy(packet->addr1, "\x00\x16\xea\x12\x34\x56", 6);

		// memcpy(packet_31->addr1, "\x00\x16\xea\x12\x34\x56", 6);
		// memcpy(packet_33->addr1, "\x00\x16\xea\x12\x34\x56", 6);
		// memcpy(packet_35->addr1, "\x00\x16\xea\x12\x34\x56", 6);
		// memcpy(packet->addr2, "\x00\x16\xea\x12\x34\x56", 6);
		// memcpy(packet_31->addr2, "\x00\x16\xea\x12\x34\x56", 6);
		// memcpy(packet_33->addr2, "\x00\x16\xea\x12\x34\x56", 6);
		// memcpy(packet_35->addr2, "\x00\x16\xea\x12\x34\x56", 6);
		// memcpy(packet->addr3, "\xff\xff\xff\xff\xff\xff", 6);
		// get_mac_address(packet->addr3, "mon0");
		// get_mac_address(packet_31->addr3, "mon0");
		// get_mac_address(packet_33->addr3, "mon0");
		// get_mac_address(packet_35->addr3, "mon0");
	}
	// packet->seq = 0;
	// packet_31->seq = 0;
	// packet_33->seq = 0;
	// packet_35->seq = 0;
	for(robot_index = 0; robot_index < num_robot; robot_index++){
		tx_packet_array[robot_index].packet = (uint8_t *)packet_array[robot_index];
		tx_packet_array[robot_index].plen =  sizeof(*packet_array[robot_index]) + packet_size+robot_index*2;
		tx_packet_array[robot_index].modulation = 0;
		// printf("Tx-packet length %d \n",tx_packet_array[i].plen);
	}
	// tx_packet.packet = (uint8_t *)packet;
	// tx_packet_31.packet = (uint8_t *)packet_31;
	// tx_packet_33.packet = (uint8_t *)packet_33;
	// tx_packet_35.packet = (uint8_t *)packet_35;
	// tx_packet.plen = sizeof(*packet) + packet_size;
	// tx_packet_31.plen = sizeof(*packet_31) + packet_size+2;
	// tx_packet_33.plen = sizeof(*packet_33) + packet_size+4;
	// tx_packet_35.plen = sizeof(*packet_35) + packet_size+6;
	/* Send packets */
	// printf("tx_len %d",tx_packet_array[0].plen);
	printf("Sending %u packets of size %u (. every thousand)\n", num_packets, packet_size);
	if (delay_us) {
		/* Get start time */
		clock_gettime(CLOCK_MONOTONIC, &start);
	}
	sleep_interval = delay_us/num_robot;
	for (i = 0; i < num_packets; ++i) {
		
		clock_gettime(CLOCK_MONOTONIC, &before);
		for(robot_index = 0; robot_index < num_robot;robot_index++){
			payload_memcpy_frame_count(robot_index, packet_array[robot_index]->payload,
										packet_size+robot_index*2, (i*(packet_size+robot_index*2))%PAYLOAD_SIZE);
		// }
		// payload_memcpy_29(packet->payload, packet_size,
				// (i*packet_size) % PAYLOAD_SIZE);
		// payload_memcpy_31(packet_31->payload, packet_size+2,
        //                         (i*(packet_size+2)) % PAYLOAD_SIZE);
		// payload_memcpy_33(packet_33->payload, packet_size+4,
        //                         (i*(packet_size+4)) % PAYLOAD_SIZE);
		// payload_memcpy_35(packet_35->payload, packet_size+6,
        //                         (i*(packet_size+6)) % PAYLOAD_SIZE);

		// for(robot_index = 0; robot_index < num_robot;robot_index++) {
		// if (delay_us) {
		/*	clock_gettime(CLOCK_MONOTONIC, &now);
			diff = (now.tv_sec - before.tv_sec) * 1000000 +
			       (now.tv_nsec - before.tv_nsec + 500) / 1000;
			//diff = delay_us*(i+0.33) - diff;
			//printf("%d\n",diff);
			//if (diff > 0 && diff < (delay_us/num_robot))
				usleep(delay_us/num_robot-diff);*/
				struct tx80211_packet this_packet = tx_packet_array[robot_index];
			printf("sending packet with length %d out of %d \n",tx_packet_array[robot_index].plen,num_robot);
			ret =tx80211_txpacket(&tx, &( tx_packet_array[robot_index]));
			if (ret < 0) {
			fprintf(stderr, "Unable to transmit packet: %s\n",
					tx.errstr);
			exit(1);
		}
			usleep(sleep_interval);
		}

		// ret = tx80211_txpacket(&tx, &tx_packet);
		// printf("Send the first\n");
				
			// printf("Send the to robot %d\n",j);


		//printf("Sent first\n");

		//  clock_gettime(CLOCK_MONOTONIC, &before);
		//  if (delay_us) {
        //                 clock_gettime(CLOCK_MONOTONIC, &now);
        //                 diff = (now.tv_sec - before.tv_sec) * 1000000 +
        //                        (now.tv_nsec - before.tv_nsec + 500) / 1000;
        //                 //diff = delay_us*(i+0.66) - diff;
        //                 if (diff > 0 && diff < (delay_us/num_robot))
        //                         usleep(delay_us/num_robot-diff);
        //         }
        //         ret = tx80211_txpacket(&tx, &tx_packet_31);
		// //printf("Sent second\n");

		// clock_gettime(CLOCK_MONOTONIC, &before);
		// if (delay_us) {
        //                 clock_gettime(CLOCK_MONOTONIC, &now);
        //                 diff = (now.tv_sec - before.tv_sec) * 1000000 +
        //                        (now.tv_nsec - before.tv_nsec + 500) / 1000;
        //               //  diff = delay_us*(i+1) - diff;
        //                 if (diff > 0 && diff < (delay_us/num_robot))
        //                         usleep(delay_us/num_robot-diff);
        //         }
        //         ret = tx80211_txpacket(&tx, &tx_packet_33);
        //        // printf("Sent third\n");
		
		// clock_gettime(CLOCK_MONOTONIC, &before);
		// if (delay_us) {
        //                 clock_gettime(CLOCK_MONOTONIC, &now);
        //                 diff = (now.tv_sec - before.tv_sec) * 1000000 +
        //                        (now.tv_nsec - before.tv_nsec + 500) / 1000;
        //               //  diff = delay_us*(i+1) - diff;
        //                 if (diff > 0 && diff < (delay_us/num_robot))
        //                         usleep(delay_us/num_robot-diff);
        //         }
        //         ret = tx80211_txpacket(&tx, &tx_packet_35);
        //        // printf("Sent third\n");

		// if (ret < 0) {
		// 	fprintf(stderr, "Unable to transmit packet: %s\n",
		// 			tx.errstr);
		// 	exit(1);
		// }
		// }
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

