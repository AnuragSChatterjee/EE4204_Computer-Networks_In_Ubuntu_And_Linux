#include "headsock.h"
#include <unistd.h>

float str_cli(FILE *fp, int sockfd, long *len);  //transmission function
void tv_sub(struct  timeval *out, struct timeval *in);	//calculate the time interval between out and in

int main(int argc, char **argv)
{ 
	int sockfd, ret;
	float ti, rt;
	long len;
	struct sockaddr_in ser_addr;
	char ** pptr;
	struct hostent *sh;
	struct in_addr **addrs;
	FILE *fp;
	int i,j;

	 
    float time_array[COMMUNICATION_TIME]; //to store time values in an array
    float rate_array[COMMUNICATION_TIME]; //to store rate values in an array
 

	if (argc != 2) {
		printf("parameters not match\n");
	}

	sh = gethostbyname(argv[1]);	                                       //get host's information
	if (sh == NULL) {
		printf("error when gethostby name\n");
		exit(0);
	}

	printf("canonical name: %s\n", sh->h_name);					//print the remote host's information
	for (pptr=sh->h_aliases; *pptr != NULL; pptr++)
		printf("the aliases name is: %s\n", *pptr);
	switch(sh->h_addrtype)
	{
		case AF_INET:
			printf("AF_INET\n");
		break;
		default:
			printf("unknown addrtype\n");
		break;
	}
        
	addrs = (struct in_addr **)sh->h_addr_list;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);  //create the UDP socket using DGRAM
	if (sockfd <0) {
		printf("error in socket\n");
		exit(1);
	}
	ser_addr.sin_family = AF_INET;                                                      
	ser_addr.sin_port = htons(MYUDP_PORT);
	memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
	bzero(&(ser_addr.sin_zero), 8);
	ret = connect(sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr));         //connect the socket with the host
	
	if (ret != 0) {
		printf ("connection failed\n"); 
		close(sockfd); 
		exit(1);
	}
	
	if((fp = fopen ("myfile.txt","r+t")) == NULL)
	{
		printf("File doesn't exit\n");
		exit(0);
	}

	for (i=0; i<COMMUNICATION_TIME;i++) {
		ti = str_cli(fp, sockfd,&len); //perform the transmission and receiving
		rt = (len/(float)ti); //caculate the average transmission rate
		printf("Time(ms) : %.3f, Data sent(byte): %d\nData rate: %f (Kbytes/s)\n", ti, (int)len, rt);

        time_array[i] = ti; //store transmission and receiving times in the time array first
        rate_array[i] = rt; //store average rates in the average rates array first

	float average_time_taken = 0.0;
    float average_rate_taken = 0.0;

    for (j=0; j<COMMUNICATION_TIME;j++) {
        average_time_taken += time_array[j]; //calculates float value for average time taken
        average_rate_taken += rate_array[j]; //calculates float value for average rate taken
    }
    average_time_taken /= COMMUNICATION_TIME; //average_time_taken = average_time_taken / COMMUNICATION_TIME;
    average_rate_taken /= COMMUNICATION_TIME; //average_rate_taken = average_rate_taken / COMMUNICATION_TIME;
    
	printf("Ave Time(ms) : %.3f\nAve Data rate: %f (Kbytes/s)\n", average_time_taken, average_rate_taken);

		close(sockfd);  
		fclose(fp);
		exit(0);

	}
}

float str_cli(FILE *fp, int sockfd, long *len)  
{
	char *buf;
	long lsize, ci; 
	char sends[DATALEN];
	struct ack_so ack;
	int n, slen;
	int length = 2; //batch size for sending data units. Change this as appropriate
	float time_inv = 0.0;
	struct timeval sendt, recvt;
	ci = 0;

	fseek (fp , 0 , SEEK_END);
	lsize = ftell (fp);
	rewind (fp);
	printf("The file length is %d bytes\n", (int)lsize);
	printf("the packet length is %d bytes\n",DATALEN);

// allocate memory to contain the whole file.
	buf = (char *) malloc (lsize);
	if (buf == NULL) exit (2);

  // copy the file into the buffer.
	fread (buf,1,lsize,fp);

  /*** the whole file is loaded in the buffer. ***/
	buf[lsize] ='\0';									//append the end byte
	gettimeofday(&sendt, NULL);							//get the current time

	int data_count = 0;//check data packets sent 

	while(ci<= lsize) 
	{
		if ((lsize+1-ci) <= DATALEN) {
			slen = lsize+1-ci;
		} else { 
			slen = DATALEN;
		} 
		memcpy(sends, (buf+ci), slen);
		n = send(sockfd, &sends, slen, 0); //addr, addrlen);

		printf("Data sent out\n");  

		if(n == -1) {
			printf("send error!");								//send the data
			exit(1);
		}
  
		//ci += slen;
		data_count++; //increment data count

		if (data_count == length || ci > lsize ) { 
            if ((n= recv(sockfd, &ack, 2, 0))==-1)  //addr, (socklen_t*)&addrlen))        //receive the ack
			{
				printf("error when receiving\n");
				exit(1);
			}
			if (ack.num != 1 || ack.len != 0) {
				printf("error in ACK transmission\n");
			}
			else {
				ci += slen;
			} 

			printf("ACK received!\n"); 
			
			if (length >= 3) {
			    length = 1;
			} 
			else {
			    length++;
			} 
			
			//printf("ACK received!\n");
            
			data_count = 0;
        }
		else {  
			ci += slen;
		}
	} 
 
	gettimeofday(&recvt, NULL);  
	*len= ci;    //get current time
	tv_sub(&recvt, &sendt);  // get the whole trans time
	time_inv += (recvt.tv_sec)*1000.0 + (recvt.tv_usec)/1000.0;
	return(time_inv);
}
 
void tv_sub(struct  timeval *out, struct timeval *in)
{
	if ((out->tv_usec -= in->tv_usec) <0)
	{
		--out ->tv_sec;
		out ->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
} 
