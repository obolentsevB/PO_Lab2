#include <iostream>
#include <vector>
#include <chrono>
#include <climits>

int main() {
    const int DATA_SIZE = 1000000;
    std::vector<int> data(DATA_SIZE);
    for (int i = 0; i < DATA_SIZE; ++i) data[i] = rand() % 1000;

    long long sum = 0;
    int min_val = INT_MAX;

    auto start = std::chrono::high_resolution_clock::now();
    
    for (int x : data) {
        if (x % 11 == 0) {
            sum += x;
            if (x < min_val) min_val = x;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Sequential: Sum=" << sum << ", Min=" << min_val 
              << ", Time=" << std::chrono::duration<double>(end - start).count() << "s\n";

    return 0;
}
