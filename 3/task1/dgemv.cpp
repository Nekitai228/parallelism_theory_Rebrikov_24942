#include <iostream>
#include <cstdlib>
#include <vector>
#include <iomanip>
#include <chrono>
#include <thread>
#include <atomic>
#include <functional>

// СЕРИАЛЬНАЯ ВЕРСИЯ
void matrix_vector_product_serial(const std::vector<double> &a, 
                                   const std::vector<double> &b, 
                                   std::vector<double> &c, 
                                   size_t m, size_t n)
{
    for (size_t i = 0; i < m; i++)
    {
        c[i] = 0.0;
        for (size_t j = 0; j < n; j++)
            c[i] += a[i * n + j] * b[j];
    }
}

//  ПАРАЛЛЕЛЬНАЯ ИНИЦИАЛИЗАЦИЯ
void parallel_init_matrix(std::vector<double> &a, size_t m, size_t n, size_t num_threads)
{
    std::vector<std::thread> threads;
    threads.reserve(num_threads);
    
    size_t rows_per_thread = m / num_threads;
    
    for (size_t t = 0; t < num_threads; t++)
    {
        size_t start_row = t * rows_per_thread;
        size_t end_row = (t == num_threads - 1) ? m : start_row + rows_per_thread;
        
        threads.emplace_back([&a, n, start_row, end_row]() {
            for (size_t i = start_row; i < end_row; i++)
            {
                for (size_t j = 0; j < n; j++)
                    a[i * n + j] = static_cast<double>(i + j);
            }
        });
    }
    
    for (auto &thread : threads)
        thread.join();
}

void parallel_init_vector(std::vector<double> &b, size_t n, size_t num_threads)
{
    std::vector<std::thread> threads;
    threads.reserve(num_threads);
    
    size_t items_per_thread = n / num_threads;
    
    for (size_t t = 0; t < num_threads; t++)
    {
        size_t start = t * items_per_thread;
        size_t end = (t == num_threads - 1) ? n : start + items_per_thread;
        
        threads.emplace_back([&b, start, end]() {
            for (size_t j = start; j < end; j++)
                b[j] = static_cast<double>(j);
        });
    }
    
    for (auto &thread : threads)
        thread.join();
}

// ВЕРСИЯ С std::thread 
void matrix_vector_product_thread(const std::vector<double> &a, 
                                   const std::vector<double> &b, 
                                   std::vector<double> &c, 
                                   size_t m, size_t n, 
                                   size_t num_threads)
{
    std::vector<std::thread> threads;
    threads.reserve(num_threads);
    
    size_t rows_per_thread = m / num_threads;
    
    for (size_t t = 0; t < num_threads; t++)
    {
        size_t start_row = t * rows_per_thread;
        size_t end_row = (t == num_threads - 1) ? m : start_row + rows_per_thread;
        
        threads.emplace_back([&a, &b, &c, n, start_row, end_row]() {
            for (size_t i = start_row; i < end_row; i++)
            {
                c[i] = 0.0;
                for (size_t j = 0; j < n; j++)
                    c[i] += a[i * n + j] * b[j];
            }
        });
    }
    
    for (auto &thread : threads)
        thread.join();
}

//ВЕРСИЯ С std::jthread (C++20) 
void matrix_vector_product_jthread(const std::vector<double> &a, 
                                    const std::vector<double> &b, 
                                    std::vector<double> &c, 
                                    size_t m, size_t n, 
                                    size_t num_threads)
{
    std::vector<std::jthread> threads;
    threads.reserve(num_threads);
    
    size_t rows_per_thread = m / num_threads;
    
    for (size_t t = 0; t < num_threads; t++)
    {
        size_t start_row = t * rows_per_thread;
        size_t end_row = (t == num_threads - 1) ? m : start_row + rows_per_thread;
        
        threads.emplace_back([&a, &b, &c, n, start_row, end_row](std::stop_token stoken) {
            for (size_t i = start_row; i < end_row; i++)
            {
                if (stoken.stop_requested()) break;
                c[i] = 0.0;
                for (size_t j = 0; j < n; j++)
                    c[i] += a[i * n + j] * b[j];
            }
        });
    }
    // jthread автоматически вызывает join() в деструкторе
}

// ЗАМЕРЫ ВРЕМЕНИ 
double run_serial(size_t n, size_t m)
{
    std::vector<double> a(m * n);
    std::vector<double> b(n);
    std::vector<double> c(m);

    // Серийная инициализация
    for (size_t i = 0; i < m; i++)
        for (size_t j = 0; j < n; j++)
            a[i * n + j] = static_cast<double>(i + j);
    
    for (size_t j = 0; j < n; j++)
        b[j] = static_cast<double>(j);

    auto begin = std::chrono::steady_clock::now();
    matrix_vector_product_serial(a, b, c, m, n);
    auto end = std::chrono::steady_clock::now();
    
    return std::chrono::duration<double>(end - begin).count();
}

double run_parallel_thread(size_t n, size_t m, size_t num_threads, bool parallel_init = false)
{
    std::vector<double> a(m * n);
    std::vector<double> b(n);
    std::vector<double> c(m);

    auto begin_init = std::chrono::steady_clock::now();
    
    if (parallel_init)
    {
        parallel_init_matrix(a, m, n, num_threads);
        parallel_init_vector(b, n, num_threads);
    }
    else
    {
        for (size_t i = 0; i < m; i++)
            for (size_t j = 0; j < n; j++)
                a[i * n + j] = static_cast<double>(i + j);
        for (size_t j = 0; j < n; j++)
            b[j] = static_cast<double>(j);
    }
    
    auto end_init = std::chrono::steady_clock::now();
    
    auto begin = std::chrono::steady_clock::now();
    matrix_vector_product_thread(a, b, c, m, n, num_threads);
    auto end = std::chrono::steady_clock::now();
    
    return std::chrono::duration<double>(end - begin).count();
}

double run_parallel_jthread(size_t n, size_t m, size_t num_threads)
{
    std::vector<double> a(m * n);
    std::vector<double> b(n);
    std::vector<double> c(m);

    // Инициализация
    for (size_t i = 0; i < m; i++)
        for (size_t j = 0; j < n; j++)
            a[i * n + j] = static_cast<double>(i + j);
    for (size_t j = 0; j < n; j++)
        b[j] = static_cast<double>(j);

    auto begin = std::chrono::steady_clock::now();
    matrix_vector_product_jthread(a, b, c, m, n, num_threads);
    auto end = std::chrono::steady_clock::now();
    
    return std::chrono::duration<double>(end - begin).count();
}

int main(int argc, char *argv[])
{
    size_t M = 1000;
    size_t N = 1000;
    size_t num_threads = std::thread::hardware_concurrency();
    int num_runs = 100;
    
    if (argc > 1) M = std::stoul(argv[1]);
    if (argc > 2) N = std::stoul(argv[2]);
    if (argc > 3) num_runs = std::stoi(argv[3]);
    
    std::cout << "Matrix size: " << M << "x" << N << "\n";
    std::cout << "Number of runs: " << num_runs << "\n\n";
    
    // Замер серийного времени
    std::cout << "Running serial version...\n";
    double tserial_total = 0.0;
    for (int i = 0; i < num_runs; i++)
        tserial_total += run_serial(N, M);
    double tserial_avg = tserial_total / num_runs;
    std::cout << "Serial time: " << std::setprecision(9) << tserial_avg << " sec\n\n";
    
    // Таблица результатов
    std::cout << "=============================================================\n";
    std::cout << std::setw(8) << "Threads" << std::setw(15) << "Time (sec)" 
              << std::setw(15) << "Speedup" << std::setw(15) << "Efficiency" << "\n";
    std::cout << "=============================================================\n";
    
    std::vector<size_t> thread_counts = {16, 20};
    
    for (size_t p : thread_counts)
    {
        if (p > std::thread::hardware_concurrency()) break;
        
        double tparallel_total = 0.0;
        for (int i = 0; i < num_runs; i++)
            tparallel_total += run_parallel_thread(N, M, p, true); // true = параллельная инициализация
        
        double tparallel_avg = tparallel_total / num_runs;
        double speedup = tserial_avg / tparallel_avg;
        double efficiency = speedup / p * 100.0;
        
        std::cout << std::setw(8) << p 
                  << std::setw(15) << std::setprecision(6) << tparallel_avg
                  << std::setw(15) << std::setprecision(4) << speedup
                  << std::setw(14) << std::setprecision(2) << efficiency << "%\n";
    }
    std::cout << "=============================================================\n";
    
    return 0;
}