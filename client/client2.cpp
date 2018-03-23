#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>


#define SERVER_PORT 8000
//#define LENGTH_OF_LISTEN_QUEUE 20
#define BUFFER_SIZE 50
#define FILE_NAME_MAX_SIZE 10

int main()
{
	struct sockaddr_in client_addr;
	bzero(&client_addr,sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = htons(INADDR_ANY);
	client_addr.sin_port = htons(SERVER_PORT);

	int client_socket_fd = socket(AF_INET,SOCK_STREAM,0);

	if (client_socket_fd < 0)
	{
  		printf("11");
		exit(1);
	}
	else
	{
  		 printf("creat socket successful\n");
	}
        // bind
	/*if (-1 == (bind(client_socket_fd,(struct sockaddr *)&client_addr,sizeof(client_addr))))
	{
  		printf("12");
		exit(1);
	}
	else
	{
  		 printf("bind successful\n");
	}*/
	/*if (-1 == listen(server_socket_fd,LENGTH_OF_LISTEN_QUEUE))
	{
		exit(1);
	}*/

	if (connect(client_socket_fd,(struct sockaddr *)&client_addr,sizeof(client_addr)) < 0)
	{
  		printf("13");
		exit(0);
	}
	else
	{
  		 printf("connect successful\n");
	}
	char file_name[FILE_NAME_MAX_SIZE];
	bzero(file_name,FILE_NAME_MAX_SIZE);
	printf("please input file name on server:");
	scanf("%s",file_name);
	
	char buffer[BUFFER_SIZE];
	bzero(buffer,BUFFER_SIZE);
	strncpy(buffer,file_name,strlen(file_name)>BUFFER_SIZE?BUFFER_SIZE:strlen(file_name));
	printf("send the filename: %s\n",buffer);
	if (send(client_socket_fd,buffer,BUFFER_SIZE,0) < 0)
	{
  		printf("14");
		exit(1);
	}
	else
	{
  		 printf("send successful\n");
	}
	FILE *fp =fopen(file_name,"w");
	if (NULL == fp)
	{
  		printf("15");
		exit(1);
	}
	else
	{
  		 printf("open successful\n");
	}
	bzero(buffer,BUFFER_SIZE);
	int length = 0;
	while ((length = recv(client_socket_fd,buffer,BUFFER_SIZE,0)) > 0)
	{
		
		//if (length > 0)
			printf("length is %d\n",length);
		//else
		//	break;
		if (fwrite(buffer,sizeof(char),length,fp) < length)
		{
			break;
		}
		bzero(buffer,BUFFER_SIZE);
	}
	printf("received file %s successful\n",file_name);
	fclose(fp);
	close(client_socket_fd);
	return 0;
}
