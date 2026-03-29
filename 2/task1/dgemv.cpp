#include <iostream>
#include <cstdlib>
#include <omp.h>
#include <vector>
#include <iomanip>
#include <chrono>


// double cpuSecond()
// {
//     struct timespec ts;
//     timespec_get(&ts, TIME_UTC);
//     return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
// }

/*
 * matrix_vector_product: Compute matrix-vector product c[m] = a[m][n] * b[n]
 */
void matrix_vector_product(std::vector<double> &a, std::vector<double> &b, std::vector<double> &c, size_t m, size_t n)
{
    for (size_t i = 0; i < m; i++)
    {
        c[i] = 0.0;
        for (int j = 0; j < n; j++)
            c[i] += a[i * n + j] * b[j];
    }
}

// void matrix_vector_product_omp(double *a, double *b, double *c, size_t m, size_t n)
// {
// #pragma omp parallel
//     {
//         double t = omp_get_wtime();
//         int nthreads = omp_get_num_threads();
//         int threadid = omp_get_thread_num();
//         int items_per_thread = m / nthreads;
//         int lb = threadid * items_per_thread;
//         int ub = (threadid == nthreads - 1) ? (m - 1) : (lb + items_per_thread - 1);
//         for (int i = lb; i <= ub; i++)
//         {
//             c[i] = 0.0;
//             for (int j = 0; j < n; j++)
//                 c[i] += a[i * n + j] * b[j];
//         }
//         t = omp_get_wtime() - t;
//         printf("Thread %d items %d [%d - %d], time: %.6f\n", threadid, ub - lb + 1, lb, ub, t);
//     }
// }

/*
    matrix_vector_product_omp: Compute matrix-vector product c[m] = a[m][n] * b[n]
*/
void matrix_vector_product_omp(std::vector<double>& a, std::vector<double>& b, std::vector<double> &c, size_t m, size_t n)
{
#pragma omp parallel
    {
        int nthreads = omp_get_num_threads();
        int threadid = omp_get_thread_num();
        int items_per_thread = m / nthreads;
        int lb = threadid * items_per_thread;
        int ub = (threadid == nthreads - 1) ? (m - 1) : (lb + items_per_thread - 1);
        for (size_t i = lb; i <= ub; i++)
        {
            c[i] = 0.0;
            for (size_t j = 0; j < n; j++)
                c[i] += a[i * n + j] * b[j];
        }
    }
}

double run_serial(size_t n, size_t m)
{
    std::vector<double> a(m*n);
    std::vector<double> b(n);
    std::vector<double> c(m);
    

    for (size_t i = 0; i < m; i++)
    {
        for (size_t j = 0; j < n; j++)
            a[i * n + j] = i + j;
    }

    for (size_t j = 0; j < n; j++)
        b[j] = j;

    auto begin = std::chrono::steady_clock::now(); 
    matrix_vector_product(a, b, c, m, n);
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsedTime = end - begin; 
    //std::cout << "Elapsed time (serial): " << std::setprecision(9) << elapsedTime.count() << " sec.\n";
    // printf("Elapsed time (serial): %.6f sec.\n", t);
    return elapsedTime.count();
}

double run_parallel(size_t n, size_t m)
{
    std::vector<double> a(m*n);
    std::vector<double> b(n);
    std::vector<double> c(m);

    for (size_t i = 0; i < m; i++)
    {
        for (size_t j = 0; j < n; j++)
            a[i * n + j] = i + j;
    }

    for (size_t j = 0; j < n; j++)
        b[j] = j;
    

    auto begin = std::chrono::steady_clock::now();
    matrix_vector_product_omp(a, b, c, m, n);
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsedTime = end - begin; 
    //std::cout << "Elapsed time (parallel): " << std::setprecision(9) << elapsedTime.count() << " sec.\n";
    // printf("Elapsed time (parallel): %.6f sec.\n", t);
    return elapsedTime.count();
}

int main(int argc, char *argv[])
{
    //double tserial = 0.0;
    double tparallel = 0.0;
    int num_threads = omp_get_max_threads();
    size_t M = 1000;
    size_t N = 1000;
    if (argc > 1)
        M = atoi(argv[1]);
    if (argc > 2)
        N = atoi(argv[2]);
    if (argc > 3)
        num_threads = std::atoi(argv[3]);
    omp_set_num_threads(num_threads);
    for(int i = 0; i < 50; i++)
    {
        //tserial += run_serial(M, N);
        tparallel += run_parallel(M, N);
    }
    //std::cout << "Elapsed time (serial): " << std::setprecision(9) << (tserial/50) << " sec.\n";
    std::cout << "Elapsed time (parallel): " << std::setprecision(9) << (tparallel/50) << " sec.\n";

    return 0;
}