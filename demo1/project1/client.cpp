// client.cpp : Defines the entry point for the console application.
//
#ifdef WIN32
	#include "stdafx.h"
	#include <WinSock2.h>
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	#pragma comment(lib, "ws2_32.lib")
#else
	#include <stdio.h>
	#include <stdlib.h>
	#include <sys/socket.h>
	#include <sys/types.h>
	#include <string.h>
	#include <fcntl.h>
	#include <ctype.h>
	#include <string>
	#include <netinet/in.h>
	#include <unistd.h>
	#include <time.h>
	#include <arpa/inet.h>
#endif

#define BUF_SIZE 50
int main(int argc, char* argv[])
{
#ifdef WIN32
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) < 0)
	{
		printf("wsadata err\n");
		return -1;
	}
#endif
#ifdef WIN32
	SOCKET client_fd = socket(AF_INET, SOCK_DGRAM, 0);
#else
	int client_fd = socket(AF_INET, SOCK_DGRAM, 0); 
#endif
	if (client_fd < 0)
	{
		printf("client create socket err\n");
		return -1;
	}
	
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(6666);
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	int i;
	char buf[BUF_SIZE];
	memset(buf, 0x00, BUF_SIZE);
	strcpy(buf, "yan");
	//for (i = 0; i < 3; i++)
	{
		if ( sendto(client_fd, buf, strlen(buf), 0, (sockaddr *)&client_addr, sizeof(client_addr)) < 0)
		{
			printf("sendto err:");
			return -1;
		}
	}
	memset(buf, 0x00, BUF_SIZE);
	socklen_t cSize = sizeof(client_addr);
	int hr;
	hr = recvfrom(client_fd, buf, BUF_SIZE, 0, (sockaddr*)&client_addr, &cSize);
	if (hr < 0)
	{
		printf("recvfrom err:");
		return -1;
	}
	printf("client recv :%s\n", buf);


	strcpy(buf, "ls");
	//for (i = 0; i < 3; i++)
	{
		if ( sendto(client_fd, buf, strlen(buf), 0, (sockaddr *)&client_addr, sizeof(client_addr)) < 0)
		{
			printf("sendto err:");
			return -1;
		}
	}
	memset(buf, 0x00, BUF_SIZE);
	//int cSize = sizeof(client_addr);
	hr = recvfrom(client_fd, buf, BUF_SIZE, 0, (sockaddr*)&client_addr, &cSize);
	if (hr < 0)
	{
		printf("recvfrom err:");
		return -1;
	}
	printf("client recv :%s\n", buf);


	strcpy(buf, "exit");
	//for (i = 0; i < 3; i++)
	{
		if ( sendto(client_fd, buf, strlen(buf), 0, (sockaddr *)&client_addr, sizeof(client_addr)) < 0)
		{
			printf("sendto err:");
			return -1;
		}
	}
	memset(buf, 0x00, BUF_SIZE);
	//int cSize = sizeof(client_addr);
	hr = recvfrom(client_fd, buf, BUF_SIZE, 0, (sockaddr*)&client_addr, &cSize);
	if (hr < 0)
	{
		printf("recvfrom err:");
		return -1;
	}
	printf("client recv :%s\n", buf);

#ifdef WIN32
	closesocket(client_fd);
#else
	close(client_fd);
#endif
	return 0;
}

