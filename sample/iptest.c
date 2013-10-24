#include <sys/unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>



int main()
{
 char hostname[128];
 int i;
 struct hostent *he;
 struct in_addr my_in_addr;


 gethostname(hostname, sizeof hostname);
 printf("My hostname: %s\n", hostname);

 he = gethostbyname(hostname);

 if (he == NULL) { // do some error checking
   herror("gethostbyname"); // herror(), NOT perror()
   return 1;
 }

 // print information about this host:
 printf("Official name is: %s\n", he->h_name);
 
 // Get the in_addr for the host
 my_in_addr = *(struct in_addr*)he->h_addr;
 printf("IP address: %s\n", inet_ntoa(my_in_addr));

 printf("\n");

 return 0;
}
