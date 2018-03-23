// tcp_server.cpp : Defines the entry point for the console application.
//
/*
#define WIN32_LEAN_AND_MEAN

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <WinSock2.h>
#include <Windows.h>
#include <time.h>
#include "../tcp_client/tcp_client/testmarc.h"
#pragma comment(lib, "ws2_32.lib")
*/
#include "nd_typedef.h"
#include "testmarc.h"
#include <string>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <assert.h>

size_t	
safe_strncpy(char* dest, const char* src, size_t n)
{
	size_t i = 0;
	char* ret = dest;

	if(n == 0 || NULL == src){
		dest[0] = 0; return 0;
	}

	for( i = 0; i < n; ++i ){
		if( 0 == (ret[i] = src[i]) ) return (i);
	}

	dest[n] = 0;	
	return n;
}
/*
bool 
dir_exist(const char *path)
{
	bool b = false;
	char tmp[MAX_PATH];

	size_t n = safe_strncpy(tmp, path, sizeof(tmp)-2);

	for( int i=0; i< (int)n; ++i){
		if(tmp[i] == '/') tmp[i] = '\\';
	}

	if( tmp[n-1] == '\\' ) {
		tmp[n-1] = 0;
	}

	WIN32_FIND_DATA fd; 
	HANDLE hFind = FindFirstFile(tmp, &fd);

	if ( hFind != INVALID_HANDLE_VALUE ) {

		if( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			b = true;
	}

	FindClose(hFind);
	return b;
}

bool 
dir_create(const char * path)
{
	if( dir_exist(path) )
		return true;

	int i, n;
	char szPath[MAX_PATH] = {0};
	char szTmp[MAX_PATH] = {0};

	n = (int)safe_strncpy( szTmp, path, sizeof(szTmp)-1 );

	for( i=0; i<n; i++ ) {		

		if( szTmp[i] == '\\' || szTmp[i] == '/' ) {

			safe_strncpy( szPath, szTmp, i );
			::CreateDirectory( szPath, NULL );
		}
	}

	return dir_exist(path);

}
*/

bool 
dir_exist(const char *path)
{
	DIR *dir;
	printf("path :%s\n", path);
	if ((dir = opendir(path)) == NULL){
		printf("false\n");
		return false;
	}else{
		printf("true\n");
		closedir(dir);
		return true;
	}
}


bool 
dir_create(const char * path)
{
	if( dir_exist(path) )
		return true;

	int i, n;
	char szPath[MAX_PATH_LEN] = {0};
	char szTmp[MAX_PATH_LEN] = {0};

	n = (int)safe_strncpy( szTmp, path, sizeof(szTmp)-1 );

	for( i=0; i<n; i++ ) {		

		if( szTmp[i] == '\\' || szTmp[i] == '/' ) {

			safe_strncpy( szPath, szTmp, i );
			mkdir( szPath, 0777 );
		}
	}

	return dir_exist(path);
}


class rcv_file
{
public:
	rcv_file()
	{
		this->buf_ = new char[SIZE_MTU];
		this->file_ = -1;
		this->listen_s_ = -1;
		this->s_ = -1;
	}
	~rcv_file()
	{
		if (this->s_ != -1) {
			close(this->s_);
			this->s_ = -1;
		}

		if (this->listen_s_ != -1) {
			close(this->listen_s_);
			this->listen_s_ = -1;
		}

		if (this->file_ != -1) {
			close(this->file_);
			this->file_ = -1;
		}

		delete []this->buf_;
	}
	bool open_t(const char* dir, int listen_port)
	{
		this->dir_ = dir;
		if (this->dir_.size() < 3) {
			return false;
		}
		char c = this->dir_[this->dir_.size() - 1];
		if (c != '/' && c != '\\') {
			this->dir_ += "/";
		}

		dir_create(this->dir_.c_str());

		this->listen_port_ = listen_port;
		/*
		WSADATA wsaDate;
		int err = WSAStartup(MAKEWORD(2, 2), &wsaDate);
		if (0 != err)
		{
			printf("WSAStartup err\n");
			return false;
		}*/

		this->listen_s_ = socket(AF_INET, SOCK_STREAM, 0);

		if (this->listen_s_ == -1)
		{
			printf("socket failed\t\n");
			return false;
		}

		struct sockaddr_in server_addr;
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(this->listen_port_);
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		int ret;
		ret = bind(this->listen_s_, (struct sockaddr*)&server_addr, sizeof(server_addr));
		if (ret == -1)
		{
			printf("bind failed\t\n");
			close(this->listen_s_);
			return false;
		}

		ret = listen(this->listen_s_, 5);
		if (ret == -1)
		{
			printf("listen failed\t\n");
			close(this->listen_s_);
			return false;
		}

		struct sockaddr_in client_addr;
		int sSize = sizeof(client_addr);
		printf("waiting client ....\t\n");
		this->s_ = accept(this->listen_s_, (struct sockaddr*)&client_addr, (socklen_t*)&sSize);
		if (this->s_ == -1)
		{
			close(this->listen_s_);
			printf("accept failed\t\n");
			return false;
		}
		int opt;
		opt = 512 * 1024;
		setsockopt(this->s_, SOL_SOCKET, SO_RCVBUF, (char*)&opt, sizeof(opt));
		opt = 512 * 1024;
		setsockopt(this->s_, SOL_SOCKET, SO_SNDBUF, (char*)&opt, sizeof(opt));
		printf("accept a connection\r\n");

		char *buf = this->buf_;
		int len = 0;
		//int tmo = 10*1000;

		if ( recv(this->s_, buf, HDR_LEN, 0) == -1 || buf[2] != 1) {
			printf("ndrudp_recv head error\r\n");
			return false;
		}


		uint16_t nbody = ntohs(*(uint16_t*)buf);

		if ( recv(this->s_, buf, nbody, 0) == -1) {
			printf("ndrudp_recv body error\r\n");
			return false;
		}
		buf[nbody] = 0;

		uint64_t fsize = nd_ntohll(*(uint64_t*)buf);
		uint16_t fn_len = ntohs(*((uint16_t*)(buf + 8)));

		if (fn_len > 1024) {
			printf("ndrudp_recv body error, invalid file_size\r\n");
			return false;
		}
		this->fn_ = this->dir_ + (buf + 8 + 2);

		/*this->file_ = CreateFile(this->fn_.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if( INVALID_HANDLE_VALUE == this->file_ ) 
			return false;*/
		
		if ((this->file_ = open(this->fn_.c_str(), O_RDWR|O_CREAT|O_TRUNC, 00664)) == -1){
					printf("Open %s Error\n", this->fn_.c_str());
					return false;
		}

		printf("start recv file: %s...\r\n", this->fn_.c_str());

		uint64_t tm_begin = GetTickCount();
		uint64_t tm = GetTickCount();
		uint64_t tm_inter = GetTickCount();
		uint64_t recv_rate = 0;
		uint64_t sum2 = 0;
		int rcv_len = 0;
		uint64_t recv_bytes = 0;
		uint64_t total_size = 0;
		while (1) {
			if (total_size == fsize) {
				printf("file recv finished, rate=%.2fMB\r\n", ((double)fsize) / (double(GetTickCount() - tm_begin) / 1000.0f + 0.1) / (double)(1024.*1024));
				break;
			}
			//for (int i = 0; i < 1; i++)
			{
				//rcv_len = recv(this->s_, buf + sum2, SIZE_MTU, 0);
				rcv_len = recv(this->s_, buf, HDR_LEN, 0);
				nbody = ntohs(*(uint16_t*)buf);
				if (nbody > 8192) {
				    printf("invalid body");
				    return false;
				}
				rcv_len = recv(this->s_, buf, nbody, 0);
				if (rcv_len == -1 || rcv_len != nbody)
				{
					printf("recv data error\r\n");
					return false;
				}
				//sum2 += rcv_len;
				total_size += rcv_len;

				recv_bytes += rcv_len;
				if (GetTickCount() - tm > 1000)
				{
					tm_inter = GetTickCount();
					recv_rate = ((double)(recv_bytes) / (double)(tm_inter - tm)) * 1000.0;
					printf("inter_time=%.2f, recv_rate=%llu(Bytes/S)\n", (double)(tm_inter - tm) / 1000.0, recv_rate);
					tm = tm_inter;
					recv_bytes = 0;
				}
			}
			
			/*uint64_t wt_len = 0;
			BOOL ret = WriteFile(this->file_, buf, rcv_len, &wt_len, NULL);
			//BOOL ret = WriteFile(this->file_, buf, sum2, &wt_len, NULL);
			if (!ret || wt_len != rcv_len) {
				printf("WriteFile error\r\n");
				return false;
			}*/
			uint64_t wt_len = write(this->file_, buf, rcv_len);
					if (-1 == wt_len || wt_len != rcv_len) {
						printf("WriteFile error\r\n");
						return false;
					}
			//sum2 = 0;
		}

		*((uint16_t*)buf) = htons(0);
		buf[2] = 3;
		buf[3] = 0;
		if (-1 == send(this->s_, buf, HDR_LEN, 0)) {
			printf("recv finished, but send bye error, rate=%.2fMB\r\n", ((double)fsize) / (double(GetTickCount() - tm) / 1000.0f + 0.1) / (double)(1024.*1024));
			return false;
		}

		close(this->s_);
		this->s_ = -1;

		return true;
	}

public:
	std::string dir_;
	std::string fn_;
	int listen_port_;
	int listen_s_;
	int s_;
	int file_;
	char *buf_;
};


int main(int argc, char* argv[])
{
	if (argc != 3) {
		printf("invalid argc, app exit...\r\n");
		return 0;
	}

	rcv_file rf;
	if (!rf.open_t(argv[1], atoi(argv[2]))) {
		printf("open error, app exit...\r\n");
		return 0;
	}

	//system("pause");
 
	return 0;
}

