
#include "task_server.h"
#include "tasks.h"
// #include "validator.h"
#include <iostream>
#include <chrono>
#include <iomanip>

struct BenchmarkStats 
{
    size_t num_tasks;
    double total_time;
    double tasks_per_second;
};

BenchmarkStats run_benchmark(size_t tasks_per_client) 
{
    BenchmarkStats stats;
    stats.num_tasks = tasks_per_client;
    
    auto total_start = std::chrono::high_resolution_clock::now();
    
    // Создаём сервера
    auto sin_server = std::make_unique<TaskServer<double, double>>(
        [](const double& x) { return std::sin(x); }
    );
    
    auto sqrt_server = std::make_unique<TaskServer<double, double>>(
        [](const double& x) { return std::sqrt(x); }
    );
    
    auto pow_server = std::make_unique<TaskServer<std::pair<double, double>, double>>(
        [](const std::pair<double, double>& p) 
        { 
            return std::pow(p.first, p.second); 
        }
    );
    
    // Запускаем сервера
    sin_server->start();
    sqrt_server->start();
    pow_server->start();
    
    // Создаём клиентов
    SinClient sin_client(tasks_per_client, "sin_results.txt");
    SqrtClient sqrt_client(tasks_per_client, "sqrt_results.txt");
    PowClient pow_client(tasks_per_client, "pow_results.txt");
    
    // Результаты
    std::vector<std::pair<double, double>> sin_results;
    std::vector<std::pair<double, double>> sqrt_results;
    std::vector<std::pair<std::pair<double, double>, double>> pow_results;
    
    // Запускаем клиентов в потоках
    std::thread sin_thread([&]() 
    {
        sin_client.add_tasks_and_wait(*sin_server, sin_results);
        sin_client.save_to_file(sin_results);
    });
    
    std::thread sqrt_thread([&]() 
    {
        sqrt_client.add_tasks_and_wait(*sqrt_server, sqrt_results);
        sqrt_client.save_to_file(sqrt_results);
    });
    
    std::thread pow_thread([&]() 
    {
        pow_client.add_tasks_and_wait(*pow_server, pow_results);
        pow_client.save_to_file(pow_results);
    });
    
    // Ждём завершения всех клиентов
    sin_thread.join();
    sqrt_thread.join();
    pow_thread.join();
    
    // Останавливаем сервера
    sin_server->stop();
    sqrt_server->stop();
    pow_server->stop();
    
    auto total_end = std::chrono::high_resolution_clock::now();
    stats.total_time = std::chrono::duration<double>(total_end - total_start).count();
    
    size_t total_tasks = tasks_per_client * 3;
    stats.tasks_per_second = total_tasks / stats.total_time;
    
    return stats;
}

void print_stats(const BenchmarkStats& stats) 
{
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Tasks per client: " << stats.num_tasks << "\n";
    std::cout << "Total tasks: " << (stats.num_tasks * 3) << "\n";
    std::cout << std::string(60, '=') << "\n";
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Total time:            " << stats.total_time << " sec\n";
    std::cout << std::setprecision(2);
    std::cout << "Throughput:            " << stats.tasks_per_second << " tasks/sec\n";
    std::cout << std::string(60, '=') << "\n";
}






int main(int argc, char* argv[]) 
{
    std::cout << "=== Client-Server with std::promise/std::future ===\n\n";
    
    size_t tasks = (argc > 1) ? std::stoul(argv[1]) : 100;
    
    std::cout << "Running with " << tasks << " tasks per client...\n";
    
    BenchmarkStats stats = run_benchmark(tasks);
    print_stats(stats);

    //test_results();
    
    std::cout << "\n=== Completed successfully! ===\n";
    
    return 0;
}