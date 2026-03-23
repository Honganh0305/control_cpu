#include <algorithm>
#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
using namespace std;

// this class is used to measure the time taken by the simulation
class Timer {
  public:
    void start() const { start_time = Clock::now(); }
    double stop() const {
        auto end_time = Clock::now();
        return std::chrono::duration<double>(end_time - start_time).count();
    }

  private:
    using Clock = chrono::high_resolution_clock;
    mutable std::chrono::time_point<Clock> start_time;
};


double run_simulation(int cores_to_use = 1) {
    Timer t;
    // start the timer
    t.start();

    vector<thread> threads; // create a vector of threads
    double sum = 0.0;
    mutex sum_mutex; // create a mutex to make sure that only one thread can access to the sum at a time
    int total_iterations = 5000000;

    auto worker = [&](int start, int end) // used to calculate the sum of a numbers in range [start,end]
    {
        double local_sum = 0.0; // each thread will have its own local sum, and then we will add these local sums to sum
        for (int i = start; i < end; ++i) {
            local_sum += i;
        }
        lock_guard<mutex> lock(sum_mutex); // lock the mutex
        sum += local_sum; // add the local sum to the sum
    };

    int chunk = total_iterations / cores_to_use; // calculate the chunk size, in other words, the number of iterations each core/thread will handle
    // distribute the work to the cores/threads
    for (int i = 0; i < cores_to_use; ++i) 
    {
        int start = i * chunk; // calculate the start of the range for the current thread/cores
        int end = (i == cores_to_use - 1) ? total_iterations : start + chunk; // calculate the end of the range for the current threads/cores
        threads.emplace_back(worker, start, end); //push the thread into the vector of threads
    }

    for (auto &th : threads) th.join(); // wait for all threads to finish their work

    double elapsed = t.stop(); // stop the timer
    return elapsed; // return the time taken by the simulation
}

int main() {
    unsigned int total_cores = std::thread::hardware_concurrency(); // get the total number of cpu cores in the machine
    cout << "Detected CPU cores: " << total_cores << "\n"; 

    for (int cores_to_use = 1; cores_to_use <= (int)total_cores; cores_to_use *= 2)  // test the simulation with different number of cores
    {
        double time_taken = run_simulation(cores_to_use);
        cout << "Using " << cores_to_use << " cores, elapsed time: " << time_taken << "s\n";
    }

    return 0;
}