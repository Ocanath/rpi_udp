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

#define PORT 9145
#define MAX_UDP_RX_SIZE 1024
uint8_t rx_buf[MAX_UDP_RX_SIZE];

void main(void)
{
	int sockfd;	
	struct sockaddr_in servaddr, cliaddr; 
        
	// Creating socket file descriptor 
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
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
	
	while(1)
	{
		int len = sizeof(cliaddr);
	    int n = recvfrom(sockfd, (char *)rx_buf, MAX_UDP_RX_SIZE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len); 
		rx_buf[n] = '\0';
		memset(disp_buf, 0, sizeof(disp_buf));
		printf("%s sent %s\r\n", inet_ntoa(cliaddr.sin_addr), rx_buf);
	}
}