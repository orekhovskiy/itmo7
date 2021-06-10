#include <pthread.h>
#include <iostream>

void* thread_routine(void* arg)
{
	pthread_mutex_t mutex;
	arg=arg;
	if (pthread_mutex_init(&mutex, NULL) != 0)
		std::cout << "sgsfgsfgsfgsgfsfg\n";
	
	pthread_mutex_lock(&mutex);
	pthread_mutex_unlock(&mutex);
	pthread_mutex_lock(&mutex);
	pthread_mutex_unlock(&mutex);

	pthread_exit(0);
	return nullptr;
}


int main()
{
	
	pthread_t thread;
	
	pthread_create(&thread, NULL, thread_routine, NULL);

	pthread_join(thread, NULL);
	return 1;
}
