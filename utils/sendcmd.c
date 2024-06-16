/* client.c */
#include <fcntl.h>
#include <sys/time.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<netdb.h>
#include <iostream>

#define SEND_LEN	0

int main(int argc, char *argv[])
{
   int sock, timeout;
   struct timeval tvstart, tvend;
   int bufsize = 8192;
   int nread, len;
   char *message = (char*)malloc(bufsize);
   char *response = (char*)malloc(bufsize);
   char *hostname = (char*)malloc (30);
   int i, port, mode;
   struct sockaddr_in address;
   struct hostent *host_struct; 

   mode = 0;    		/* 0: wait, 1: not wait*/
   timeout = 10;
   i = 1;
   if (argc > 1) 
	while (i < argc) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == 'h')
				{hostname = argv[i+1]; i++;}
			else
				if (argv[i][1] == 'p')
					{port = atoi (argv[i+1]); i++;}
			else
				if (argv[i][1] == 't')
                        		{timeout = atoi(argv[i+1]); i++;} 
			else
				if (argv[i][1] == 'm')
                                	{mode = atoi (argv[i+1]); i++;}
			else
//				if (argv[i][1] == '?')
                                	{printf ("usage: sendcmd [-h hostname] [-p port] [-t timeout] [-m mode] command\n"); return (0);}
		} else
			message = argv[i];
		i++;
   	}
   if ((host_struct = gethostbyname (hostname)) == NULL){
	   printf ("ERROR %d getting_mem\n", errno);
           return (-errno);
   }
    
   if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
     	printf("ERROR %d creating_socket\n", errno);
	return (-errno);
   }

   if (fcntl (sock, F_SETFL, O_NONBLOCK) <0){
		printf ("ERROR %d setting_socket", errno);
		close (sock);
                return (-errno);
   }

    bcopy (host_struct->h_addr, (char *) &address.sin_addr.s_addr, host_struct->h_length); 
   address.sin_family = AF_INET;
   address.sin_port = htons(port);

   if (gettimeofday (&tvstart, NULL) <0) {
        printf ("ERROR %d reading_time\n", errno);
	close (sock);
	return (-errno);
   }

   if ((connect(sock,(struct sockaddr *) &address, sizeof(address)) == -1) && (errno != EINPROGRESS)) {
	printf ("ERROR %d connecting\n", errno);
	close (sock);
	return (-errno);
   }

   len = strlen (message);
   message[len]='\n';
   message[len+1]='\0';
   len++;

#if SEND_LEN
   char *stringlen = malloc(10);

   sprintf (stringlen, "%08d", len);
   printf ("%s", stringlen);
   while ((nread = write (sock, stringlen, 8)) < 0) {
           if (errno != EAGAIN) {
                   printf ("ERROR %d writing_len\n", errno);
                   close (sock);
                   return (-errno);
           }
           gettimeofday (&tvend, NULL);
           if (tvend.tv_sec - tvstart.tv_sec >= timeout) {
                   printf ("ERROR %d TIMEOUT\n", -ETIME);
                   close (sock);
                   return (-ETIME);
           }
   }
#endif

   while ((nread = write (sock, message, len)) < 0) {
           if (errno != EAGAIN) {
                   printf ("ERROR %d writing_message\n", errno);
                   close (sock);
                   return (-errno);
           }
           gettimeofday (&tvend, NULL);
           if (tvend.tv_sec - tvstart.tv_sec >= timeout) {
                   printf ("ERROR %d TIMEOUT\n", -ETIME);
                   close (sock);
                   return (-ETIME);
           }
   }

   if (mode == 0) {
  	while ((nread = read (sock, response, bufsize)) < 0) {
		if (errno != EAGAIN) {
			printf ("(sendcmd) ERROR %d reading\n", errno);
			std::cerr << "(sendcmd) nread:" << nread << " response:" << response << "\n";
			close (sock);
			return (-errno);
		}
		usleep (10000);
		gettimeofday (&tvend, NULL);
		if (tvend.tv_sec - tvstart.tv_sec >= timeout) {
        		printf ("ERROR %d TIMEOUT\n", -ETIME);
        		close (sock);
        		return (-ETIME);
		}
   	}
   }
  if (mode == 0)
  	printf("%s\n", response);
  else
	printf("cmd_sent\n");

  return (close(sock));
}
