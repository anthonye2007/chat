/* File: chat.h
 * Author: CS 3470
 * Description: Header file for chat v2 
*/


#include <stdint.h> /* Needed for unsigned types */
#include <arpa/inet.h> /* Needed for in_addr_t address format */

#define MAX_DATA_LEN 140 /* So we are on 16-bit boundary */


/* You must send this packet across the socket.  Notice there are
 * no pointers inside this packet. Why?*/
struct chat_packet {
	u_short version; /* 16 bits -- Set to version 2 in code */
	struct in_addr src_addr; /* 32 bits */
	struct in_addr dest_addr; /* 32 bits */
	char data[MAX_DATA_LEN]; /* Message goes in here */
	u_short checksum; /* output of chksum() goes here */
};

/* Remember, you must use htons() when setting the version and checksum
 * before you send the char_packet over the socket.
 * Conversly, you must use ntohs() when reading the version and checksum
 * after you receive the char_packet over the socket.
 */
