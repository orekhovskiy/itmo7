class DiningPhilosophers {
public:
    DiningPhilosophers() {
        
    }

    std::array<std::mutex, 5> forkMutexes;
    
    void wantsToEat(int philosopher,
                    function<void()> pickLeftFork,
                    function<void()> pickRightFork,
                    function<void()> eat,
                    function<void()> putLeftFork,
                    function<void()> putRightFork) {
        
        forkMutexes[std::min(philosopher, (philosopher + 4) % 5)].lock();
        forkMutexes[std::max(philosopher, (philosopher + 4) % 5)].lock();
        
        pickLeftFork();
        pickRightFork();
        eat();
        putLeftFork();
        putRightFork();
        
        forkMutexes[philosopher].unlock();
        forkMutexes[(philosopher + 4) % 5].unlock();
		
    }
};