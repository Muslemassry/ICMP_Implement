/*
 ============================================================================
 Name        : PingsSender.c
 Author      : Amaragy
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <netinet/ip_icmp.h>
#include <string.h>
#include <unistd.h>

// timeout for socket to wait for a receive command
#define MAX_TIMEOUT = 1

// in order to exit the ping loop once CTR+C is pressed
int to_be_interrupted = 1;

struct ping_pkt
{
	struct icmphdr hdr;
	char msg[64-sizeof(struct icmphdr)];
};

struct in_addr* get_host_address (char * host_name);
void print_fatal_msg(char *msg);
void print_fatal_msgs(char *msg1, char *msg2);
int char_strlen(const char *str);
void send_ping(int socket, struct sockaddr_in address, char* to_ip, int *ttl);
void do_interrupt(int value);
unsigned short checksum(void *b, int len);

int main(void) {
	struct hostent *host_info;
	struct in_addr *address;
	char host_name[1024];
	char buffer[1024];
	int ttl;
	// times for the total report ------>> tfstart, tfend
	struct timespec tfstart, tfend;

/*	printf("enter host name:\n");
	scanf("%s", &host_name);
	printf("enter ttl:\n");
	scanf("%d", &ttl);*/

	// the following to lines are for testing
	strcpy(host_name, "smtp.wp.pl");
	printf("the host name is: %s\n", host_name);
	ttl = 30;

	char * host_name_ptr = host_name;
	struct in_addr* distination_add = get_host_address(host_name_ptr);
	printf("%s has address %s\n", host_name, inet_ntoa(*distination_add));

	// open a socket as row... needs super user to run because it is row socket
	int a_row_socket = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (a_row_socket == -1) {
		print_fatal_msg("Failed to open a raw socket");
	}
	// prepare the address
	struct sockaddr_in sockaddr_in_destination;
	sockaddr_in_destination.sin_addr = *distination_add;
	sockaddr_in_destination.sin_port = htons(0);

	// set the start time
	clock_gettime(CLOCK_MONOTONIC, &tfstart);

	// send
	send_ping(a_row_socket, sockaddr_in_destination, inet_ntoa(*distination_add), &ttl);

	// set the end time
	clock_gettime(CLOCK_MONOTONIC, &tfend);

	// do the report
	return 0;
}

struct in_addr* get_host_address (char * host_name) {
	struct hostent *host_info;
	struct in_addr* address;
	host_info = gethostbyname(host_name);
	if(host_info == NULL) {
		print_fatal_msgs(host_name, "Not Found");
	} else {
		address = (struct in_addr *) (host_info->h_addr);
	}

	return address;
}

void print_fatal_msg(char *msg) {
    printf("FATAL ERROR: %s\n", msg);
    exit(1);
}

void print_fatal_msgs(char *msg1, char *msg2) {
	printf("FATAL ERROR: %s %s", msg1, msg2);
	exit(1);
}

int char_strlen(const char *str) {
	int i;
	for (i = 0; str[i]; i++);
	return i;
}

void do_interrupt(int value) {
	to_be_interrupted = 0;
}

void send_ping(int socket, struct sockaddr_in address, char* to_ip, int *ttl) {
	int packet_icmp_seq, packet_transmitted, packet_received, packet_loss;
	packet_icmp_seq = 0;
	packet_transmitted = 0;
	packet_received = 0;
	packet_loss = 0;
	struct timeval time_val;
	time_val.tv_sec = 1;
	time_val.tv_usec = 0;
	int has_packet_been_sent = 1;
	struct sockaddr_in received_address;

	// times for each packet ----------->> time_start, time_end
	struct timespec time_start, time_end;

	// set ttl option on the IP
	setsockopt(socket, SOL_IP, IP_TTL, ttl, sizeof(*ttl));

	// set timeout on the socket level to prevent it from being stuck
	setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*) &time_val, sizeof(time_val));
	struct ping_pkt icmp_packet;
	while (to_be_interrupted) { // send echo icmp packets
		//prepare the icmp packet
		bzero(&icmp_packet, sizeof(icmp_packet));
		icmp_packet.hdr.type = ICMP_ECHO;
		icmp_packet.hdr.un.echo.id = getpid();

		// fill the message padding part with random data
		int i;
		for (i = 0; i < sizeof(icmp_packet.msg)-1; i++ ) {
			icmp_packet.msg[i]=i+'0';
		}

		icmp_packet.msg[i]='0';
		icmp_packet.hdr.un.echo.sequence = packet_icmp_seq++;

		// calculate the checksum in order to maintain the integrity of the icmp packet
		icmp_packet.hdr.checksum = checksum(&icmp_packet, sizeof(icmp_packet));

		// wait for the next echo request about half a second
		usleep(500000);
		clock_gettime(CLOCK_MONOTONIC, &time_start);
		if (sendto(socket, &icmp_packet, sizeof(icmp_packet), 0, (struct sockaddr*) &address, sizeof(address)) <= 0) {
			printf("\nPacket Sending Failed!\n");
		    has_packet_been_sent = 0;
		}

		int received_arr_len = sizeof(received_address);
		if (recvfrom(socket, &icmp_packet, sizeof(icmp_packet), 0, (struct sockaddr*)&received_address, &received_arr_len) <= 0 && packet_icmp_seq>1) {
			printf("\nPacket receive failed!\n");
		} else {
			clock_gettime(CLOCK_MONOTONIC, &time_end);
			double timeElapsed = ((double)(time_end.tv_nsec -  time_start.tv_nsec))/1000000.0;

			if(has_packet_been_sent) {
				if(!(icmp_packet.hdr.code == 0)) {
					printf("Error..Packet received with ICMP type %d code %d\n", icmp_packet.hdr.type, icmp_packet.hdr.code);
				} else {
					int size = 64;
					char * inet_ntoa_address = inet_ntoa(address.sin_addr);
					printf("%d bytes from %s (%s) msg_seq=%d ttl=%d r.\n", size, inet_ntoa_address, inet_ntoa_address, packet_icmp_seq, *ttl);
					packet_received++;
				}
			}
		}


	}



	printf("hello");
}

unsigned short checksum(void *b, int len)
{   unsigned short *buf = b;
    unsigned int sum=0;
    unsigned short result;

    for ( sum = 0; len > 1; len -= 2 )
        sum += *buf++;
    if ( len == 1 )
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}
