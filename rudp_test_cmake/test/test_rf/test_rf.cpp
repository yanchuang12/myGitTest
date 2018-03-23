// test_rf.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"

#include "../../rudp/nd_core/nd_typedef.h"
// #include <utility/api_tools.h>
// #include <sys/sys_asy.h>
// #include <sys/sys_thread.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "../../rudp/nd_rudp_api.h"

#define MAX_PATH_LEN 260

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

bool
is_dir2(char *path)
{
	int r;
	struct stat info;
	r = stat(path, &info);

	if (r == 0) {
		return info.st_mode & S_IFDIR;
	}
	else {
		return -1;
	}

	//return (s == 0 && (info.st_mode & S_ISDIR));
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


/*bool 
dir_exist(const char *path)
{
	bool b = false;
	struct stat s;
	char tmp[MAX_PATH_LEN];

	size_t n = safe_strncpy(tmp, path, sizeof(tmp)-2);

	for( int i=0; i< (int)n; ++i){
		if(tmp[i] == '/') tmp[i] = '\\';
	}

	if( tmp[n-1] == '\\' ) {
		tmp[n-1] = 0;
	}

	lstat(tmp, &s);
	if (S_ISDIR(s.st_mode)){
		return true;
	}
	else{
		return false;
	}
	return b;
}*/

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

class rcv_file // : public xy::thread_base
{
	enum{MAX_BUF_SIZE = 1024 * 32, HDR_LEN = 4};
public:
	rcv_file()
	{
		this->buf_ = new char[MAX_BUF_SIZE];
		this->file_ = -1;
		this->listen_s_ = ND_RUDP_INVALID_SOCKET;
		this->s_ = ND_RUDP_INVALID_SOCKET;
		ndrudp_init();
		ndrudp_init_log(0xffff, 0x01, "d:/nd_rudp_rcv.log");
	}
	~rcv_file()
	{
		if (this->s_ != ND_RUDP_INVALID_SOCKET) {
			ndrudp_close(this->s_);
		}

		if (this->listen_s_ != ND_RUDP_INVALID_SOCKET) {
			ndrudp_close(this->listen_s_);
		}

		/*if (this->file_ != -1) {
			//CloseHandle(this->file_);
			close(this->file_);
			this->file_ = -1;//INVALID_HANDLE_VALUE;
		}*/

		//shut_down();
		//wait();
		delete []this->buf_;

		ndrudp_uninit();
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

		this->listen_s_ = ndrudp_socket(0, this->listen_port_, -1, 1);
		if (ND_RUDP_INVALID_SOCKET == this->listen_s_) {
			printf("ndrudp_socket error\r\n");
			return false;
		}

		if (0 != ndrudp_listen(this->listen_s_)) {
			printf("ndrudp_listen error\r\n");
			return false;
		}
int delay_lv = ND_RUDP_DELAY_LV_HIGH;
		ndrudp_set_opt(this->listen_s_, ND_RUDP_OPT_LV_SO, ND_RUDP_OPT_NM_SO_DELAY_LEVEL, &delay_lv, sizeof(delay_lv));
while(1){
		ndrudp_poll_t poll_in, poll_out;
		ndrudp_poll_zero(&poll_in);
		ndrudp_poll_zero(&poll_out);
		ndrudp_poll_add_event(&poll_in, this->listen_s_, ND_RUDP_IO_READ);
		ndrudp_poll_wait(&poll_in, &poll_out, 5);}

		printf("listen at %d\r\n", this->listen_port_);

		//active();

		svc();
		return true;
	}
public:
	int svc()
	{
		bool b = __svc();
		ndrudp_close(this->listen_s_);
		this->listen_s_ = ND_RUDP_INVALID_SOCKET;
		return 0;
	}
	bool folder_recv(char cflag, uint64_t &recv_total_size, unsigned long &tm)
	{
		char *buf = this->buf_;

		//tm = GetTickCount();
		struct timeval t;
		gettimeofday(&t, 0);
		tm = t.tv_sec * 1000 + t.tv_usec/1000;
		recv_total_size = 0;
		while (1)
		{
			if (HDR_LEN != ndrudp_recv(this->s_, buf, HDR_LEN, -1, ND_RUDP_R_FLAG_RCV_N)) { 
				printf("ndrudp_recv head error\r\n");
				return false;
			}

			/// close symbol
			if (buf[3] == 2)
			{
				//printf("file recv finished, rate=%.2fMB\r\n", ((double)send_total_size) / (double(GetTickCount() - tm) / 1000.0f + 0.1) / (double)(1024.*1024));
				return true;
			}

			uint16_t nbody = ntohs(*(uint16_t*)buf);

			if (nbody != ndrudp_recv(this->s_, buf, nbody, -1, ND_RUDP_R_FLAG_RCV_N)) {
				printf("ndrudp_recv body error\r\n");
				return false;
			}
			buf[nbody] = 0;
			cflag = buf[nbody-1];
			if (cflag == '/')  /// folder
			{
				this->fn_ = this->dir_ + (buf + 8 + 2);
				dir_create(this->fn_.c_str());
				//printf("dir_create filename:%s\t\n",this->fn_.c_str());
			}
			else
			{
				this->fn_ = this->dir_ + (buf + 8 + 2);

				uint64_t fsize = nd_ntohll(*(uint64_t*)buf);
				uint16_t fn_len = ntohs(*((uint16_t*)(buf + 8)));

				//printf("filename:%s, fsize:%ld, fn_len:%d\n", this->fn_.c_str(), fsize, (int)fn_len);
				if (fn_len > 1024) {
					printf("ndrudp_recv body error, invalid file_size\r\n");
					return false;
				}

				if ((this->file_ = open(this->fn_.c_str(), O_RDWR|O_CREAT|O_TRUNC, 00664)) == -1){
					printf("Open %s Error\n", this->fn_.c_str());
					return false;
				}
				printf("start recv file: %s...\r\n", this->fn_.c_str());

				uint64_t total_size = 0;
				while (1) 
				{
					if (total_size == fsize) {
						//printf("file recv finished, rate=%.2fMB\r\n", ((double)fTotalSize) / (double(GetTickCount() - tm) / 1000.0f + 0.1) / (double)(1024.*1024));
						recv_total_size += fsize;
						break;
					}

					if (HDR_LEN != ndrudp_recv(this->s_, buf, HDR_LEN, -1, ND_RUDP_R_FLAG_RCV_N) || buf[2] != 2) {
						printf("ndrudp_recv head error\r\n");
						return false;
					}

					nbody = ntohs(*(uint16_t*)buf);
					if (nbody > MAX_BUF_SIZE) {
						printf("invalid body length\r\n");
						assert(0);
						return false;
					}

					int rcv_len = ndrudp_recv(this->s_, buf, nbody, -1, ND_RUDP_R_FLAG_RCV_N);
					if (rcv_len != nbody) {
						printf("recv body error\r\n");
						return false;
					}

					total_size += rcv_len;

					//DWORD wt_len = 0;
					//BOOL ret = WriteFile(this->file_, buf, rcv_len, &wt_len, NULL);
					uint64_t wt_len = write(this->file_, buf, rcv_len);
					if (-1 == wt_len || wt_len != rcv_len) {
						printf("WriteFile error\r\n");
						return false;
					}
				}  // end of while
				if (this->file_ != -1) {
					close(this->file_);
					this->file_ = -1;
				}
			} // end of else 
		} // end of while (i < nDirNum)
	}
	bool __svc()
	{
		while (1) {
			int r = ndrudp_accept(this->listen_s_, &this->s_, 3*1000);
			if (r == 0)
				break;
			if (r == -2)
				continue;
			else
				return false;
		}

		printf("accept a connection\r\n");

		struct timeval t;
		char *buf = this->buf_;
		int len = 0;
		int tmo = 10*1000;
		char cflag = '\0';
		if (HDR_LEN != ndrudp_recv(this->s_, buf, HDR_LEN, -1, ND_RUDP_R_FLAG_RCV_N) || buf[2] != 1) { 
			printf("ndrudp_recv head error\r\n");
printf("buf[2] =%c...\n",buf[2]);

			return false;
		}

		uint16_t nbody = ntohs(*(uint16_t*)buf);

		if (nbody != ndrudp_recv(this->s_, buf, nbody, -1, ND_RUDP_R_FLAG_RCV_N)) {
			printf("ndrudp_recv body error\r\n");
			return false;
		}
		buf[nbody] = 0;

		cflag = buf[nbody-1];
		uint64_t recv_total_size = 0;
		//DWORD tm;
		unsigned long tm;
		if (cflag == '/')  /// folder symbol
		{
			this->fn_ = this->dir_ + (buf + 8 + 2);
			dir_create(this->fn_.c_str());
			//printf("dir_create filename:%s\t\n",this->fn_.c_str());

			if (!folder_recv(cflag, recv_total_size, tm))
			{
				printf("folder_recv failed\n");
				return false;
			}
		}
		else
		{
			uint64_t fsize = nd_ntohll(*(uint64_t*)buf);
			uint16_t fn_len = ntohs(*((uint16_t*)(buf + 8)));

			if (fn_len > 1024) {
				printf("ndrudp_recv body error, invalid file_size\r\n");
				return false;
			}

			this->fn_ = this->dir_ + (buf + 8 + 2);

			if ((this->file_ = open(this->fn_.c_str(), O_RDWR|O_CREAT|O_TRUNC, 00664)) == -1){
				printf("Open %s Error , error:%d\n", this->fn_.c_str(), errno);
				return false;
			}
			printf("start recv file: %s...\r\n", this->fn_.c_str());
			//tm = GetTickCount();
			gettimeofday(&t, 0);
			tm = t.tv_sec * 1000 + t.tv_usec/1000;
			while (1) {
				if (recv_total_size == fsize) {
					//printf("file recv finished, rate=%.2fMB\r\n", ((double)fsize) / (double(GetTickCount() - tm) / 1000.0f + 0.1) / (double)(1024.*1024));
					
					break;
				}

				if (HDR_LEN != ndrudp_recv(this->s_, buf, HDR_LEN, -1, ND_RUDP_R_FLAG_RCV_N) || buf[2] != 2) {
					printf("ndrudp_recv head error\r\n");
					return false;
				}

				nbody = ntohs(*(uint16_t*)buf);
				if (nbody > MAX_BUF_SIZE) {
					printf("invalid body length\r\n");
					assert(0);
					return false;
				}

				int rcv_len = ndrudp_recv(this->s_, buf, nbody, -1, ND_RUDP_R_FLAG_RCV_N);
				if (rcv_len != nbody) {
					printf("recv body error. rcv_len :%d,nbody :%d\r\n", rcv_len,nbody);
					return false;
				}

				recv_total_size += rcv_len;

				uint64_t wt_len = write(this->file_, buf, rcv_len);
				if (-1 == wt_len || wt_len != rcv_len) {
					printf("WriteFile error, wt_len :%d, rcv_len :%d, error :%d \r\n", wt_len, rcv_len, errno);
					return false;
				}
			}
			if (this->file_ != -1) {
				close(this->file_);
				this->file_ = -1;
			}
		}

		*((uint16_t*)buf) = htons(0);
		buf[2] = 3;
		buf[3] = 0;

		unsigned long tm_end = 0;	
		gettimeofday(&t, 0);
		tm_end = t.tv_sec * 1000 + t.tv_usec/1000;
		if (HDR_LEN != ndrudp_send(this->s_, buf, HDR_LEN, -1, ND_RUDP_S_FLAG_SND_N)) {
			printf("recv finished, but send bye error, rate=%.2fMB\r\n", ((double)recv_total_size) / (double(tm_end - tm) / 1000.0f + 0.1) / (double)(1024.*1024));
			return false;
		}
		
		printf("file recv finished, rate=%.2fMB\r\n", ((double)recv_total_size) / (double(tm_end - tm) / 1000.0f + 0.1) / (double)(1024.*1024));

		ndrudp_close(this->s_);
		this->s_ = ND_RUDP_INVALID_SOCKET;

		return true;
	}
public:
	std::string dir_;
	std::string fn_;
	int listen_port_;
	ND_RUDP_SOCKET listen_s_;
	ND_RUDP_SOCKET s_;
	int file_;
	char *buf_;
};

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("invalid args, app exit...\r\n");
		return 0;
	}

	rcv_file rf;
	if (!rf.open_t(argv[1], atoi(argv[2]))) {
		printf("open error, app exit...\r\n");
		return 0;
	}

	//rf.wait();
	//system("pause");
	return 0; 
}

