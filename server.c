/*Name: Yen Pham
CS3530
Programming Assignment 5: Simulate the working of DHCP server using a client-server architecture.
*/

#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h> 
#include <netdb.h>
#include <wchar.h>
#include <signal.h> 
#include "dhcp_packet.h"

struct client 
{				
	int socket_fd;//file descriptor
	unsigned int client_ip; //ip
};

//to hold 254 maximum clients
struct client *client_list[254];

//to count the number of clients
int client_count = 0;

void remove_client(int sockfd)
{
	for (int i = 0; i < client_count; i++)
	{
		if (client_list[i] != NULL && client_list[i]->socket_fd == sockfd)
		{
			client_list[i] = NULL; //reset to null to remove the client
			client_count--;
		}
	}
}

void *server_handler(void *c)
{
	char *message = malloc (sizeof(char)*50);
	unsigned int temp_transaction_ID;
	struct client *temp_client = (struct client *) c;
	struct dhcp_packet *DHCP_segment = malloc(sizeof(struct dhcp_packet));
	int rec_bytes;
	int sent_bytes;
	struct in_addr ip_address;
	bool toContinue = true;	

	while (toContinue)
	{
		//Receive DHCP discover packer from client
		memset(DHCP_segment, 0, sizeof(struct dhcp_packet));
		rec_bytes = read(temp_client->socket_fd, DHCP_segment, sizeof(struct dhcp_packet));
	
		if (rec_bytes > 0)
		{
			printf("\n--------------------------------------------\n");
			printf("\nNew client entered\n");
			printf("Received DHCP Discover packet from client\n");
			
			//display server ip address
			ip_address.s_addr = htonl(DHCP_segment->siaddr);
			printf("Server ip address: %s\n", inet_ntoa(ip_address));

			//display client ip address
			ip_address.s_addr = htonl(DHCP_segment->yiaddr);
			printf("Client ip address: %s\n", inet_ntoa(ip_address));

			temp_transaction_ID = DHCP_segment->tran_ID;

			//display transaction id and ip lease time
			printf("Transaction ID: %u\n", DHCP_segment->tran_ID);
			printf("IP lease time: %hu secs\n", DHCP_segment->lifetime);

			//Prepare DHCP Offer Packet
			memset(DHCP_segment, 0, sizeof(struct dhcp_packet));
			inet_aton("129.120.151.94", &ip_address);	
			DHCP_segment->siaddr = ntohl(ip_address.s_addr);
		
			DHCP_segment->yiaddr = temp_client->client_ip;
			DHCP_segment->tran_ID = temp_transaction_ID;
			DHCP_segment->lifetime = 3600;

			sent_bytes = write(temp_client->socket_fd, DHCP_segment, sizeof(struct dhcp_packet));//send DHCP 
			if (sent_bytes <= 0)
			{
				printf("\nServer failed to send DHCP Offer packet to client\n");
			}
			else
			{
				printf("\nDHCP Offer packet was sent to client\n");
				//display server ip address
				ip_address.s_addr = htonl(DHCP_segment->siaddr);
				printf("Server ip address: %s\n", inet_ntoa(ip_address));

				//display client ip address
				ip_address.s_addr = htonl(DHCP_segment->yiaddr);
				printf("Client ip address: %s\n", inet_ntoa(ip_address));

				//display transation id and ip lease time
				printf("Transaction ID: %u\n", DHCP_segment->tran_ID);
				printf("IP lease time: %hu secs\n", DHCP_segment->lifetime);
			}
		}
		else
		{
			printf("\nCannot receive DHCP Discover packet from client\n");
		}

		memset(DHCP_segment, 0, sizeof(struct dhcp_packet));
		rec_bytes = read(temp_client->socket_fd, DHCP_segment, sizeof(struct dhcp_packet));
	
		if (rec_bytes > 0)
		{
			printf("\nReceived DHCP Request packet from client\n");
			//display server ip address
			ip_address.s_addr = htonl(DHCP_segment->siaddr);
			printf("Server ip address: %s\n", inet_ntoa(ip_address));

			//display client ip address
			ip_address.s_addr = htonl(DHCP_segment->yiaddr);
			printf("Client ip address: %s\n", inet_ntoa(ip_address));

			temp_transaction_ID = DHCP_segment->tran_ID;
			printf("Transaction ID: %u\n", DHCP_segment->tran_ID);
			printf("IP lease time: %hu secs\n", DHCP_segment->lifetime);

			//Send back DHCP ACK packet to client
			memset(DHCP_segment, 0, sizeof(struct dhcp_packet));
			inet_aton("129.120.151.94", &ip_address);	
			DHCP_segment->siaddr = ntohl(ip_address.s_addr);
		
			DHCP_segment->yiaddr = temp_client->client_ip;
			DHCP_segment->tran_ID = temp_transaction_ID;
			DHCP_segment->lifetime = 3600;

			sent_bytes = write(temp_client->socket_fd, DHCP_segment, sizeof(struct dhcp_packet));//send DHCP 
			if (sent_bytes <= 0)
			{
				printf("\nServer failed to send DHCP ACK packet to client\n");
			}
			else
			{
				//display info
				printf("\nDHCP ACK packet was sent to client\n");

				ip_address.s_addr = htonl(DHCP_segment->siaddr);
				printf("Server ip address: %s\n", inet_ntoa(ip_address));

				ip_address.s_addr = htonl(DHCP_segment->yiaddr);
				printf("Client ip address: %s\n", inet_ntoa(ip_address));

				printf("Transaction ID: %u\n", DHCP_segment->tran_ID);
				printf("IP lease time: %hu secs\n", DHCP_segment->lifetime);
			}	
		}
		else
		{
			printf("\nCannot receive DHCP Request packet from client\n");
		}

		memset(message, 0, 50);//reset buffer memory
		rec_bytes = recv(temp_client->socket_fd, message, 50, 0);//receive message from client

		if (strcmp(message, "quit") == 0)
		{
			toContinue = false;
			printf("Client at socket %d quitted\n", temp_client->socket_fd);	
		}
	}

	remove_client(temp_client->socket_fd);
	close(temp_client->socket_fd);
	free(temp_client);										//free temporary struct client
	pthread_exit(NULL);
}

void sigintHandler(int sig_num) 
{ 
    signal(SIGINT, sigintHandler); 
    printf("\n Cannot be terminated using Ctrl+C \n"); 
} 

int main(int argc, char *argv[])
{
	signal(SIGINT, sigintHandler); 
	int portnum;
	int listen_fd, connection_fd;
	pthread_t recvt[254];
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;

	if(argc == 2)
	{
		portnum = atoi(argv[1]);
	}
	else
	{
		printf("Please run this program with format ./server portnum\n");
		exit(0);
	}

	char ip_address[15];
	unsigned int subnet_part;
	struct in_addr addr;
	unsigned int network_ip_num;
	unsigned int host_ip_num[254];

	//prompt user input an ip address
	printf("Enter an ip address: ");
	scanf("%s", ip_address);
	
	while (inet_aton(ip_address, &addr) == 0)
	{
		printf("Invalid ip address\n");
		printf("Enter an ip address again: ");
		memset(ip_address, 0, 15);
		scanf("%s", ip_address);
	}

	//prompt user to enter subnet part
	printf("Enter subnet part (23 < subnet part < 31): ");
	scanf("%u", &subnet_part);

	while (subnet_part < 24 || subnet_part > 30)
	{
		printf("Please enter a number from 24 to 30\n");
		printf("Enter subnet part: ");
		scanf("%u", &subnet_part); 
	}

	unsigned int subnet_num = 0xffffffff << (32 - subnet_part);
	//printf("subnet num: %u\n", subnet_num);

	network_ip_num = ntohl(addr.s_addr);
	network_ip_num = network_ip_num & subnet_num;
	//printf("network ip: %u\n", network_ip_num);
	
	//get ip host range
	unsigned int min_host_ip = network_ip_num + 1;
	//printf("min host ip: %u\n", min_host_ip);

	unsigned int max_host_ip = network_ip_num + (0xffffffff >> subnet_part) - 1;

	unsigned int temp_ip = min_host_ip;

	int ip_count = 0;
	while(temp_ip <= max_host_ip)
	{
		host_ip_num[ip_count] = temp_ip;
		temp_ip++;
		ip_count++;
	}

	//print info
	printf("\ndhcp_server %d\n", portnum);
	struct in_addr address;
	address.s_addr = htonl(network_ip_num); 
	printf("Network address: %s\n", inet_ntoa(address));
	printf("Subnet part: %u\n", subnet_part);

	/*
	for (int i = 0; i < ip_count; i++)
	{
		struct in_addr address = {htonl(host_ip_num[i])};
		printf("%s\n", inet_ntoa(address));
	}
	*/

	struct in_addr address1;
	address1.s_addr = htonl(min_host_ip); 

	struct in_addr address2;
	address2.s_addr = htonl(max_host_ip);

	//print host range
	printf("Usable host IP range: %s ", inet_ntoa(address1));
	printf("- %s\n", inet_ntoa(address2));

	//listen to socket
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd == -1)
	{
		perror("Cannot listen to socket\n");
		exit(0);
	}	

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(portnum);

	//to make sure it can use that some port later too
	int on = 1; 
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	if (bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)))
	{
		perror("Bind error\n");
		exit(EXIT_FAILURE);
	}

	if (listen(listen_fd, 10) == -1)
	{
		perror("Listen error\n");
		exit(EXIT_FAILURE);
	}

	int g = 0;
	while(1)
	{
		int length = sizeof(client_addr);
		connection_fd = accept(listen_fd, (struct sockaddr*) &client_addr, &length);//connect to client

		if (client_count > ip_count) //reject if too many client get in
		{
			printf("Number of clients cannot exceed %d. Connecion Rejected\n", ip_count);
			continue;
		}

		struct client *temp_client;
		temp_client = malloc(sizeof(struct client));

		temp_client->socket_fd = connection_fd;//socket number
		temp_client->client_ip = host_ip_num[client_count];

		int i = 0;
		while(1)
		{
			if(client_list[i] == NULL)
			{
				client_list[i] = temp_client; //add client to array
				break;
			}
			i++;
		}

		pthread_create(&recvt[g++], NULL, (void*) &server_handler, (void*)temp_client);
		client_count++;	
	}

 	for (int n = 0 ; n < g; n++)
	{
		pthread_detach(recvt[n]);
	}

	return 0;
}