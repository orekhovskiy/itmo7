#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

//#define DEBUG

//#define IM_WINDOWS

#include <dlfcn.h>
#include <pthread.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <map>
#include <string.h>
#include <fstream>
#ifdef IM_WINDOWS
typedef void *pthread_uid;
#else
typedef pthread_t pthread_uid;
#endif
std::vector<unsigned int> used, path;
int flag = 0, n, m;

std::map<pthread_uid, std::vector<pthread_mutex_t *>> uid_mutex_map;
std::map<pthread_mutex_t *, unsigned int> mutex_graph_map;

typedef int (*real_mutex_lock_t)(pthread_mutex_t *);
typedef int (*real_mutex_unlock_t)(pthread_mutex_t *);


int real_mutex_lock(pthread_mutex_t *mutex)
{
    real_mutex_lock_t orig_lock = (real_mutex_lock_t)dlsym(RTLD_NEXT, "pthread_mutex_lock");
    if (orig_lock)
        return orig_lock(mutex);
    else
        return 0;
}

int real_mutex_unlock(pthread_mutex_t *mutex)
{
    real_mutex_unlock_t orig_unlock = (real_mutex_unlock_t)dlsym(RTLD_NEXT, "pthread_mutex_unlock");
    if (orig_unlock)
        return orig_unlock(mutex);
    else
        return 0;
}

void not_zero_return_code()
{
    exit(1);
}

void zero_return_code()
{
    exit(0);
}
#define BT_BUF_SIZE 100
#include <execinfo.h>

void myfunc3(void)
{
    int nptrs;
    void *buffer[BT_BUF_SIZE];
    char **strings;

    nptrs = backtrace(buffer, BT_BUF_SIZE);
    printf("backtrace() returned %d addresses\n", nptrs);

    /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
              would produce similar output to the following: */

    strings = backtrace_symbols(buffer, nptrs);
    if (strings == NULL)
    {
        perror("backtrace_symbols");
    }

    for (int j = 0; j < nptrs; j++)
        printf("%s\n", strings[j]);

    free(strings);
}

class graph
{
    std::vector<std::vector<int>> matrix;

    ///////
    // ����� � ������� ��� ������ �������
    void dfs(int v)
    {
        if (flag == 1)
            return; // ���� ��� ����� ����, �� ���������������
        else
        {
            used[v] = 1;       // �������� �������
            path.push_back(v); // ��������� �� � ������� ������ �����
            int to;
            for (unsigned int i = 0; i < size(v); i++)
            {
                to = at(v, i); // ��������� ������� �����
                if (to == 1)
                {
                    if (used[i] == 1)
                    { // ���� �� �� ��������, �� �� ����� �� ���, ������ �� ����� ����
                        //path.push_back(i); // ��������� ��������� ������� � ������� ������ �����
                        flag = 1; // ������ ���������, ��� �� ����� ���� � ���������������
                        return;
                    }
                    else
                    {
                        dfs(i); // ���� �� ��������, �� ��������
                    }
                }
                if (flag == 1)
                    return; // ���� ����� ����, �� ���������������
            }
            used[v] = 2; // ���� �� ����� ����, �� ������� �� �������
            path.pop_back();
        }
    }

public:
    void add(unsigned int old_pos, unsigned int new_pos)
    {
        if (matrix.size() < old_pos + 1)
            matrix.resize(old_pos + 1);
        if (matrix[old_pos].size() < new_pos + 1)
            matrix[old_pos].resize(new_pos + 1, 0);

        matrix[old_pos][new_pos] = 1;
    }

    void print()
    {
        for (unsigned int i = 0; i < matrix.size(); i++)
        {
            for (unsigned int j = 0; j < matrix[i].size(); j++)
                std::cout << matrix[i][j] << " ";
            std::cout << std::endl;
        }
        std::cout << std::endl
                  << "---" << std::endl;
    }

    int &at(unsigned int from, unsigned int to)
    {
        return matrix[from][to];
    }

    bool isdead()
    {
        if (mutex_graph_map.size())
            for (unsigned int i = 0; i < mutex_graph_map.size(); i++)
            {
                if (used[i] == 0)
                { // ���� �� �������� �������, �� �������� ��
                    dfs(i);
                    if (flag == 1)
                    {
                        flag = 0;
                        used.assign(used.size(), 0);
                        return 1; // ���� ����� ����, �� ���������������
                    }
                }
            }
        used.assign(used.size(), 0);
        path.resize(0);
        return 0;
    }

    size_t size()
    {
        return matrix.size();
    }

    size_t size(unsigned int from)
    {
        if (from >= size())
            return 0;
        return matrix[from].size();
    }
};

graph deadgraph;

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    static pthread_mutex_t mute_x = PTHREAD_MUTEX_INITIALIZER;
    int result;

    // Our malicious code
    real_mutex_lock(&mute_x);
#ifdef IM_WINDOWS
    pthread_uid uid = (void *)pthread_self().p;
    auto it4prevmutex = uid_mutex_map.find(pthread_self().p);
    if (it4prevmutex == uid_mutex_map.end())
    {
        it4prevmutex = uid_mutex_map.insert(std::pair<pthread_uid, pthread_mutex_t *>((void *)pthread_self().p, mutex)).first;
    } //now there is needed previous mutex in it4prevmutex->second and if uid is new, then current and previous are the same
#else
    //pthread_uid uid = pthread_self();
    auto it4prevmutex = uid_mutex_map.find(pthread_self());
    
    auto it4graphpos_prev = mutex_graph_map.find(0);
    if (it4prevmutex != uid_mutex_map.end())
        it4graphpos_prev = mutex_graph_map.find(it4prevmutex->second.at(0));
    
    if (it4prevmutex == uid_mutex_map.end())
    {
        std::vector<pthread_mutex_t *> vec(1, mutex);
        it4prevmutex = uid_mutex_map.insert(std::pair<pthread_uid, std::vector<pthread_mutex_t *>>(pthread_self(), vec)).first;
    } //now there is needed previous mutex in it4prevmutex->second and if uid is new, then current and previous are the same
#endif

    auto it4graphpos_curr = mutex_graph_map.find(mutex);
    if (it4graphpos_curr == mutex_graph_map.end())
    { //if current mutex is new in graph
        it4graphpos_curr = mutex_graph_map.insert(std::pair<pthread_mutex_t *, unsigned int>(mutex, mutex_graph_map.size())).first;
        used.push_back(0);

        //        if (it4graphpos_prev == mutex_graph_map.end())//if uid is new, then old mutex might be in end too, so we need to keep it equal with current
        //            it4graphpos_prev = mutex_graph_map.find(it4graphpos_curr->first);//instead of it4graphpos_prev = it4graphpos_curr;

    } //now there is needed position in graph of old mutex in it4graphpos_prev->second and if uid is new, then current and previous are the same

    //    if (it4graphpos_prev != it4graphpos_curr)
    //        deadgraph.add(it4graphpos_prev->second, it4graphpos_curr->second);

    if (find(it4prevmutex->second.begin() + 1, it4prevmutex->second.end(), mutex) == it4prevmutex->second.end())
    {
        it4prevmutex->second.push_back(mutex);

        /*std::cout << "Vector " << pthread_self() << ": ";
     for (size_t i = 0; i < it4prevmutex->second.size(); i++)
            std::cout << " " << it4prevmutex->second.at(i);
        std::cout << std::endl
                  << "------" << std::endl;*/
    }

    if (it4graphpos_prev != mutex_graph_map.end())
        deadgraph.add(it4graphpos_prev->second, it4graphpos_curr->second);

    it4prevmutex->second.at(0) = mutex;

    //deadgraph.print();

    if (deadgraph.isdead())
    {
        //help-message
        //atexit(not_zero_return_code);
        std::cout << "You are dead! (potentially)" << std::endl;
        std::cout << "there is mutex lock cycle between: " << std::endl;

        /*for (const auto& [key, value] : someMap)
    if (value == someValue)
        return key;*/

        for (size_t i = 0; i < path.size(); i++)
        {
            for (auto ya : mutex_graph_map)
                if (path.at(i) == ya.second)
                {
                    std::cout << (void *)ya.first << std::endl;
                    break;
                }
        }

        path.resize(0);
        myfunc3();
    }
    //atexit(experimental_print);
    //exit(0);
    real_mutex_unlock(&mute_x);

    // Perform the actual system call
    result = real_mutex_lock(mutex);
    // atexit(zero_return_code);
    // Behave just like the regular syscall would
    return result;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    static pthread_mutex_t mute_xx = PTHREAD_MUTEX_INITIALIZER;
    int result;

    // Our malicious code
    real_mutex_lock(&mute_xx);

    auto it4prevmutex = uid_mutex_map.find(pthread_self());

    it4prevmutex->second.erase(find(it4prevmutex->second.begin() + 1, it4prevmutex->second.end(), mutex));

    if(it4prevmutex->second.size() == 1)
        it4prevmutex->second.at(0) = 0;

    /*std::cout << "Vector " << pthread_self() << ": ";
    for (size_t i = 0; i < it4prevmutex->second.size(); i++)
        std::cout << " " << it4prevmutex->second.at(i);
    std::cout << std::endl
              << "------" << std::endl;*/

    real_mutex_unlock(&mute_xx);

    result = real_mutex_unlock(mutex);
    return result;
}

/*
struct arg_t
{
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

void *thread0_routine(void *arg)
{
    arg_t *thread_arg = (arg_t *)arg;

    pthread_mutex_lock(&thread_arg->mutexes[0]);
    pthread_mutex_lock(&thread_arg->mutexes[1]);

    //	std::cout << "0 is alive" << std::endl;

    pthread_mutex_unlock(&thread_arg->mutexes[1]);
    pthread_mutex_unlock(&thread_arg->mutexes[0]);

    pthread_exit(0);
    return nullptr;
}

void *thread1_routine(void *arg)
{
    arg_t *thread_arg = (arg_t *)arg;

    pthread_mutex_lock(&thread_arg->mutexes[1]);
    pthread_mutex_lock(&thread_arg->mutexes[2]);

    //	std::cout << "1 is alive" << std::endl;

    pthread_mutex_unlock(&thread_arg->mutexes[2]);
    pthread_mutex_unlock(&thread_arg->mutexes[1]);

    pthread_exit(0);
    return nullptr;
}

void *thread2_routine(void *arg)
{
    arg_t *thread_arg = (arg_t *)arg;

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
    atexit(not_zero_return_code);
    pthread_create(&threads[0], NULL, thread0_routine, (void *)&thread_arg);
    pthread_create(&threads[1], NULL, thread1_routine, (void *)&thread_arg);
    //pthread_create(&threads[2], NULL, thread2_routine, (void *)&thread_arg);

    pthread_join(threads[0], NULL);
    pthread_create(&threads[0], NULL, thread0_routine, (void *)&thread_arg);
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    pthread_join(threads[2], NULL);
    return 0;
}
*/
