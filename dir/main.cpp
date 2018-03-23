#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>
#include <string>
#include <list>
#include <iostream>
#include <fcntl.h>

#define MAX_PATH_LEN 260

int count =0;
//char dirPath[MAX_PATH_LEN];

std::list<std::string> gfn_lst;

// traversal folder

void listAllFiles(char *dirname)
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
			listAllFiles(path);//递归调用
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
	if (S_ISDIR(s.st_mode))
	{
		return true;
	}
	else
	{
		return false;
	}
	return b;
}

bool 
dir_exist(const char *path)
{
	/*bool b = false;
	
	char tmp[MAX_PATH_LEN];
	//struct stat s;
	size_t n = safe_strncpy(tmp, path, sizeof(tmp)-2);

	for( int i=0; i< (int)n; ++i){
		if(tmp[i] == '/') tmp[i] = '\\';
	}

	if( tmp[n-1] == '\\' ) {
		tmp[n-1] = 0;
	}*/
	/*
	lstat(tmp, &s);
	if (S_ISDIR(s.st_mode))
	{
		return true;
	}
	else
	{
		return false;
	}*/
	printf("path :%s\n", path);
	if ( opendir(path) == NULL){
		printf("false\n");
		return false;
	}else{
		printf("true\n");
		return true;
	}
	//return b;
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
	printf("path :%s, n :%d\n",path, n);
	for( i=0; i<n; i++ ) {		

		if( szTmp[i] == '\\' || szTmp[i] == '/' ) {

			safe_strncpy( szPath, szTmp, i );
			printf("szpath: %s\n", szPath);
			mkdir( szPath, 0777 );
		}
	}

	return dir_exist(path);
}

int main(int argc, char **argv)
{

	if(argc != 2)
	{
		printf("one dir required!(for eample: ./a.out /home/myFolder)\n");
		exit(1);
	}
	//strcpy(dirPath,argv[1]);
	/*listAllFiles(argv[1]);
	printf("total files:%d\n",count);
	std::list<std::string>::iterator it;
	for (it = gfn_lst.begin(); it != gfn_lst.end(); it++)
	{
		std::cout<<*it<<std::endl;
	}
	printf("\n");*/

	//printf("is_dir :%d\n", is_dir(argv[1]));
	if(dir_create(argv[1]))
		printf("create success\t\n");	
	else
		printf("create failed\t\n");
	/*printf("dir_exist :%d\n", dir_exist(argv[1]));
	printf("is_dir :%d\n", is_dir(argv[1]));
	if ( opendir(argv[1]) == NULL){
		printf("false\n");
		return false;
	}else{
		printf("true\n");
		return true;
	}*/

	return 0;
}

