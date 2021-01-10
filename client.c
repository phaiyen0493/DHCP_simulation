/*Name: Yen Pham
CS3530
Programming Assignment 5: Simulate the working of DHCP server using a client-server architecture.
*/

#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <signal.h>
#include <wchar.h>
#include <time.h> 
#include <math.h>
#include <signal.h> 
#include "dhcp_packet.h"

void send_dhcp_packet(int socket_fd)
{
	char message[50];
	struct dhcp_packet *DHCP_segment;
	struct in_addr ip_address;
	unsigned int temp_transaction_ID;
	int rec_bytes;
	int sent_bytes;
	bool toCont = true;
	
	while (toCont) //continue until message is "quit"
	{	
		DHCP_segment = malloc(sizeof(struct dhcp_packet));
		struct in_addr ip_address;

		//Prepare DHCP Discover packet
		inet_aton("129.120.151.94", &ip_address);	
		DHCP_segment->siaddr = ntohl(ip_address.s_addr);
		
		inet_aton("0.0.0.0", &ip_address);	
		DHCP_segment->yiaddr = ntohl(ip_address.s_addr);

		srand(time(NULL));
		DHCP_segment->tran_ID = rand()% (int) (pow(2,32));
		DHCP_segment->lifetime = 0;

		sent_bytes = write(socket_fd, DHCP_segment, sizeof(struct dhcp_packet));//send DHCP Discover packet to server
		if (sent_bytes <= 0)
		{
			printf("\nClient failed to send DHCP Discover packet to server\n");
		}
		else
		{
			printf("\nDHCP Discover packet was sent to server\n");

			ip_address.s_addr = htonl(DHCP_segment->siaddr);
			printf("Server ip address: %s\n", inet_ntoa(ip_address));

			ip_address.s_addr = htonl(DHCP_segment->yiaddr);
			printf("Client ip address: %s\n", inet_ntoa(ip_address));

			printf("Transaction ID: %u\n", DHCP_segment->tran_ID);
			printf("IP lease time: %hu secs\n", DHCP_segment->lifetime);
		}

		//Get DHCP offer packet
		memset(DHCP_segment, 0, sizeof(struct dhcp_packet));
		rec_bytes = read(socket_fd, DHCP_segment, sizeof(struct dhcp_packet));

		if (rec_bytes > 0)
		{
			printf("\nReceived DHCP Offer packet from server\n");

			ip_address.s_addr = htonl(DHCP_segment->siaddr);
			printf("Server ip address: %s\n", inet_ntoa(ip_address));

			unsigned int client_ip = DHCP_segment->yiaddr;
			ip_address.s_addr = htonl(DHCP_segment->yiaddr);
			printf("Client ip address: %s\n", inet_ntoa(ip_address));

			temp_transaction_ID = DHCP_segment->tran_ID;
			printf("Transaction ID: %u\n", DHCP_segment->tran_ID);
			printf("IP lease time: %hu secs\n", DHCP_segment->lifetime);

			//Prepare DHCP Request packet to sent to server
			memset(DHCP_segment, 0, sizeof(struct dhcp_packet));
			inet_aton("129.120.151.94", &ip_address);	
			DHCP_segment->siaddr = ntohl(ip_address.s_addr);
		
			DHCP_segment->yiaddr = client_ip;
			DHCP_segment->tran_ID = temp_transaction_ID + 1;
			DHCP_segment->lifetime = 3600;

			sent_bytes = write(socket_fd, DHCP_segment, sizeof(struct dhcp_packet));//send DHCP 
			if (sent_bytes <= 0)
			{
				printf("\nClient failed to send DHCP Request packet to server\n");
			}
			else
			{
				//print dhcp packet info
				printf("\nDHCP Request packet was sent to server\n");

				ip_address.s_addr = htonl(DHCP_segment->siaddr);
				printf("Server ip address: %s\n", inet_ntoa(ip_address));

				ip_address.s_addr = htonl(DHCP_segment->yiaddr);
				printf("Client ip address: %s\n", inet_ntoa(ip_address));

				printf("Transaction ID: %u\n", DHCP_segment->tran_ID);
				printf("IP lease time: %hu secs\n", DHCP_segment->lifetime);
			}

			//Receive DHCP ACK packet
			memset(DHCP_segment, 0, sizeof(struct dhcp_packet));
			rec_bytes = read(socket_fd, DHCP_segment, sizeof(struct dhcp_packet));

			if (rec_bytes > 0)
			{
				//print DHCP packet info
				printf("\nReceived DHCP ACK packet from server\n");

				ip_address.s_addr = htonl(DHCP_segment->siaddr);
				printf("Server ip address: %s\n", inet_ntoa(ip_address));

				ip_address.s_addr = htonl(DHCP_segment->yiaddr);
				printf("Client ip address: %s\n", inet_ntoa(ip_address));

				printf("Transaction ID: %u\n", DHCP_segment->tran_ID);
				printf("IP lease time: %hu secs\n", DHCP_segment->lifetime);
			}
			else
			{
				printf("\nCannot receive DHCP ACK packet from server\n");
			}	
		}
		else
		{
			printf("\nCannot receive DHCP Offer packet from server\n");
		}

		//read message from console
		do
		{	
			memset(message, 0, 50);
			scanf("%s", message);
		} while ( strcmp(message, "quit") != 0); 

		if ( strcmp(message, "quit") == 0 )
		{
			sent_bytes = write(socket_fd, message, 50);
			printf("Quitting\n");		
			toCont = false;
		}
	}
}

void sigintHandler(int sig_num) 
{ 
    signal(SIGINT, sigintHandler); 
    printf("\n Cannot be terminated using Ctrl+C \n"); 
} 

int main(int argc, char* argv[])
{
	signal(SIGINT, sigintHandler); 
	int portnum;
	if (argc == 2)
	{
		portnum = atoi(argv[1]);
	}
	else
	{
		printf("Please enter port number of server\n");
		exit(0);
	}

	int length;
	int socket_fd;
	struct sockaddr_in server_addr;
	socket_fd = socket(AF_INET, SOCK_STREAM,0);
	server_addr.sin_port = htons(portnum);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("129.120.151.94");
	
	int sk_option = 1;
	//display if failed to send
	if(setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, (char *) &sk_option, sizeof(sk_option)) < 0 ) 
	{ 
		perror("ERROR: setsockopt failed"); 
		return EXIT_FAILURE; 
	} 
  	
	//connect the server
	if ((connect (socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr))) == -1)
	{
		printf("Connection to socket failed \n");
		exit(EXIT_FAILURE);
	}

	printf("client %d\n", portnum);

	send_dhcp_packet(socket_fd);
}