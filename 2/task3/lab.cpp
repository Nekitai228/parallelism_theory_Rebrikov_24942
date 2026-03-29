#include <iostream>
#include <omp.h>
#include <vector>
#include <cmath>
#include <chrono>
#include <iomanip>


const unsigned long N = 20000;
const double tau = 0.00005;
const double epsilon = 0.00001;
const long MAX_ITER = 10000; 

//Перечисления для типов расписания
enum class SchedType {STATIC, DYNAMIC, GUIDED, AUTO};


double l2_normalization(const std::vector<double> &b)
{
    double sum = 0.0;
    
    
    for(double val : b)
    {
        sum += val*val;
    }

    return sqrt(sum);
}


// A*xn - b
std::vector<double> compute_residual_serial(const std::vector<double> &A, const std::vector<double> &x, const std::vector<double> &b)
{
    std::vector<double> res(N);
    for(size_t i = 0; i < N; i++)
    {
        double dot = 0.0;
        for(size_t j = 0; j < N; j++)
        {
            dot += A[i * N + j] * x[j];
        }
        res[i] = dot - b[i];
    }

    return res;
}

std::vector<double> compute_residual_parallel(const std::vector<double> &A, const std::vector<double> &x, const std::vector<double> &b)
{
    std::vector<double> res(N);
    #pragma omp parallel for
    for(size_t i = 0; i < N; i++)
    {
        double dot = 0.0;
        for(size_t j = 0; j < N; j++)
        {
            dot += A[i * N + j] * x[j];
        }
        res[i] = dot - b[i];
    }

    return res;
}


// Обновление: x_new = x - tau * residual
void update_solution_serial(std::vector<double> &x, const std::vector<double>& A, const double& tau)
{
    for(size_t i = 0; i < N; i++)
    {
        x[i] = x[i] - tau * A[i];
    }
}

void update_solution_parallel(std::vector<double> &x, const std::vector<double>& A, const double& tau)
{
    #pragma omp parallel for
    for(size_t i = 0; i < N; i++)
    {
        x[i] = x[i] - tau * A[i];
    }
}


void simple_iteration_serial(std::vector<double> &A, std::vector<double> &b, std::vector<double> &x)
{
    const size_t MAX_ITER = 10000;  
    
    for(size_t iter = 0; iter < MAX_ITER; ++iter)
    {
        // Вычисляем невязку один раз
        std::vector<double> residual = compute_residual_serial(A, x, b);
        
        // Проверка сходимости: ||A*x_n - b||2  / ||b||2 < eps
        double rel_residual = l2_normalization(residual) / l2_normalization(b);
        if (rel_residual < epsilon) break;
        
        // Обновляем решение
        update_solution_serial(x, residual, tau);
    }
    
}


void simple_iteration_parallel_1(std::vector<double> &A, std::vector<double> &b, std::vector<double> &x)
{
    const size_t MAX_ITER = 10000;  
    
    for(size_t iter = 0; iter < MAX_ITER; ++iter)
    {
        // Вычисляем невязку один раз
        std::vector<double> residual = compute_residual_parallel(A, x, b);
        
        // Проверка сходимости: ||A*x_n - b||2  / ||b||2 < eps
        double rel_residual = l2_normalization(residual) / l2_normalization(b);
        if (rel_residual < epsilon) break;
        
        // Обновляем решение
        update_solution_parallel(x, residual, tau);
    }
    
}

// Параллельный запуск: Вариант 2 (единый parallel регион)
void simple_iteration_parallel_2(const std::vector<double> &A, 
                                 const std::vector<double> &b, 
                                 std::vector<double> &x,
                                 SchedType sched_type = SchedType::GUIDED, //тип расписания
                                 int chunk_size = 1) // Размер чанка для static/dynamic/guided 
{
    std::vector<double> residual(N);
    bool converged = false;
    
    //Применяем расписание через API
    omp_sched_t omp_sched;
    switch(sched_type)
    {
        case SchedType::STATIC: omp_sched = omp_sched_static; break;
        case SchedType::DYNAMIC: omp_sched = omp_sched_dynamic; break;
        case SchedType::GUIDED: omp_sched = omp_sched_guided; break;
        case SchedType::AUTO: omp_sched = omp_sched_auto; break;
    }
    omp_set_schedule(omp_sched, chunk_size); // Устанавливаем расписание
    

    #pragma omp parallel shared(A, b, x, residual, converged)
    {
        for(size_t iter = 0; iter < MAX_ITER && !converged; ++iter) 
        {
            // 1. Параллельное вычисление невязки прямо В residual
            #pragma omp for schedule(runtime)
            for(size_t i = 0; i < N; i++)
            {
                double dot = 0.0;
                for(size_t j = 0; j < N; j++)
                    dot += A[i * N + j] * x[j];
                residual[i] = dot - b[i];
            }
            
            // 2. Проверка сходимости (один поток)
            #pragma omp single
            {
                double rel_residual = l2_normalization(residual) / l2_normalization(b);
                if (rel_residual < epsilon)
                    converged = true;
            }

            // Неявный барьер в конце single
            
            if (converged) break;
            
            // 3. Параллельное обновление решения
            #pragma omp for schedule(runtime)
            for(size_t i = 0; i < N; i++)
            {
                x[i] -= tau * residual[i];
            }
        }
    }
}

int main(int argc, char** argv)
{
    std::vector<double> A(N*N);
    std::vector<double> b(N, N+1);
    std::vector<double> x(N, 0);


    std::vector<double> A_2(N*N);
    std::vector<double> b_2(N, N+1);
    std::vector<double> x_2(N, 0);


    for(size_t i = 0; i < N; i++)
    {
        for(size_t j = 0; j < N; j++)
        {
            A[i * N + j] = 1.0;
            A_2[i * N + j] = 1.0;
        }
    }
    for(size_t i = 0; i < N; i++)
    {
        A[i*N + i] = 2.0;
        A_2[i*N + i] = 2.0;
    }

    int num_threads = omp_get_max_threads();
    if(argc > 1)
    {
        num_threads = std::atoi(argv[1]);
    }

    omp_set_num_threads(num_threads);

    //double tparallel_1 = 0.0;
    //double tserial = 0.0;
    double tparallel_2 = 0.0;
    for(int i = 0; i < 100; i++)
    {
        // auto begin = std::chrono::steady_clock::now();
        // simple_iteration_serial(A,b,x);
        // auto end = std::chrono::steady_clock::now();
        // std::chrono::duration<double> elapsedTime = end - begin;
        
        // tserial += elapsedTime.count();
        
        // auto begin = std::chrono::steady_clock::now();
        // simple_iteration_parallel_1(A,b,x);
        // auto end = std::chrono::steady_clock::now();
        // std::chrono::duration<double> elapsedTime = end - begin;
        
        // tparallel_1 += elapsedTime.count();


        auto begin = std::chrono::steady_clock::now();
        simple_iteration_parallel_2(A_2,b_2,x_2);
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsedTime = end - begin;
        
        tparallel_2 += elapsedTime.count();
    }


    //std::cout << "Elapsed time (serial): " << std::fixed << tserial/10 << " sec.\n";

    //std::cout << "Elapsed time (parallel_1): " << std::fixed << tparallel_1/10 << " sec.\n";

    std::cout << "Elapsed time (parallel_2): " << std::fixed << tparallel_2/100 << " sec.\n";

    // std::cout << "Speedup_var1: " << std::fixed << std::setprecision(2) << (tserial / tparallel_1) << std::endl;

    // std::cout << "Speedup_var2: " << std::fixed << std::setprecision(2) << (tserial / tparallel_2) << std::endl;

    //std::cout << "it should have been a vector of ones, but it turned out to be: " << x[0] << std::endl;

    std::cout << "it should have been a vector of ones, but it turned out to be: " << x_2[0] << std::endl;

    return 0;
}
