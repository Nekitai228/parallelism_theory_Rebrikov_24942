#include "heat_solver.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <iomanip>

HeatSolver::HeatSolver(int nx, int ny) : nx(nx), ny(ny) {
    A.resize(nx * ny, 0.0);
}

double HeatSolver::lerp(double a, double b, double t) const {
    return a + t * (b - a);
}

void HeatSolver::initialize() {
    std::fill(A.begin(), A.end(), 0.0);

    // Граничные условия
    for (int j = 0; j < ny; j++) A[idx(0, j)] = lerp(10.0, 20.0, static_cast<double>(j)/(ny-1));
    for (int i = 0; i < nx; i++) A[idx(i, ny-1)] = lerp(20.0, 30.0, static_cast<double>(i)/(nx-1));
    for (int j = 0; j < ny; j++) A[idx(nx-1, j)] = lerp(20.0, 30.0, static_cast<double>(j)/(ny-1));
    for (int i = 0; i < nx; i++) A[idx(i, 0)] = lerp(10.0, 20.0, static_cast<double>(i)/(nx-1));

    // Перенос на GPU
    double* pA = A.data();
    #pragma acc enter data copyin(pA[:nx*ny])
}

// === Red-Black SOR Ядро ===
// omega (релаксация) вычисляется один раз для оптимальной сходимости
// omega = 2 / (1 + sin(pi / N))
double HeatSolver::compute_kernel(int nx, int ny, double* __restrict__ A) {
    double error = 0.0;
    
    // Оптимальный параметр релаксации для квадратной сетки
    double omega = 2.0 / (1.0 + std::sin(M_PI / nx));

    // === КРАСНЫЙ ПРОХОД ===
    #pragma acc parallel loop present(A[:nx*ny]) reduction(max:error)
    for (int i = 1; i < nx - 1; i++) {
        int j_start = 1 + (i % 2);
        for (int j = j_start; j < ny - 1; j += 2) {
            int idx = i * ny + j;
            
            // Стандартное среднее (пятиточечный шаблон)
            double avg = 0.25 * (A[idx - ny] + A[idx + ny] + A[idx - 1] + A[idx + 1]);
            
            // Формула SOR: новое = старое + omega * (среднее - старое)
            double new_val = A[idx] + omega * (avg - A[idx]);
            
            double diff = new_val - A[idx];
            if (diff < 0.0) diff = -diff;
            if (diff > error) error = diff;
            
            A[idx] = new_val;
        }
    }

    // === ЧЕРНЫЙ ПРОХОД ===
    #pragma acc parallel loop present(A[:nx*ny]) reduction(max:error)
    for (int i = 1; i < nx - 1; i++) {
        int j_start = 2 - (i % 2);
        for (int j = j_start; j < ny - 1; j += 2) {
            int idx = i * ny + j;
            
            double avg = 0.25 * (A[idx - ny] + A[idx + ny] + A[idx - 1] + A[idx + 1]);
            double new_val = A[idx] + omega * (avg - A[idx]);
            
            double diff = new_val - A[idx];
            if (diff < 0.0) diff = -diff;
            if (diff > error) error = diff;
            
            A[idx] = new_val;
        }
    }

    return error;
}

void HeatSolver::printGrid(int rows, int cols) const {
    const double* ptr_A = A.data();
    #pragma acc update self(ptr_A[0:nx*ny])

    int step_i = std::max(1, nx / (rows - 1));
    int step_j = std::max(1, ny / (cols - 1));

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "\nGrid (" << rows << "x" << cols << " sample from " 
              << nx << "x" << ny << "):\n";

    for (int i = 0; i < nx; i += step_i) {
        for (int j = 0; j < ny; j += step_j) {
            std::cout << std::setw(8) << A[idx(i, j)] << " ";
        }
        std::cout << "\n";
    }
    std::cout << std::endl;
}