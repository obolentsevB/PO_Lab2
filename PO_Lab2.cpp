#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <climits>

std::mutex mtx;

void locked_task(const std::vector<int>& data, int start, int end, long long& global_sum, int& global_min) {
    long long local_sum = 0;
    int local_min = INT_MAX;

    for (int i = start; i < end; ++i) {
        if (data[i] % 11 == 0) {
            local_sum += data[i];
            if (data[i] < local_min) local_min = data[i];
        }
    }

    // Блокуюча синхронізація для оновлення спільних ресурсів
    std::lock_guard<std::mutex> lock(mtx);
    global_sum += local_sum;
    if (local_min < global_min) global_min = local_min;
}

int main() {
    const int DATA_SIZE = 1000000;
    const int NUM_THREADS = 6;
    std::vector<int> data(DATA_SIZE);
    for (int i = 0; i < DATA_SIZE; ++i) data[i] = rand() % 1000;

    // 1. Послідовно
    long long seq_sum = 0;
    int seq_min = INT_MAX;
    auto start_seq = std::chrono::high_resolution_clock::now();
    for (int x : data) { if (x % 11 == 0) { seq_sum += x; if (x < seq_min) seq_min = x; } }
    auto end_seq = std::chrono::high_resolution_clock::now();

    // 2. З м'ютексом
    long long lock_sum = 0;
    int lock_min = INT_MAX;
    std::vector<std::thread> threads;
    auto start_lock = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(locked_task, std::ref(data), i * (DATA_SIZE / NUM_THREADS), (i + 1) * (DATA_SIZE / NUM_THREADS), std::ref(lock_sum), std::ref(lock_min));
    }
    for (auto& t : threads) t.join();
    auto end_lock = std::chrono::high_resolution_clock::now();

    std::cout << "Sequential: Sum=" << seq_sum << ", Min=" << seq_min << ", Time=" << std::chrono::duration<double>(end_seq - start_seq).count() << "s\n";
    std::cout << "Mutex: Sum=" << lock_sum << ", Min=" << lock_min << ", Time=" << std::chrono::duration<double>(end_lock - start_lock).count() << "s\n";

    return 0;
}
