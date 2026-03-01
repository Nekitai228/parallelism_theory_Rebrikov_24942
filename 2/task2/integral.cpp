
#include <cmath>
#include <omp.h>
#include <iostream>
#include <iomanip>
#include <chrono>


const double PI = 3.14159265358979323846;
const double a = -4.0;
const double b = 4.0;
const int nsteps = 40000000;

// double cpuSecond()
// {
//     struct timespec ts;
//     timespec_get(&ts, TIME_UTC);
//     return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
// }

double func(double x)
{
    return exp(-x * x);
}

double integrate(double (*func)(double), double a, double b, int n)
{
    double h = (b - a) / n;
    double sum = 0.0;

    for (int i = 0; i < n; i++)
        sum += func(a + h * (i + 0.5));

    sum *= h;

    return sum;
}

double integrate_omp(double (*func)(double), double a, double b, int n)
{
    double h = (b - a) / n;
    double sum = 0.0;

#pragma omp parallel
    {
        int nthreads = omp_get_num_threads();
        int threadid = omp_get_thread_num();
        int items_per_thread = n / nthreads;
        int lb = threadid * items_per_thread;
        int ub = (threadid == nthreads - 1) ? (n - 1) : (lb + items_per_thread - 1);
        double sumloc = 0.0;
        for (int i = lb; i <= ub; i++)
            sumloc += func(a + h * (i + 0.5));

        #pragma omp atomic
        sum += sumloc;
    }
    sum *= h;

    return sum;
}

double run_serial()
{

    auto begin = std::chrono::steady_clock::now();
    double res = integrate(func, a, b, nsteps);
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsedTime = end - begin;
    
    std::cout << "Result (serial): " << std::fixed << std::setprecision(12) << res << " res; error " << std::setprecision(12) << fabs(res - sqrt(PI)) << std::endl;
    // printf("Result (serial): %.12f; error %.12f\n", res, fabs(res - sqrt(PI)));
    return elapsedTime.count();
}
double run_parallel()
{
    
    auto begin = std::chrono::steady_clock::now();
    double res = integrate_omp(func, a, b, nsteps);
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsedTime = end - begin;
    std::cout << "Result (parallel): " << std::fixed << std::setprecision(12) << res << " res; error " << std::setprecision(12) << fabs(res - sqrt(PI)) << std::endl;

    //printf("Result (parallel): %.12f; error %.12f\n", res, fabs(res - sqrt(PI)));
    return elapsedTime.count();
}
int main(int argc, char **argv)
{
    std::cout << "Integration f(x) on [" << std::setprecision(12) << a << ", " << b << "], nsteps = " << nsteps << std::endl;
    // printf("Integration f(x) on [%.12f, %.12f], nsteps = %d\n", a, b, nsteps);
    double tserial = run_serial();
    double tparallel = run_parallel();

    std::cout << "Execution time (serial): " << std::fixed << std::setprecision(6) << tserial << std::endl;
   
    std::cout << "Execution time (parallel): " << std::fixed << std::setprecision(6) << tparallel << std::endl;

    std::cout << "Speedup: " << std::fixed << std::setprecision(2) << (tserial / tparallel) << std::endl;
    return 0;
}