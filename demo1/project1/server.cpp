// server.cpp : Defines the entry point for the console application.
//
#ifdef WIN32
	#include "stdafx.h"
	#include <WinSock2.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <iostream>
	#include <string>

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
	#include <iostream>
#endif

#include <list>
#include <map>
//using namespace std;
#define BUF_SIZE 50

int main(int argc, char* argv[])
{
	std::list<std::string> name;
	//std::list<std::string> password;
	name.push_back("yan");
	name.push_back("zhang");
	name.push_back("jiang");
	name.push_back("liu");
	std::list<std::string>::iterator it_list;

	std::map<std::string, std::string> account;
	account.insert(std::pair<std::string, std::string>("yan", "123456"));
	account.insert(std::pair<std::string, std::string>("zhang", "123456"));
	account.insert(std::pair<std::string, std::string>("jiang", "123456"));
	account.insert(std::pair<std::string, std::string>("liu", "123456"));

	std::map<std::string, std::string>::iterator it;
	for (it = account.begin(); it != account.end(); it++)
	{
		std::cout<<it->first<<" "<<it->second<<std::endl;
	}
#ifdef WIN32
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{
		printf("wsastartup err\n");
		return -1;
	}
#endif


#ifdef WIN32
	SOCKET server_fd = socket(AF_INET, SOCK_DGRAM, 0);
#else
	int server_fd = socket(AF_INET, SOCK_DGRAM, 0);
#endif
	if (server_fd < 0)
	{
		printf("create socket err\n");
		return -1;
	}
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	//bzero(&server_addr, sizeof(server_addr));	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(6666);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if ( bind(server_fd, (sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		printf("bind err\n");
		return -1;
	}
	
	char buf[BUF_SIZE];
	char buf2[BUF_SIZE];
	char sendbuf[BUF_SIZE];
	struct sockaddr_in clntAddr;
	socklen_t cSize = sizeof(clntAddr);
	int flag = 1;
	while (flag)  /// µÇÂ¼³É¹¦£¬¼ì²é
	{
		memset(buf2, 0x00, BUF_SIZE);
		int hr = recvfrom(server_fd, buf2, BUF_SIZE, 0, (sockaddr*)&clntAddr, &cSize);
		if (hr < 0)
		{
			printf("server recv err:");
			return -1;
		}

		printf("server recv :%s\n", buf2);

		for (it_list = name.begin(); it_list != name.end(); it_list++)
		{
//printf("");
std::cout<<*it_list<<std::endl;
			//char *tmp = it_list->c_str();
			memset(sendbuf, 0x00, BUF_SIZE);
			if ( strcmp(buf2, it_list->c_str()) == 0)
			{
				//strcpy(buf, "welcome!");
				sprintf(sendbuf, "%s,%s", buf2, "welcome");
				if ( sendto(server_fd, sendbuf, strlen(sendbuf), 0, (sockaddr*)&clntAddr, sizeof(clntAddr)) < 0)
				{
					printf("server sendto err\n");
					return -1;
				}
				else
				{
					flag = 0;
					printf("sendto success\n");
					break;
				}
			}
		}
	}

	printf("server 2\n");
	flag = 1;
	FILE *stream;

	while (flag)  /// 
	{
		memset(buf, 0x00, BUF_SIZE);
		memset(sendbuf, 0x00, BUF_SIZE);

		int hr = recvfrom(server_fd, buf, BUF_SIZE, 0, (sockaddr*)&clntAddr, &cSize);
		if (hr < 0)
		{
			printf("server recv err:");
			return -1;
		}

		printf("server recv :%s\n", buf);
		
		if (strcmp(buf, "ls") == 0)
		{
			printf("come 1\n");
			stream = popen("pwd", "r");
			if (!stream)
			{
				printf("_popen err\n");
				return -1;
			}
			fread(sendbuf, sizeof(char), BUF_SIZE, stream);
			printf("send buf :%s\n", sendbuf);
			if ( sendto(server_fd, sendbuf, strlen(sendbuf), 0, (sockaddr*)&clntAddr, sizeof(clntAddr)) < 0)
			{
				printf("server sendto err\n");
				return -1;
			}
			else
			{
				//flag = 0;
				//break;
			}
		}

		//printf("server recv :%s\n", buf);
		
		if (strcmp(buf, "exit") == 0)
		{
			sprintf(sendbuf, "%s,%s", buf2, "bye bye!");
			if ( sendto(server_fd, sendbuf, strlen(sendbuf), 0, (sockaddr*)&clntAddr, sizeof(clntAddr)) < 0)
			{
				printf("server sendto err\n");
				return -1;
			}
			else
			{
				flag = 0;
				//break;
			}
		}


// 		for (it_list = name.begin(); it_list != name.end(); it_list++)
// 		{
// 			//char *tmp = it_list->c_str();
// 			memset(sendbuf, 0x00, BUF_SIZE);
// 			if ( strcmp(buf, it_list->c_str()) == 0)
// 			{
// 				//strcpy(buf, "welcome!");
// 				sprintf(sendbuf, "%s,%s", buf, "welcome");
// 				if ( sendto(server_fd, sendbuf, strlen(sendbuf), 0, (sockaddr*)&clntAddr, sizeof(clntAddr)) < 0)
// 				{
// 					printf("server sendto err\n");
// 					return -1;
// 				}
// 				else
// 				{
// 					flag = 0;
// 					break;
// 				}
// 			}
// 		}
	}
#ifdef WIN32
	closesocket(server_fd);
#else
	close(server_fd);
#endif
	return 0;
}

