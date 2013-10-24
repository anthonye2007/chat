/* File: checksum.c
 * Author: CS 3470
 * Description: This program demonstrates the checksum function
 * found in the textbook on page 95
 */ 


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

u_short cksum(u_short *buf, int count)
{
	register u_long sum=0;

	while (count--) {
		sum += *buf++;
		if (sum & 0xFFFF0000)
		{
			/* carry occured, so wrap around */
			sum &= 0xFFFF;
			sum++;
		}
	}

	return ~(sum & 0xFFFF);
}

int main()
{
	u_short checksum;
	char buf[140] = "123456789012345"; //test value

	/* We must cast our buf to a u_short ptr for cksum() */
	checksum = cksum((u_short *)buf, 1);

	/* Print the checksum in hex */
	printf("checksum is 0x%x\n", checksum);

	return 0;
}
