
#pragma once

#include <string>
#include <fstream>
#include <random>
#include <vector>
#include <chrono>
#include <future>
#include <utility>

template<typename TaskType, typename ResultType>
class Client 
{
protected:
    std::string name;
    std::string output_file;
    size_t num_tasks;
    std::mt19937 rng;
    
public:
    Client(const std::string& client_name, size_t n_tasks, const std::string& filename)
        : name(client_name), num_tasks(n_tasks), output_file(filename) 
    {
        auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        rng.seed(seed + std::hash<std::string>{}(name));
    }

    virtual ~Client() = default;

    // Генерация случайной задачи
    virtual TaskType generate_task() = 0;
    
    // Форматирование результата
    virtual std::string format_result(const TaskType& task, const ResultType& result) = 0;

    // Добавление задач и ожидание результатов
    template<typename ServerType>
    void add_tasks_and_wait(ServerType& server, std::vector<std::pair<TaskType, ResultType>>& out_results) {
        std::vector<std::future<ResultType>> futures;
        std::vector<TaskType> tasks;
        
        futures.reserve(num_tasks);
        tasks.reserve(num_tasks);
        
        // Добавляем задачи и сохраняем futures
        for (size_t i = 0; i < num_tasks; ++i) 
        {
            TaskType task = generate_task();
            tasks.push_back(task);
            futures.push_back(server.add_task(task));
        }
        
        // Ждём результаты из futures
        out_results.clear();
        out_results.reserve(num_tasks);
        
        for (size_t i = 0; i < num_tasks; ++i) 
        {
            try 
            {
                // future.get() можно вызвать только один раз!
                ResultType result = futures[i].get();
                out_results.emplace_back(tasks[i], result);
            } 
            catch (const std::exception& e) 
            {
                std::cerr << name << ": Task " << i << " failed: " << e.what() << "\n";
            }
        }
    }

    // Сохранение результатов в файл
    void save_to_file(const std::vector<std::pair<TaskType, ResultType>>& results) 
    {
        std::ofstream file(output_file);
        if (!file.is_open()) 
        {
            throw std::runtime_error("Cannot open file: " + output_file);
        }
        
        file << "# Client: " << name << "\n";
        file << "# Tasks: " << num_tasks << "\n";
        file << "# Format: task_input | result\n\n";
        
        for (const auto& [task, result] : results) 
        {
            file << format_result(task, result) << "\n";
        }
        
        file.close();
        std::cout << name << ": saved " << results.size() 
                  << " results to " << output_file << "\n";
    }

    const std::string& get_name() const { return name; }
    size_t get_num_tasks() const { return num_tasks; }
};