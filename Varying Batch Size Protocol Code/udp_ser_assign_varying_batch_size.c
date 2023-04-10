#include "headsock.h"
#include "errno.h"

#define BACKLOG 10

void str_ser(int sockfd); // transmitting and receiving function

int main(void)
{
	int sockfd;
	struct sockaddr_in my_addr;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {	//create the UDP socket through DGRAM
		printf("error in socket");
		exit(1);
	}

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYUDP_PORT); 
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero), 8);
	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) {   //bind socket
		printf("error in binding");
		exit(1);
	}
	printf("start receiving data\n");
	while(1) {
		printf("waiting to data to be received\n");
		str_ser(sockfd);  // send and receive data
	}
	close(sockfd);
	exit(0);
}

void str_ser(int sockfd) // struct sockaddr *addr, int addrlen)
{
	char buf[BUFSIZE];
	FILE *fp;
	char recvs[DATALEN];
	struct ack_so ack;
	ack.num = 1;
	ack.len = 0;
	int end = 0;
	int n = 0;
	int length = 2; //batch size for sending data units. Change this as appropriate
	int packet_data_count_received = 0;
	long lseek=0;
	end = 0;
	struct sockaddr_in their_addr;
	int sin_size = sizeof(struct sockaddr_in);

	printf("Available to receive more data!\n");

	while(!end)
	{
		if ((n=recvfrom(sockfd, &recvs, DATALEN, 0, (struct sockaddr *)&their_addr, &sin_size)) == -1) //receive the packet and save their_addr
		{
			printf("error when receiving\n");
			exit(1); 
		}

		printf("Packet received!\n"); 

		packet_data_count_received++;
		if (recvs[n-1] == '\0')	//if it is the end of the file
		{
			end = 1; 
			n --;
		}
        memcpy((buf+lseek), recvs, n); 
        lseek += n;
        if (packet_data_count_received == length || end == 1) {
            ack.num = 1;
            ack.len = 0;

			if (sendto(sockfd, &ack, 2, 0, (struct sockaddr *)&their_addr, sin_size) == -1) { //send ack to their_addr
				printf("sending error for ACK!"); 
				exit(1);
			} 

            // n = sendto(sockfd, &ack, 2, 0, addr, addrlen);
            // if (n == -1) {
            //     printf("error sending ACK");
            //     exit(1);
            // }
 
            printf("ACK sent!\n"); 

			if (length >= 3) {
			    length = 1;
			} else {  
			    length++; 
			}  

            packet_data_count_received = 0;
        }
    }    
  
	if ((fp = fopen ("receivedUDPfile.txt","wt")) == NULL)
	{
		printf("File doesn't exit\n");
		exit(0);
	}
	fwrite (buf , 1 , lseek , fp);					//write data into file
	fclose(fp);
	printf("a file has been successfully received!\nthe total data received is %d bytes\n", (int)lseek);
}
  