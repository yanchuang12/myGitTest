#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <iostream>
#include <pthread.h>
#include<unistd.h>//fork,getpid

using namespace std;

int fd[100];//建立的套字接描诉符号。 

int haha = 1;
void* thread(void* s)
{
	printf("thread start\n");
	int len = 1;
	char buf[BUFSIZ];
	//string a("liutengfei:");

	while((len = recv(fd[haha-1],buf,BUFSIZ,0)) > 0)
	{
		buf[len-1] = '\0';
		string c(buf,buf+len);
		string b;
		b += c;
		printf("ID%d,%s\n",fd[haha-1],b.data());
	}
	return s;
}

int main()
{
	int s;//服务器socket()函数返回的套字接描诉符，用于通过网络发送和接收数据

	//int fd;//服务器accept()函数返回的一个信的套字结描诉符，用于与链接上的客户点进行数据的交换，原来的描诉符仍然被用于接受其他客户端的链接请求

	int len;//用于recv()函数返回的字节长度

	sockaddr_in my_addr;//linux提供的专用于IP地址结构体

	sockaddr_in remote_addr;
	socklen_t sin_size;//不能用int,必须用socklen_t,等下去查查资料

	char buf[BUFSIZ];//存接收到的字节


	memset(&my_addr, 0, sizeof(my_addr));//让该结构体中的全部数据为0

	my_addr.sin_family = AF_INET;//地址族，定义为short类型，AF_INET表示他使用的是因特网地址族

	my_addr.sin_addr.s_addr = INADDR_ANY;//INADDR_ANY表示表示任意IP地址，也可以用inet_addr("192.168.1.1")来让IP地址转换成正确的IP地址结构

	my_addr.sin_port = htons(8000);//端口号，定义为short类型，后面的htons函数用于转换正确的端口号的值


	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket!");
		return 1;
	}

	if (bind(s,(sockaddr *)&my_addr,sizeof(sockaddr)) < 0)//创建的套字接必须被绑定到一个本地IP地址和端口号用于TCP通信

	{//第1个参数是套字接描诉符，第二个参数决定本地网络定义，第3个参数为sockaddr结构体的长度

		perror("bind!");
		return 1;//函数成果调用返回0，否则返回-1

	}

	listen(s,5);//开始监听客户端的连接，第一个参数是套字接描诉符，第2个参数表示表示监听队列的长度


	sin_size = sizeof(sockaddr_in);//结构体的长度


	while(1)
	{
		if((fd[haha++] = accept(s,(sockaddr *)&remote_addr,&sin_size)) < 0)//当一个链接请求被接受以后，该函数返回一个新的套字接描述符号。这个描诉符用于与远程客户通信

		{//第一个参数为那个可以被监听的套字接描诉符，后面2个参数分别接受此结构的指针与长度的指针

			perror("accept!");
			return 1;
		}
		if(!fork())break;//假如成功建立子进程就跳出这该死的循环。

	}

	printf("accept client %s\n",inet_ntoa(remote_addr.sin_addr));//显示接受的客户端的IP

	len = send(fd[haha-1],"Welcome to lthyxy's server!\n",28, 0);//发句欢迎词给客户端

	string zhuji("lthyxy:");
	pthread_t id;//新建立的线程的ID

	int ret = pthread_create(&id,NULL,thread,NULL);//让线程进行，并进入对应的函数。

	if (ret) { printf("create thread error!\n"); return 1;}//如过线程成功进行了就返回0，否则返回1？    

	while(1)//死循环，不停的向客户端发的信息

	{
		char buff[BUFSIZ];
		string k1;
		string k2(zhuji);
		while(getline(cin,k1)){ k2 += k1;}//感觉处理没C++的方便就偷懒了一下，呵呵。

		cin.clear();
		len = k2.size();
		strcpy(buff,k2.c_str());
		buff[len] = '\n';
		if(send(fd[haha-1], buff, len+1, 0) < 0)//再发回给客户端.

		{
			perror("write!");
			return 1;
		}
	}
	shutdown(fd[haha-1],2);
	shutdown(s,2);//第一个参数套字接描诉符，后面的以后参数0表示不接受，1表示不发送，2表示既然不接受也不发送    

	pthread_join(id,NULL);//等待其他线程都完成运行。

	return 0;
}

