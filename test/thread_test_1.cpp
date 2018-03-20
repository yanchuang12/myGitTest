#include<iostream>
#include"/usr/include/pthread.h"
using namespace std;

#define NUM_THREAD 5

void* sayhello(void *args)
{
    cout<<"hello ..."<<endl;
}

int main()
{
    pthread_t tids[NUM_THREAD];
    for(int i = 0;i < NUM_THREAD;++i)
    {
        int ret = pthread_create(&tids[i],NULL,sayhello,NULL);
	if(ret != 0)
		cout<<"error"<<endl;
    }
	pthread_exit(NULL);
    return 0;
}
