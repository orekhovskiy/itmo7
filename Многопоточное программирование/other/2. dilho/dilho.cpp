#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <iostream>
#include <atomic>
#include <map>
#include <chrono>



struct consumeArgs_t {
	pthread_mutex_t* mutex;
	bool isdebug;
	bool* exit;
	bool* updated;
	long long int* value;
	int* updated_id;
	int sleep;
	void set(
		pthread_mutex_t* mute_x,
		bool isdebu_g,
		bool* exi_t,
		bool* update_d,
		long long int* valu_e,
		int* updated_i_d,
		int slee_p)
	{
		mutex = mute_x;
		isdebug = isdebu_g;
		exit = exi_t;
		updated = update_d;
		value = valu_e;
		updated_id = updated_i_d;
		sleep = slee_p;
	}
};

struct produceArgs_t {
	pthread_mutex_t* mutex;
	bool* updated;
	bool* exit;
	long long int* value;
	int N_value;
	produceArgs_t(
		pthread_mutex_t* mute_x,
		bool* update_d,
		bool* exi_t,
		long long* valu_e,
		int N_valu_e) : mutex(mute_x), updated(update_d), exit(exi_t), value(valu_e), N_value(N_valu_e)
	{ }
};

struct interruptArgs_t {
	int N_value;
	int* updated_id;
	bool* exit;
	interruptArgs_t(
		bool* exi_t,
		int* updated_i_d,
		int N_valu_e) : N_value(N_valu_e), updated_id(updated_i_d), exit(exi_t)
	{ }
};


int run_treads(const int N_thread, int timer_thread, bool isdebug);
int get_tid();
void* producer_routine(void* param);
void* consumer_routine(void* param);
void* consumer_interruptor_routine(void* param);

int main(int argc, char* argv[])
{
	auto start_time = std::chrono::high_resolution_clock::now();
	if (argc == 4) {
		if (std::string(argv[1]).compare("-debug") == 0)
		{
			const int N_thread = atoi(argv[2]);
			int timer_thread = atoi(argv[3]);
			if(N_thread <1 || timer_thread < 0)
				return 1;			

			std::cout << run_treads(N_thread, timer_thread, 1) << std::endl;
		}

		if (std::string(argv[3]).compare("-debug") == 0)
		{
			const int N_thread = atoi(argv[1]);
			int timer_thread = atoi(argv[2]);
			if(N_thread <1 || timer_thread < 0)
				return 1;
			std::cout << run_treads(N_thread, timer_thread, 1) << std::endl;
		}
	}
	if (argc == 3) {
		const int N_thread = atoi(argv[1]);

		int timer_thread = atoi(argv[2]);
		if(N_thread <1 || timer_thread < 0)
			return 1;
		std::cout << run_treads(N_thread, timer_thread, 0) << std::endl;
	}
	if(argc != 3 && argc != 4)
		return 1; 
	auto end_time = std::chrono::high_resolution_clock::now();
	std::cout << "Time difference:" << std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() << " milliseconds" << std::endl;
}


void* producer_routine(void* arg)
{
	produceArgs_t* produceArg = (produceArgs_t*)arg;

	std::string str;
	std::getline(std::cin, str);

	size_t lastpos = 0;
	std::string subline;

	for (size_t pos = str.find_first_of(' ', 0); pos != std::string::npos; pos = str.find_first_of(' ', lastpos))
	{
		subline = str.substr(lastpos, pos - lastpos);
		lastpos = pos + 1;

		while (*produceArg->updated)
			usleep(0);

		*produceArg->value = atoll(subline.c_str());
		*produceArg->updated = 1;
	}

	subline = str.substr(lastpos, str.length() - lastpos);
	while (*produceArg->updated)
		usleep(0);

	*produceArg->value = atoll(subline.c_str());
	*produceArg->updated = 1;

	*produceArg->exit = 1;

	pthread_exit(0);
	return nullptr;
}

void* consumer_routine(void* arg)
{
	consumeArgs_t* consumeArg = (consumeArgs_t*)arg;
	long long int sum = 0;
	while (!(*consumeArg->exit) || *consumeArg->updated != 0)
	{
		if (*consumeArg->updated != 0 )
		{
			pthread_mutex_lock(consumeArg->mutex);
			if (*consumeArg->updated != 0 )
			{
				sum += *consumeArg->value;
				*consumeArg->updated = 0;
				pthread_mutex_unlock(consumeArg->mutex);
				if (consumeArg->isdebug){
					std::cout << "(" + std::to_string(get_tid()) + ", " + std::to_string(sum) + ")\n";
				}
				if (*consumeArg->updated_id != get_tid())
					if(consumeArg->sleep != 0)
						usleep(rand() % consumeArg->sleep);
					else
						usleep(0);
				else
					*consumeArg->updated_id = 0;
			}
			else
				pthread_mutex_unlock(consumeArg->mutex);
		}
		usleep(100);
	}
	pthread_exit((void*) sum);
	return nullptr;
}

void* consumer_interruptor_routine(void* arg)
{
	
	interruptArgs_t* interruptArg = (interruptArgs_t*)arg;
	while (!(*interruptArg->exit))
	{
		if (*interruptArg->updated_id == 0)
			*interruptArg->updated_id = rand() % interruptArg->N_value;
		usleep(0);
	}
	pthread_exit(0);
	return nullptr;
}


int get_tid()
{
	static std::atomic<int> count_threads;
	thread_local int id = count_threads++;
	return id;
}


int run_treads(const int N_thread, int timer_thread, bool isdebug)
{
	pthread_mutex_t mutex;
	pthread_mutex_t mutexx;
	std::map<pthread_t, int> heap;
	bool updated = 0, exit = 0;
	long long int value = 0;
	int updated_id = 0;
	std::vector<pthread_t> threads(N_thread + 2);
	timer_thread*=1000;
	std::vector<consumeArgs_t> consumeArg(N_thread);
	if(pthread_mutex_init(&mutex, NULL)!=0)
		std::cout<<"sgsfgsfgsfgsgfsfg\n";
	if(pthread_mutex_init(&mutexx, NULL)!=0)
		std::cout<<"sgsfgsfgsfgsgfsfg\n";
	///consumers
	for (int i = 0; i < N_thread; i++)
	{
		consumeArg[i].set(&mutex, isdebug, &exit, &updated, &value, &updated_id, timer_thread);
		pthread_create(&threads[i + 2], NULL, consumer_routine, (void*)&(consumeArg.at(i)));

		/* ���� ���������� ���������� ������ */
	}

	///producer
	/* ������� ����� ����� */
	produceArgs_t produceArg(&mutex, &updated, &exit, &value, N_thread);
	pthread_create(&threads[0], NULL, producer_routine, (void*)&produceArg);

	///interruptor
	/* ������� ����� ����� */
	interruptArgs_t interrupArg(&exit, &updated_id, N_thread);
	pthread_create(&threads[1], NULL, consumer_interruptor_routine, (void*)&interrupArg);

	int result = 0, sum = 0;
	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);
	for (int i = 0; i < N_thread; i++)
	{
		pthread_join(threads[i + 2], (void**)&result);
		sum += result;
	}
	
	return sum;
}
