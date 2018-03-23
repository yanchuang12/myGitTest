#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>

#define SERVER_PORT 8000
#define LENGTH_OF_LISTEN_QUEUE 20
#define BUFFER_SIZE 50
#define FILE_NAME_MAX_SIZE 10

int main()
{
	struct sockaddr_in server_addr;
	bzero(&server_addr,sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(SERVER_PORT);

	int server_socket_fd = socket(AF_INET,SOCK_STREAM,0);
	if (server_socket_fd < 0)
	{
                printf("11");
		exit(1);
	}
	else
	{
  		 printf("creat socket successful\n");
	}
        // bind
	if (-1 == (bind(server_socket_fd,(struct sockaddr *)&server_addr,sizeof(server_addr))))
	{
   		printf("12");
		exit(1);
	}
	else
	{
  	 printf("bind successful\n");
	}
        // listen
	if (-1 == listen(server_socket_fd,LENGTH_OF_LISTEN_QUEUE))
	{
 		printf("13");
		exit(1);
	}
	else
	{
   		printf("listen successful\n");
	}

	while (1)
	{
		struct sockaddr_in client_addr;
		socklen_t client_addr_length= sizeof(client_addr);
		int new_server_socket_fd = accept(server_socket_fd,(struct sockaddr *)&client_addr,&client_addr_length);
		if (new_server_socket_fd < 0)
		{
 			printf("14");
			break;
		}
		else
		{
   			printf("accept successful\n");
		}

		char buffer[BUFFER_SIZE];
		bzero(buffer,BUFFER_SIZE);
		if (recv(new_server_socket_fd,buffer,BUFFER_SIZE,0) < 0)
		{
 			printf("15");
			break;
		}
		else
		{
   			printf("receive successful\n");
		}

		char file_name[FILE_NAME_MAX_SIZE];
		bzero(file_name,FILE_NAME_MAX_SIZE);
		strncpy(file_name,buffer,strlen(buffer)>FILE_NAME_MAX_SIZE?FILE_NAME_MAX_SIZE:strlen(buffer));

		printf("the file name :%s\n",file_name);
		FILE *fp = fopen(file_name,"r");
		if (NULL == fp)
		{
			printf("file not found");
		}
		else
		{
			printf("file opened\n");
			bzero(buffer,BUFFER_SIZE);
			int length = 0;
			int num = 0;
			printf("buffer_size is %d\n",BUFFER_SIZE);
			while ((length = fread(buffer,sizeof(char),BUFFER_SIZE,fp)) > 0)
			{
				
				//if(length > 0)
				{
					printf("fread length :%d\n",length);
				}
				//else
					//break;
				num++;
				if (send(new_server_socket_fd,buffer,length,0) < 0)
				{
					printf("out\n");
					break;
				}
				printf("num is %d\n",num);
				bzero(buffer,BUFFER_SIZE);
			}
			fclose(fp);
			printf("transfer %s successful\n",file_name);
		}
		close(new_server_socket_fd);
	}
	close(server_socket_fd);
	return 0;
}
