/*
 * (c) 2008-2011 Daniel Halperin <dhalperi@cs.washington.edu>
 */
#ifndef __UTIL_H__
#define __UTIL_H__

#include <sys/types.h>
#include <stdint.h>

void generate_payloads_29(uint8_t *buffer, size_t buffer_size);
void generate_payloads_31(uint8_t *buffer, size_t buffer_size);
void generate_payloads_33(uint8_t *buffer, size_t buffer_size);

void generate_payloads_count(uint8_t *buffer, size_t buffer_size);

int get_mac_address(uint8_t *buf, const char *ifname);

#endif	/* __UTIL_H__ */
