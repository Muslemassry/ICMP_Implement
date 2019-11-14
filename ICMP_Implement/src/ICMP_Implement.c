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

// timeout for socket to wait for a receive command
#define MAX_TIMEOUT = 1

// in order to exit the ping loop once CTR+C is pressed
int to_be_interrupted = 1;

struct in_addr* get_host_address (char * host_name);
void print_fatal_msg(char *msg);
void print_fatal_msgs(char *msg1, char *msg2);
int char_strlen(const char *str);
void send_ping(int socket, struct sockaddr_in address, char* to_ip);
void do_interrupt(int value);

int main(void) {
	struct hostent *host_info;
	struct in_addr *address;
	char host_name[1024];
	char buffer[1024];

	// times for the total report ------>> tfstart, tfend
	struct timespec tfstart, tfend;

/*	printf("enter host name:\n");
	scanf("%s", &host_name);*/

	// the following to lines are for testing
	strcpy(host_name, "smtp.wp.pl");
	printf("the host name is: %s\n", host_name);


	char * host_name_ptr = host_name;
	struct in_addr* distination_add = get_host_address(host_name_ptr);
	printf("%s has address %s\n", host_name, inet_ntoa(*distination_add));

	// open a socket as row... needs super user to run because it is row socket
	int a_row_socket = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);
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
	send_ping(a_row_socket, sockaddr_in_destination, inet_ntoa(*distination_add));

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

void send_ping(int socket, struct sockaddr_in address, char* to_ip) {
	int packet_icmp_seq, packet_transmitted, packet_received, packet_loss;

	// times for each packet ----------->> time_start, time_end
	struct timespec time_start, time_end;

	printf("hello");
}
