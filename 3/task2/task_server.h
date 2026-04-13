
#pragma once

#include <iostream>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <unordered_map>
#include <atomic>
#include <memory>

template<typename TaskType, typename ResultType>
class TaskServer 
{
private:
    // Структура задачи с promise для результата
    struct Task 
    {
        size_t id;
        TaskType data;
        std::promise<ResultType> promise;
    };

    std::queue<std::unique_ptr<Task>> task_queue;
    std::thread server_thread;
    
    std::mutex queue_mutex;
    std::condition_variable cv;
    
    std::atomic<bool> running{false};
    std::atomic<size_t> next_task_id{0};

    // Функция-обработчик задач
    std::function<ResultType(const TaskType&)> task_processor;

public:
    explicit TaskServer(std::function<ResultType(const TaskType&)> processor)
        : task_processor(processor) {}

    ~TaskServer()
    {
        stop();
    }

    // Запуск сервера
    void start() {
        if (running.exchange(true)) 
        {
            return;
        }
        
        server_thread = std::thread([this]() 
        {
            while (running.load()) 
            {
                std::unique_ptr<Task> task;
                
                {
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    cv.wait(lock, [this]() 
                    {
                        return !task_queue.empty() || !running.load();
                    });
                    
                    if (!running.load() && task_queue.empty()) 
                    {
                        break;
                    }
                    
                    if (!task_queue.empty()) 
                    {
                        task = std::move(task_queue.front());
                        task_queue.pop();
                    }
                }
                
                if (task) 
                {
                    try 
                    {
                        // Выполняем задачу и устанавливаем результат в promise
                        ResultType result = task_processor(task->data);
                        task->promise.set_value(result);
                    } 
                    catch (...) 
                    {
                        // При ошибке устанавливаем исключение
                        task->promise.set_exception(std::current_exception());
                    }
                }
            }
        });
    }

    // Остановка сервера
    void stop() {
        if (!running.exchange(false)) 
        {
            return;
        }
        
        cv.notify_all();
        
        if (server_thread.joinable()) 
        {
            server_thread.join();
        }
    }

    // Добавление задачи - возвращает future для получения результата
    std::future<ResultType> add_task(const TaskType& task_data) 
    {
        auto task = std::make_unique<Task>();
        task->id = next_task_id++;
        task->data = task_data;
        
        // Получаем future ДО того, как положили задачу в очередь
        std::future<ResultType> future = task->promise.get_future();
        
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            task_queue.push(std::move(task));
        }
        
        cv.notify_one();
        return future;  // Возвращаем future клиенту
    }

    // Неблокирующая проверка готовности
    bool is_running() const 
    {
        return running.load();
    }
};