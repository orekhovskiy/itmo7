#include <pthread.h>
#include <iostream>
#include <vector>

struct arg_t {
	std::vector<pthread_mutex_t> mutexes;
	arg_t(size_t size)
	{
		mutexes.resize(size);
		for (size_t i = 0; i < size; i++)
		{
			if (pthread_mutex_init(&mutexes[i], NULL) != 0)
				std::cout << "sgsfgsfgsfgsgfsfg\n";
		}
	}
};

void* thread0_routine(void* arg)
{
	arg_t* thread_arg = (arg_t*)arg;


	pthread_mutex_lock(&thread_arg->mutexes[0]);
	pthread_mutex_lock(&thread_arg->mutexes[1]);

//	std::cout << "0 is alive" << std::endl;

	pthread_mutex_unlock(&thread_arg->mutexes[1]);
	pthread_mutex_unlock(&thread_arg->mutexes[0]);

	pthread_exit(0);
	return nullptr;
}

void* thread1_routine(void* arg)
{
	arg_t* thread_arg = (arg_t*)arg;


	pthread_mutex_lock(&thread_arg->mutexes[1]);
	pthread_mutex_lock(&thread_arg->mutexes[2]);

//	std::cout << "1 is alive" << std::endl;

	pthread_mutex_unlock(&thread_arg->mutexes[2]);
	pthread_mutex_unlock(&thread_arg->mutexes[1]);

	pthread_exit(0);
	return nullptr;
}

void* thread2_routine(void* arg)
{
	arg_t* thread_arg = (arg_t*)arg;


	pthread_mutex_lock(&thread_arg->mutexes[2]);
	pthread_mutex_lock(&thread_arg->mutexes[0]);

//	std::cout << "2 is alive" << std::endl;

	pthread_mutex_unlock(&thread_arg->mutexes[0]);
	pthread_mutex_unlock(&thread_arg->mutexes[2]);

	pthread_exit(0);
	return nullptr;
}

int main()
{
	arg_t thread_arg(3);
	
	std::vector<pthread_t> threads;
	threads.resize(3);

	pthread_create(&threads[0], NULL, thread0_routine, (void*)&thread_arg);
	pthread_create(&threads[1], NULL, thread1_routine, (void*)&thread_arg);
	pthread_create(&threads[2], NULL, thread2_routine, (void*)&thread_arg);

	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);
	pthread_join(threads[2], NULL);
	return 1;
}
