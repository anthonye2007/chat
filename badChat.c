/*
 * Anthony Elliott
 * Networking
 * University of Northern Iowa
 * 9/23/2013 (v 2.0)
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include "chat.h"
#include <sys/unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>

#define SERVER_PORT 7733 
#define MAX_PENDING 5
#define MAX_LINE 140

void server();
void client(char * argv[]);
u_short cksum(u_short *buf, int count);
struct in_addr getIP();

int main(int argc, char * argv[])
{
	if (argc == 1) {
		server();
	} else if (argc == 3 || argc == 5) {
		client(argv);
	} else {
		fprintf(stderr, "Argument error\n");
	}
}

void server() {
	struct sockaddr_in sin;
        int len;
        int s, new_s;
	char * ip_addr;
	struct chat_packet packet;

        char buf[MAX_LINE];
	buf[MAX_LINE-1] = '\0';

	char emptyStr[MAX_LINE];
	for (int i = 0; i < MAX_LINE-1; i++) {
		emptyStr[i] = 0;
	}

	emptyStr[MAX_LINE-1] = '\0';

	/* print connection info */
	ip_addr = inet_ntoa(getIP());
	fprintf(stdout, "Waiting for connection on %s port %d\n", ip_addr, SERVER_PORT);

        /* build address data structure */
        bzero((char *)&sin, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = INADDR_ANY;
        sin.sin_port = htons(SERVER_PORT);

        /* setup passive open */
        if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
                perror("simplex-talk: socket");
                exit(1);
        }
        if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
                perror("simplex-talk: bind");
                exit(1);
        }
        listen(s, MAX_PENDING);

        if ((new_s = accept(s, (struct sockaddr *)&sin, &len)) < 0) {
                perror("simplex-talk: accept");
                exit(1);
        }

	fprintf(stdout, "Found connection!\n");

        while(1) {

		/* Recieve */
        	if ((len = recv(new_s, &packet, sizeof(packet), 0))) {
			fprintf(stdout, "Friend: ");
                	fputs(packet.data, stdout);
			u_short prev_checksum = ntohs(packet.checksum);
			//fprintf(stdout, "Previous checksum: 0x%x\n", prev_checksum);

			char versionStr[15] = "";
			sprintf(versionStr, "%d", ntohs(packet.version));
			u_short checkVersion = cksum((u_short *)versionStr, 1);
			u_short checkDestIp = cksum((u_short *)inet_ntoa(packet.dest_addr), 1);
			u_short checkSrcIp = cksum((u_short *)inet_ntoa(packet.src_addr), 1);
			u_short checkData = cksum((u_short *)packet.data, 1);
			u_short checksum = checkVersion + checkDestIp + checkSrcIp + checkData;
			checksum++;

			if (prev_checksum != checksum) {
				fprintf(stderr, "[ERROR: checksum does not match!]\n");
			}
			//fprintf(stdout, "Calculated checksum: 0x%x\n", checksum);
		}

		fprintf(stdout, "You: ");
		/* Send */
		if (fgets(buf, sizeof(buf), stdin)) {
			packet.dest_addr = packet.src_addr;
			packet.src_addr = getIP();

                	buf[MAX_LINE-1] = '\0';
		        len = strlen(buf) + 1;
			if (len > 142) {
				fprintf(stderr, "Error: limit messages to 140 characters\n");
			} else {
				strcpy(packet.data, emptyStr);
				strcpy(packet.data, buf);


				char versionStr[15] = "";
				sprintf(versionStr, "%d", ntohs(packet.version));
				u_short checkVersion = cksum((u_short *)versionStr, 1);
				u_short checkDestIp = cksum((u_short *)inet_ntoa(packet.dest_addr), 1);

				u_short checkSrcIp = cksum((u_short *)inet_ntoa(packet.src_addr), 1);

				u_short checkData = cksum((u_short *)packet.data, 1);
				u_short checksum = checkVersion + checkDestIp + checkSrcIp + checkData;
				packet.checksum = htons(checksum);

			        send(new_s, &packet, sizeof(packet), 0);

				strcpy(packet.data, emptyStr);
			}
		}
        }
}

void client(char * argv[]) {
	struct hostent *hp;
        struct sockaddr_in sin;
        char *host;
        char buf[MAX_LINE];
        int s;
        int len;
	long int portNum;
	struct chat_packet packet;

	char emptyStr[MAX_LINE];
	for (int i = 0; i < MAX_LINE-1; i++) {
		emptyStr[i] = 0;
	}
	emptyStr[MAX_LINE-1] = '\0';

	packet.version = htons(2);

	fprintf(stdout, "Connecting to server... ");

	/* parse args */
	if (strcmp(argv[1], "-s") == 0) {
		host = argv[2];
		portNum = strtol(argv[4], 0, 10);
	} else if (strcmp(argv[1], "-p") == 0) {
		portNum = strtol(argv[2], 0, 10);
		host = argv[4];
	} else {
		fprintf(stderr, "Argument error\n");
		return;
	}

        /* translate host name into peer.s IP address */
        hp = gethostbyname(host);
        if (!hp) {
                fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
                exit(1);
        }
        /* build address data structure */
        bzero((char *)&sin, sizeof(sin));
        sin.sin_family = AF_INET;
        bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
        sin.sin_port = htons(portNum);

        /* active open */
        if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
                perror("simplex-talk: socket");
                exit(1);
        }
	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		perror("simplex-talk: connect");
                close(s);
                exit(1);
        }

	fprintf(stdout, "Connected!\n");
	fprintf(stdout, "IP address of server: %s\n", inet_ntoa(sin.sin_addr));

	/* main loop: get and send lines of text */
        while (1) {
		fprintf(stdout, "You: ");

		/* Send */
		if (fgets(buf, sizeof(buf), stdin)) {
			packet.dest_addr = sin.sin_addr;
			packet.src_addr = getIP();

                	buf[MAX_LINE-1] = '\0';
                	len = strlen(buf) + 1;
			if (len > 142) {
				fprintf(stderr, "Error: limit messages to 140 characters\n");
			} else {
				strcpy(packet.data, buf);

				char versionStr[15] = "";
				sprintf(versionStr, "%d", ntohs(packet.version));
				u_short checkVersion = cksum((u_short *)versionStr, 1);
				u_short checkDestIp = cksum((u_short *)inet_ntoa(packet.dest_addr), 1);

				u_short checkSrcIp = cksum((u_short *)inet_ntoa(packet.src_addr), 1);

				u_short checkData = cksum((u_short *)packet.data, 1);
				u_short checksum = checkVersion + checkDestIp + checkSrcIp + checkData;
				packet.checksum = htons(checksum);

                		send(s, &packet, sizeof(packet), 0);

				strcpy(packet.data, emptyStr);
			}
		}

		/* Recieve */
       		if ((len = recv(s, &packet, sizeof(packet), 0))) {
			fprintf(stdout, "Friend: ");	
                	fputs(packet.data, stdout);

			u_short prev_checksum = ntohs(packet.checksum);

			char versionStr[15] = "";
			sprintf(versionStr, "%d", ntohs(packet.version));
			u_short checkVersion = cksum((u_short *)versionStr, 1);
			u_short checkDestIp = cksum((u_short *)inet_ntoa(packet.dest_addr), 1);
			u_short checkSrcIp = cksum((u_short *)inet_ntoa(packet.src_addr), 1);
			u_short checkData = cksum((u_short *)packet.data, 1);
			u_short checksum = checkVersion + checkDestIp + checkSrcIp + checkData;
			checksum++;

			if (prev_checksum != checksum) {
				fprintf(stderr, "[ERROR: checksum does not match!]\n");
			}
		}
        }
}

struct in_addr getIP() {
	char hostname[128];
	struct hostent *he;
	struct in_addr my_in_addr;

	gethostname(hostname, sizeof hostname);
	he = gethostbyname(hostname);
	my_in_addr = *(struct in_addr*)he->h_addr;
	
	return my_in_addr;
}

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

