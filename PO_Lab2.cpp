#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <climits>
#include <iomanip>

const int NUM_THREADS = 6;
const int DATA_SIZE = 10000000;

void sequential_task(const std::vector<int>& data, long long& sum, int& min_val) {
    sum = 0;
    min_val = INT_MAX;
    for (int x : data) {
        if (x % 11 == 0) {
            sum += x;
            if (x < min_val) min_val = x;
        }
    }
}

std::mutex mtx;
void locked_task_in_loop(const std::vector<int>& data, int start, int end, long long& sum, int& min_val) {
    for (int i = start; i < end; ++i) {
        if (data[i] % 11 == 0) {
            std::lock_guard<std::mutex> lock(mtx); 
            sum += data[i];
            if (data[i] < min_val) min_val = data[i];
        }
    }
}

void atomic_cas_task(const std::vector<int>& data, int start, int end, std::atomic<long long>& sum, std::atomic<int>& min_val) {
    for (int i = start; i < end; ++i) {
        if (data[i] % 11 == 0) {
            int val = data[i];
            long long current_sum = sum.load();
            while (!sum.compare_exchange_weak(current_sum, current_sum + val)) {} 

            int current_min = min_val.load();
            while (val < current_min && !min_val.compare_exchange_weak(current_min, val)) {} 
        }
    }
}

int main() {
    std::vector<int> data(DATA_SIZE);
    for (int i = 0; i < DATA_SIZE; ++i) data[i] = rand() % 1000;

    long long seq_sum; int seq_min;
    auto s_start = std::chrono::high_resolution_clock::now();
    sequential_task(data, seq_sum, seq_min);
    auto s_end = std::chrono::high_resolution_clock::now();
    double t_seq = std::chrono::duration<double>(s_end - s_start).count();

    long long lock_sum = 0; int lock_min = INT_MAX;
    std::vector<std::thread> threads;
    int chunk = DATA_SIZE / NUM_THREADS;
    auto m_start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_THREADS; ++i) {
        int start = i * chunk;
        int end = (i == NUM_THREADS - 1) ? DATA_SIZE : (i + 1) * chunk;
        threads.emplace_back(locked_task_in_loop, std::ref(data), start, end, std::ref(lock_sum), std::ref(lock_min));
    }
    for (auto& t : threads) t.join();
    auto m_end = std::chrono::high_resolution_clock::now();
    double t_lock = std::chrono::duration<double>(m_end - m_start).count();

    std::atomic<long long> atom_sum(0);
    std::atomic<int> atom_min(INT_MAX);
    threads.clear();
    auto a_start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_THREADS; ++i) {
        int start = i * chunk;
        int end = (i == NUM_THREADS - 1) ? DATA_SIZE : (i + 1) * chunk;
        threads.emplace_back(atomic_cas_task, std::ref(data), start, end, std::ref(atom_sum), std::ref(atom_min));
    }
    for (auto& t : threads) t.join();
    auto a_end = std::chrono::high_resolution_clock::now();
    double t_atom = std::chrono::duration<double>(a_end - a_start).count();

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Sequential: Time=" << t_seq << "s\n";
    std::cout << "Mutex (In Loop): Acceleration=" << (t_seq / t_lock) << "x (Time: " << t_lock << "s)\n";
    std::cout << "Atomic CAS: Acceleration=" << (t_seq / t_atom) << "x (Time: " << t_atom << "s)\n";

    return 0;
}
