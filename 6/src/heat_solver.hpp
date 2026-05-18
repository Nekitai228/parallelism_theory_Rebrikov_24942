#pragma once
#include <vector>

class HeatSolver {
public:
    HeatSolver(int nx, int ny);
    void initialize();
    
    // Статическое ядро принимает массив (A)
    static double compute_kernel(int nx, int ny, double* __restrict__ A);
    
    void printGrid(int rows = 10, int cols = 10) const;

    // Геттеры для доступа к размерам и сырым указателям
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