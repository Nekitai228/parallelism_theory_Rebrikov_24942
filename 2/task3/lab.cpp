#include <iostream>
#include <omp.h>
#include <vector>
#include <cmath>
#include <chrono>
#include <iomanip>


const unsigned long N = 20000;
const double tau = 0.00005;
const double epsilon = 0.00001;


double l2_normalization(std::vector<double> &b)
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
void update_solution(std::vector<double> &x, const std::vector<double>& A, const double& tau)
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
        update_solution(x, residual, tau);
    }
    
}


void simple_iteration_parallel(std::vector<double> &A, std::vector<double> &b, std::vector<double> &x)
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
        update_solution(x, residual, tau);
    }
    
}


int main()
{
    std::vector<double> A(N*N);
    std::vector<double> b(N, N+1);
    std::vector<double> x(N, 0);


    for(size_t i = 0; i < N; i++)
    {
        for(size_t j = 0; j < N; j++)
        {
            A[i * N + j] = 1.0;
        }
    }
    for(size_t i = 0; i < N; i++)
    {
        A[i*N + i] = 2.0;
    }

    auto begin = std::chrono::steady_clock::now();
    simple_iteration_serial(A,b,x);
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsedTime = end - begin;
    std::cout << "Elapsed time (serial): " << std::fixed << elapsedTime.count() << " sec.\n";
    double tserial = elapsedTime.count();
    
    begin = std::chrono::steady_clock::now();
    simple_iteration_parallel(A,b,x);
    end = std::chrono::steady_clock::now();
    elapsedTime = end - begin;
    std::cout << "Elapsed time (parallel): " << std::fixed << elapsedTime.count() << " sec.\n";
    double tparallel = elapsedTime.count();

    std::cout << "Speedup: " << std::fixed << std::setprecision(2) << (tserial / tparallel) << std::endl;

    std::cout << "it should have been a vector of ones, but it turned out to be: " << x[0] << std::endl;

    return 0;
}
