#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <iostream>
#include <pthread.h>
using namespace std;

int s;//用于套字接描诉符，为了给线程用


void* thread(void *ss)
{
	int len = 1;
	char buf[BUFSIZ];
	//string a("lthyxy:");

	while((len = recv(s,buf,BUFSIZ,0)) > 0)
	{
		buf[len-1] = '\0';
		string c(buf,buf+len);
		string b;
		b += c;
		printf("%s\n",b.data());
	}
	return ss;
}

int main()
{
	int len;
	sockaddr_in remote_addr;
	char buf[BUFSIZ];

	memset(&remote_addr, 0, sizeof(remote_addr));
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	remote_addr.sin_port = htons(8000);

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket!");
		return 1;
	}

	if ((connect(s, (sockaddr *)&remote_addr, sizeof(sockaddr))) < 0)
	{
		perror("connect!");
		return 1;
	}

	printf("connected to lthyxy....\n");

	pthread_t id;
	int ret = pthread_create(&id, NULL, thread, NULL);
	if(ret)
	{ 
		printf("creat thread error!\n");
		return 1;
	}
	string kehuname("liutengfei:");
	while(1)
	{
		string k1(kehuname);
		string k2;
		while(getline(cin,k2))
		{
			k1 += k2;
		}
		cin.clear();
		len = k1.size();
		strcpy(buf,k1.c_str());
		buf[len] = '\n';
		if (send(s,buf,len+1,0) < 0)
		{
			perror("write!");
			return 1;
		}
	}
	shutdown(s,2);
	pthread_join(id,NULL);
	return 0;
}

