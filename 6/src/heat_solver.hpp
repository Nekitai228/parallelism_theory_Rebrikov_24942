#pragma once
#include <vector>

class HeatSolver {
public:
    HeatSolver(int nx, int ny);
    void initialize();
    
    // Быстрое ядро: только вычисления, без reduction
    static void compute_kernel_fast(int nx, int ny, double* __restrict__ A, double* __restrict__ Anew);
    
    // Ядро с проверкой ошибки: с reduction
    static double compute_kernel_with_error(int nx, int ny, double* __restrict__ A, double* __restrict__ Anew);
    
    void printGrid(int rows = 10, int cols = 10) const;

    int getNx() const { return nx; }
    int getNy() const { return ny; }
    double* getPtrA() { return A.data(); }
    double* getPtrAnew() { return Anew.data(); }

private:
    int nx, ny;
    std::vector<double> A;
    std::vector<double> Anew;
    
    double lerp(double a, double b, double t) const;
    inline int idx(int i, int j) const { return i * ny + j; }
};