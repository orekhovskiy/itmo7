#include <iostream>
#include <pthread.h>
#include <string>
#include <sstream>
#include <queue>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <atomic>
#include <memory>
#include <cassert>
#include <time.h>

typedef struct {
    int sum;
} thread_st;

int threads_count;
int max_sleep_delay;
bool debug = false;
bool calc_end = false;

pthread_t* consumer_threads;

pthread_barrier_t init_barrier;

pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_add_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t buffer_take_cond = PTHREAD_COND_INITIALIZER;

std::queue<std::string> buffer;

void* consumer_routine(void* arg);

int getRandomNumber(int min, int max)
{
    static const double fraction = 1.0 / (static_cast<double>(RAND_MAX) + 1.0);
    // Равномерно распределяем рандомное число в нашем диапазоне
    return static_cast<int>(rand() * fraction * (max - min + 1) + min);
}

void buffer_add(const std::string &number) {
    pthread_mutex_lock(&buffer_mutex);

    buffer.push(number);
    pthread_cond_signal(&buffer_take_cond);

    pthread_mutex_unlock(&buffer_mutex);
}

std::string buffer_take() {
    pthread_mutex_lock(&buffer_mutex);

    while (buffer.empty()) {
        if (buffer.empty() && calc_end) {
            pthread_mutex_unlock(&buffer_mutex);
            return "";
        }

        pthread_cond_wait(&buffer_take_cond, &buffer_mutex);
    }

    auto string = buffer.front();
    buffer.pop();
    pthread_cond_signal(&buffer_add_cond);

    pthread_mutex_unlock(&buffer_mutex);

    return string;
}

int get_tid() {
    static std::atomic<int> total(0);
    thread_local std::unique_ptr<int> id(new int);
    if (*id == 0) {
        *id = ++total;
    }
    return *id;
}

void* producer_routine() {
  // Wait for consumer to start
 
    pthread_barrier_wait(&init_barrier);
  // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    std::string inputString;
    getline(std::cin, inputString);
    std::stringstream ss;
    ss << inputString;
    std::string value;
    while (ss >> value) {
        buffer_add(value);
    }

    calc_end = true;
    pthread_cond_broadcast(&buffer_take_cond);
    return nullptr;
}
 
void* consumer_routine(void* arg) {
  // notify about start
  // for every update issued by producer, read the value and add to sum
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

    auto state = (thread_st*) arg;
    state->sum = 0;
    
    pthread_barrier_wait(&init_barrier);

    while (true) {
        auto value = buffer_take();
        if (value.empty()) {
            break;
        }
        try {
            state->sum += std::stoi(value);
        } catch (const std::exception &e) {
        }
        if (debug) {
            static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

            pthread_mutex_lock(&m);
            std::cout << get_tid() << " " << state->sum << std::endl;
            pthread_mutex_unlock(&m);
        }
        usleep(getRandomNumber(0, max_sleep_delay));
    }
    return nullptr;
}
 
void* consumer_interruptor_routine() {
  // wait for consumers to start
    pthread_barrier_wait(&init_barrier);
  // interrupt random consumer while producer is running
    while (!calc_end) {
        pthread_cancel(consumer_threads[getRandomNumber(0, threads_count - 1)]);
    }
    return nullptr;
}

int run_threads() {
    pthread_barrier_init(&init_barrier, nullptr, threads_count + 2);

    pthread_t producer_thread;
    pthread_t interrupter_thread;
    
    consumer_threads = new pthread_t[threads_count];
    auto consumer_states = new thread_st[threads_count];


    pthread_create(&producer_thread, nullptr, reinterpret_cast<void* (*)(void*)>(producer_routine), nullptr);
    pthread_create(&interrupter_thread, nullptr, reinterpret_cast<void* (*)(void*)>(consumer_interruptor_routine), nullptr);
    
    for (int i = 0; i < threads_count; ++i) {
        pthread_create(&consumer_threads[i], nullptr, consumer_routine, &consumer_states[i]);
    }

    pthread_join(producer_thread, nullptr);
    pthread_join(interrupter_thread, nullptr);

    auto sum = 0;

    for (auto i = 0; i < threads_count; ++i) {
        pthread_join(consumer_threads[i], nullptr);
        sum += consumer_states[i].sum;
    }

    delete[] consumer_threads;
    delete[] consumer_states;

    return sum;
}
 
int main(int argc, char* argv[]) {
    if (argc < 3) {
        return 1;
    }

    try {
        threads_count = std::stoi(argv[1]);
        max_sleep_delay = std::stoi(argv[2]);
    } catch (const std::exception &e) {
        return 1;
    }

    if (argc == 4) {
        std::string debug_key = argv[3];
        debug = debug_key == "-debug";
    }

    if (threads_count < 1 || max_sleep_delay < 0) {
        return 1;
    }
    srand(time(NULL));
    auto time_start = clock();
    std::cout << run_threads() << std::endl;
	printf("Time taken: %.2fs\n", (double)(clock() - time_start)/CLOCKS_PER_SEC);
    return 0;
}
