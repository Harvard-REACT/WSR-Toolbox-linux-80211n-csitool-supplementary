/*
 * (c) 2008-2011 Daniel Halperin <dhalperi@cs.washington.edu>
 */
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

static inline uint32_t advance_lfsr(uint32_t lfsr)
{
	return (lfsr << 1) | (((lfsr >> 31) ^ (lfsr >> 29) ^ (lfsr >> 25) ^
				(lfsr >> 24)) & 1);
}

void generate_payloads_29(uint8_t *buffer, size_t buffer_size)
{
	uint32_t lfsr = 0x1f3d5b79;
	uint32_t i;
	unsigned long n = 1;
	for (i = 0; i < buffer_size; ++i) {
		if(i%29 == 0)
			buffer[i] = (n >> 24) & 0xFF;
		else if(i%29 == 1)
                        buffer[i] = (n >> 16) & 0xFF;
		else if(i%29 == 2)
                        buffer[i] = (n >> 8) & 0xFF;
		else if(i%29 == 3 ){
                        buffer[i] = n & 0xFF;
			n = n+1;
		}
		else{
		lfsr = advance_lfsr(lfsr);
		lfsr = advance_lfsr(lfsr);
		lfsr = advance_lfsr(lfsr);
		lfsr = advance_lfsr(lfsr);
		lfsr = advance_lfsr(lfsr);
		lfsr = advance_lfsr(lfsr);
		lfsr = advance_lfsr(lfsr);
		lfsr = advance_lfsr(lfsr);
		buffer[i] = lfsr & 0xff;}
	}
	
}
void generate_payloads_31(uint8_t *buffer, size_t buffer_size)
{
        uint32_t lfsr = 0x1f3d5b79;
        uint32_t i;
        unsigned long n = 1;
        for (i = 0; i < buffer_size; ++i) {
                if(i%31 == 0)
                        buffer[i] = (n >> 24) & 0xFF;
                else if(i%31 == 1)
                        buffer[i] = (n >> 16) & 0xFF;
                else if(i%31 == 2)
                        buffer[i] = (n >> 8) & 0xFF;
                else if(i%31 == 3 ){
                        buffer[i] = n & 0xFF;
                        n = n+1;
                }
                else{
                lfsr = advance_lfsr(lfsr);
                lfsr = advance_lfsr(lfsr);
                lfsr = advance_lfsr(lfsr);
                lfsr = advance_lfsr(lfsr);
                lfsr = advance_lfsr(lfsr);
                lfsr = advance_lfsr(lfsr);
                lfsr = advance_lfsr(lfsr);
                lfsr = advance_lfsr(lfsr);
                buffer[i] = lfsr & 0xff;}
        }
        
}
void generate_payloads_33(uint8_t *buffer, size_t buffer_size)
{
        uint32_t lfsr = 0x1f3d5b79;
        uint32_t i;
        unsigned long n = 1;
        for (i = 0; i < buffer_size; ++i) {
                if(i%33 == 0)
                        buffer[i] = (n >> 24) & 0xFF;
                else if(i%33 == 1)
                        buffer[i] = (n >> 16) & 0xFF;
                else if(i%33 == 2)
                        buffer[i] = (n >> 8) & 0xFF;
                else if(i%33 == 3 ){
                        buffer[i] = n & 0xFF;
                        n = n+1;
                }
                else{
                lfsr = advance_lfsr(lfsr);
                lfsr = advance_lfsr(lfsr);
                lfsr = advance_lfsr(lfsr);
                lfsr = advance_lfsr(lfsr);
                lfsr = advance_lfsr(lfsr);
                lfsr = advance_lfsr(lfsr);
                lfsr = advance_lfsr(lfsr);
                lfsr = advance_lfsr(lfsr);
                buffer[i] = lfsr & 0xff;}
        }

}


void generate_payloads_count(uint8_t *buffer, size_t buffer_size)
{
        uint32_t lfsr = 0x1f3db79;
	unsigned long n = 1;
        uint32_t i;
        for (i = 0; i < buffer_size; i+=4) {
                buffer[i] = (n >> 24) & 0xFF;
		buffer[i+1] = (n >> 16) & 0xFF;
		buffer[i+2] = (n >> 8) & 0xFF;
		buffer[i+3] =n & 0xFF;
        }
}

int get_mac_address(uint8_t *buf, const char *ifname)
{
	int fd;
	struct ifreq ifr;

	/* Open generic socket */
	fd = socket(PF_INET, SOCK_PACKET, htons(ETH_P_ALL));
	if (fd == -1) {
		fprintf(stderr, "Error opening socket on %s to get MAC.\n",
				ifname);
		return 1;
	}

	/* Store interface name */
	strcpy(ifr.ifr_name, ifname);

	/* Get Hardware Address (i.e., MAC) */
	if (-1 == ioctl(fd, SIOCGIFHWADDR, &ifr)) {
		fprintf(stderr, "Error calling SIOCGIFHWADDR to get MAC.\n");
		return 1;
	}

	/* Store it in the return buffer */
	memcpy(buf, ifr.ifr_hwaddr.sa_data, 6);

	/* Close the socket and return success */
	close(fd);
	return 0;
}

