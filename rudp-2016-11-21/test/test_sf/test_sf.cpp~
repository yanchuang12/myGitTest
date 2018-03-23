// test_sf.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"

#include "../../rudp/nd_core/nd_typedef.h"

// #include <sys/sys_asy.h>
// #include <sys/sys_thread.h>
#include <string>
#include <list>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>

#include "../../rudp/nd_rudp_api.h"

#define MAX_PATH_LEN 260
std::list<std::string> gfn_lst;
int count = 0;
// traversal folder

void fnlst_find(const char *dirname)
{
	assert(dirname != NULL);

	char path[512];
	struct dirent *filename;//readdir 的返回类型
	DIR *dir;//血的教训阿，不要随便把变量就设成全局变量。。。。

	dir = opendir(dirname);
	if(dir == NULL)
	{
		printf("open dir %s error!\n",dirname);
		exit(1);
	}

	while((filename = readdir(dir)) != NULL)
	{
		//目录结构下面问什么会有两个.和..的目录？ 跳过着两个目录
		if(!strcmp(filename->d_name,".")||!strcmp(filename->d_name,".."))
			continue;
		if(0 == count)
		{
			gfn_lst.push_back(dirname);
			printf("%d. %s\n",++count,dirname);
		}
		//非常好用的一个函数，比什么字符串拼接什么的来的快的多
		sprintf(path,"%s/%s",dirname,filename->d_name);

		gfn_lst.push_back(path);
		printf("%d. %s\n",++count,path);
		struct stat s;
		lstat(path,&s);

		if(S_ISDIR(s.st_mode))
		{
			fnlst_find(path);//递归调用
		}
		else
		{
			//printf("%d. %s\n",++count,filename->d_name);
		}
	}
	closedir(dir);
}

size_t	
safe_strncpy(char* dest, const char* src, size_t n)
{
	size_t i = 0;
	char* ret = dest;

	if(n == 0 || NULL == src){
		dest[0] = 0; 
		return 0;
	}

	for( i = 0; i < n; ++i ){
		if( 0 == (ret[i] = src[i]) )  
			return (i);
	}

	dest[n] = 0;	
	return n;
}

bool
is_dir(const char *path)
{

	bool b = false;
	char tmp[MAX_PATH_LEN];
	struct stat s;
	
	size_t n = safe_strncpy(tmp, path, sizeof(tmp));

	lstat(tmp, &s);
	if (S_ISDIR(s.st_mode)){
		return true;
	}
	else{
		return false;
	}
	return b;
}



class snd_file //: public xy::thread_base
{
	enum {MAX_BUF_SIZE = 2000, HDR_LEN = 2 + 2};
public:
	snd_file()
	{	
		this->buf_ = (char*)malloc(MAX_BUF_SIZE);
		this->file_ = -1;//INVALID_HANDLE_VALUE;
		this->s_ = ND_RUDP_INVALID_SOCKET;
		ndrudp_init();
		ndrudp_init_log(0xffff, 0x01, "d:/nd_rudp_snd.log");
	}
	~snd_file()
	{
		if (this->s_ != ND_RUDP_INVALID_SOCKET) {
			ndrudp_close(this->s_);
		}

		/*if (this->file_ != -1) {
			close(this->file_);
			this->file_ = -1;
		}*/

		//shut_down();
		//wait();

		ndrudp_uninit();

		free(this->buf_);
	}
	bool open_t(const char* fn, const char* addr)
	{
		printf("open_t start, fn :%s, addr :%s\n", fn, addr);
		this->addr_ = addr;
		std::string::size_type pos = this->addr_.find(':');
		if (std::string::npos == pos || pos < 5) {
			printf("invalid addr\r\n");
			return false;
		}

		std::string ip = this->addr_.substr(0, pos);
		this->ip_ = ntohl(inet_addr(ip.c_str()));
		this->port_ = atoi(this->addr_.substr(pos + 1).c_str());

		/// whether is folder
		if (is_dir(fn))
		{
			this->is_folder_ = true;
			fnlst_find(fn); /// traver folder
			std::list<std::string>::iterator it;
			for (it = gfn_lst.begin(); it != gfn_lst.end(); it++)
			{
				std::cout<<*it<<std::endl;
			}
			printf("\n");
			this->fn_ = fn;
		}
		else 
		{
			this->is_folder_ = false;
			this->fn_ = fn;

			if ((this->file_ = open(fn, O_RDONLY|O_CREAT)) == -1){
				printf("Open %s Error\n", fn);
				return false;
			}
			struct stat statbuff;
			if (stat(fn, &statbuff) < 0){
				printf("stat Error\n");
				return false;
			}
			else{
				this->fsize_ = statbuff.st_size;
			}
		}

		this->s_ = ndrudp_socket(0, 0, -1, 0);
		if (ND_RUDP_INVALID_SOCKET == this->s_) {
			printf("ndrudp_socket error\r\n");
			return false;
		}

		if (0 != ndrudp_connect(this->s_, this->ip_, this->port_, 30*1000)) {
			printf("ndrudp_connect to %s error\r\n", this->addr_.c_str());		
			return false;
		}

		//active();
		svc();
		return true;
	}
public:
	int svc()
	{
		bool b = __svc();
		ndrudp_close(this->s_);
		this->s_ = ND_RUDP_INVALID_SOCKET;
		
		return 0;
	}
	bool folder_send(uint64_t &send_total_size, unsigned long &tm)
	{
		struct timeval t;
		int len = 0;
		char *buf = this->buf_;

		send_total_size = 0;
		int folder_size = this->fn_.size();

		std::string short_fn = "";
		char *c = &this->fn_[this->fn_.size() - 1];
		int i = 0;
		for (; i != this->fn_.size(); i++) {
			if (*c == '/' || *c == '\\') {
				short_fn = c + 1;
				break;
			}
			c--;
		}

		short_fn += "/";  /// folder symbol
		*((uint16_t*)buf) = htons(8 + 2 + short_fn.size());
		buf[2] = 1;
		buf[3] = 0;
		*((uint64_t*)(buf + HDR_LEN)) = nd_htonll(0);
		*((uint16_t*)(buf + HDR_LEN + 8)) = htons(short_fn.size());

		memcpy(buf + HDR_LEN + 8 + 2, short_fn.c_str(), short_fn.size());
		len = HDR_LEN + 8 + 2 + short_fn.size(); 

		folder_size -= short_fn.size();
		//std::cout<<"send:"<<short_fn<<std::endl;
		if (ndrudp_send(this->s_, buf, len, -1) != len)   
		{
			printf("send error\r\n");
			return false;
		}

		//tm = GetTickCount();
		gettimeofday(&t, 0);
		tm = t.tv_sec * 1000 + t.tv_usec/1000;
		while (!gfn_lst.empty())
		{
			std::string file = gfn_lst.front();
			bool bf = is_dir(file.c_str());

			std::string b = file.substr(folder_size + 1); 
			short_fn = b;

			if (bf) 
			{
				short_fn += "/"; 
				*((uint16_t*)buf) = htons(8 + 2 + short_fn.size()); 
				buf[2] = 1;
				buf[3] = 0;  
				*((uint64_t*)(buf + HDR_LEN)) = nd_htonll(0);
				*((uint16_t*)(buf + HDR_LEN + 8)) = htons(short_fn.size());

				memcpy(buf + HDR_LEN + 8 + 2, short_fn.c_str(), short_fn.size());
				len = HDR_LEN + 8 + 2 + short_fn.size();  

				//std::cout<<"send:"<<short_fn<<std::endl;
				if (ndrudp_send(this->s_, buf, len, -1) != len)   
				{
					printf("send error\r\n");
					return false;
				}

			}
			else  
			{

				if ((this->file_ = open(file.c_str(), O_RDONLY|O_CREAT)) == -1){
					printf("Open %s Error\n", file.c_str());
					return false;
				}

				struct stat statbuff;
				if (stat(file.c_str(), &statbuff) < 0){
					printf("stat Error\n");
					return false;
				}
				else{
					this->fsize_ = statbuff.st_size;
				}

				send_total_size += this->fsize_;
				*((uint16_t*)buf) = htons(8 + 2 + short_fn.size()); 
				buf[2] = 1;
				buf[3] = 0;
				*((uint64_t*)(buf + HDR_LEN)) = nd_htonll(this->fsize_);
				*((uint16_t*)(buf + HDR_LEN + 8)) = htons(short_fn.size());

				memcpy(buf + HDR_LEN + 8 + 2, short_fn.c_str(), short_fn.size());
				len = HDR_LEN + 8 + 2 + short_fn.size(); 

				//std::cout<<"send:"<<short_fn<<std::endl;
				if (ndrudp_send(this->s_, buf, len, -1) != len)     
				{
					printf("send error\r\n");
					return false;
				}

				while (1)
				{
					int rd_len = 0;
					rd_len = read(this->file_, buf + HDR_LEN, MAX_BUF_SIZE - HDR_LEN);

					if (rd_len <= 0)
						break;

					*((uint16_t*)buf) = htons(rd_len);
					buf[2] = 2;
					buf[3] = 0;

					int snd_len = ndrudp_send(this->s_, buf, rd_len + HDR_LEN, -1);
					if (snd_len != rd_len + HDR_LEN) {
						printf("send data error\r\n");
						return false;
					}
				}
				if (this->file_ != -1) {
					close(this->file_);
					this->file_ = -1;
				}
			}
			short_fn = "";
			gfn_lst.pop_front();
		}
		/// close socket 

		*((uint16_t*)buf) = htons(0); 
		buf[2] = 1;
		buf[3] = 2;  /// close symbol

		len = HDR_LEN; 

		if (ndrudp_send(this->s_, buf, len, -1) != len)   
		{
			printf("send error\r\n");
			return false;
		}

		//printf("file send finished, rate=%.2fMB\r\n", ((double)send_total_size) / (double(GetTickCount() - tm) / 1000.0f  + 0.1) / (double)(1024*1024));

		return true;
	}
	bool __svc()
	{
		int len = 0;
		int tmo = 10*1000;
		char *buf = this->buf_;

		struct timeval t;
		unsigned long tm;
		uint64_t send_total_size = 0;

		if (this->is_folder_)
		{ 
			if (!folder_send(send_total_size, tm)){
				printf("folder_send failed\n");
				return false;
			}		
		}
		else  /// file 
		{
			std::string short_fn = "";
			char *c = &this->fn_[this->fn_.size() - 1];
			int i = 0;
			for (; i != this->fn_.size(); i++) {  
				if (*c == '/' || *c == '\\') {
					short_fn = c + 1;
					break;
				}
				c--;
			}

			if (short_fn.size() == 0) {
				printf("invalid fn: %s\r\n", this->fn_.c_str());
				return false;
			}

			*((uint16_t*)buf) = htons(8 + 2 + short_fn.size());
			buf[2] = 1;
			buf[3] = 0;
			*((uint64_t*)(buf + HDR_LEN)) = nd_htonll(this->fsize_);
			*((uint16_t*)(buf + HDR_LEN + 8)) = htons(short_fn.size());
			
			send_total_size += this->fsize_;
			memcpy(buf + HDR_LEN + 8 + 2, short_fn.c_str(), short_fn.size());
			len = HDR_LEN + 8 + 2 + short_fn.size();

			if (ndrudp_send(this->s_, buf, len, -1) != len) {
				printf("send error\r\n");
				return false;
			}
sleep(1);

			//tm = GetTickCount();
			gettimeofday(&t, 0);
			tm = t.tv_sec * 1000 + t.tv_usec/1000;
			while (1) {
				int rd_len = 0;
				rd_len = read(this->file_, buf + HDR_LEN, MAX_BUF_SIZE - HDR_LEN);

				if (rd_len <= 0)
					break;

				*((uint16_t*)buf) = htons(rd_len);
				buf[2] = 2;
				buf[3] = 0;

				int snd_len = ndrudp_send(this->s_, buf, rd_len + HDR_LEN, -1);
				if (snd_len != rd_len + HDR_LEN) {
					printf("send data error\r\n");
					return false;
				}
sleep(1);
			}
			if (this->file_ != -1) {
				close(this->file_);
				this->file_ = -1;
			}
			//printf("file send finished, rate=%.2fMB\r\n", ((double)send_total_size) / (double(GetTickCount() - tm) / 1000.0f  + 0.1) / (double)(1024*1024));

		}

		if (HDR_LEN != ndrudp_recv(this->s_, buf, HDR_LEN, -1) || buf[2] != 3) {
			printf("send finished, but recv bye error\r\n");
			return true;
		}
		unsigned long tm_end = 0;	
		gettimeofday(&t, 0);
		tm_end = t.tv_sec * 1000 + t.tv_usec/1000;
		printf("file send finished, rate=%.2fMB\r\n", ((double)send_total_size) / (double(tm_end - tm) / 1000.0f  + 0.1) / (double)(1024*1024));

		return true;
	}
public:
	std::string fn_;
	unsigned long fsize_;
	std::string addr_;
	long ip_;
	int port_;
	char *buf_;
	int file_;
	bool is_folder_;
	ND_RUDP_SOCKET s_;
};

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("invalid args, app exit...\r\n");
		system("pause");
		return 0;
	}

	snd_file sf;
	if (!sf.open_t(argv[1], argv[2])) {
		printf("open error, app exit...\r\n");
		system("pause");
		return 0;
	}

	//sf.wait();
	//system("pause");
	return 0; 
}


