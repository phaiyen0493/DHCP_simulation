#include <stdio.h>

struct dhcp_packet
{
	unsigned int siaddr; //server ip address
	unsigned int yiaddr; //client ip address
	unsigned int tran_ID; //transacion id
	unsigned short int lifetime; //lease time of the ip address
};