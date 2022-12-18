#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#include <sys/ioctl.h>


typedef union u32_fmt_t
{
	uint32_t u32;
	int32_t i32;
	float f32;
	int16_t i16[sizeof(uint32_t) / sizeof(int16_t)];
	uint16_t ui16[sizeof(uint32_t) / sizeof(uint16_t)];
	int8_t i8[sizeof(uint32_t) / sizeof(int8_t)];
	uint8_t ui8[sizeof(uint32_t) / sizeof(uint8_t)];
}u32_fmt_t;


/*Setup server parameters*/
#define PORT 9145
#define MAX_UDP_RX_SIZE 1024
uint8_t rx_buf[MAX_UDP_RX_SIZE];
u32_fmt_t * fmt_buf = (u32_fmt_t*)(&rx_buf[0]);


/*
Generic hex checksum calculation.
TODO: use this in the psyonic API
*/
uint32_t get_checksum32(uint32_t* arr, int size)
{
	int32_t checksum = 0;
	for (int i = 0; i < size; i++)
		checksum += (int32_t)arr[i];
	return -checksum;
}


void main(void)
{
	int sockfd;	
	struct sockaddr_in servaddr, cliaddr; 
        
	// Creating socket file descriptor . include nonblocking flag here for... nonblocking functionality
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0)) < 0 ) 
	{ 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	}
	memset(&servaddr, 0, sizeof(servaddr)); 
	memset(&cliaddr, 0, sizeof(cliaddr)); 


	/*Obtain linux hostname and ip address, for attempting to bind to it later.*/
	struct ifreq ifr;
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name,"wlan0",IFNAMSIZ-1);
	ioctl(sockfd,SIOCGIFADDR,&ifr);
	//printf("ip of %s is %s\r\n",ifr.ifr_name, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
	
	// Filling server information 
    servaddr.sin_family    = AF_INET; // IPv4 
    //servaddr.sin_addr.s_addr = INADDR_ANY; 
	servaddr.sin_addr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
    servaddr.sin_port = htons(PORT); 

	char * disp_buf[256] = {0};
	inet_ntop(AF_INET, &servaddr.sin_addr.s_addr, (char*)disp_buf, 256);	//convert again the value we copied thru and display
	printf("Binding to server address: %s with port %d\r\n", disp_buf, PORT);
	if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }
	printf("Bind success\r\n");
		
	int cli_len = sizeof(cliaddr);
	while(1)
	{
		
	    int n = recvfrom(sockfd, (char *)rx_buf, MAX_UDP_RX_SIZE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &cli_len); 
		if(n == -1)
		{
			//waiting
		}
		else
		{
			rx_buf[n] = '\0';
			uint8_t chk_match=0;
			if(n % sizeof(u32_fmt_t) == 0)	//32bit checksum only applies if we have a multiple of 4 length data input
			{
				int u32_len = n/sizeof(u32_fmt_t);
				if(get_checksum32((uint32_t*)rx_buf, u32_len-1) == fmt_buf[u32_len - 1].u32)
				{
					chk_match = 1;
					
					//print formatted datas
					printf("%s sent: ", inet_ntoa(cliaddr.sin_addr));
					for(int i = 0; i < u32_len-2; i++)
					{
						printf("%0.4f, ", fmt_buf[i].f32);
					}
					printf("%0.4f\r\n", fmt_buf[u32_len-2].f32);
				}
			}
			else if(chk_match == 0)
			{
				printf("%s sent %s\r\n", inet_ntoa(cliaddr.sin_addr), rx_buf);
			}
		}
	}
}