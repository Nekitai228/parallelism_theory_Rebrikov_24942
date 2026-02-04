#include "config.h.in"
#include <iostream>
#include <cmath>
#include <vector>
#define PI 3.1415926535

int main() 
{
    const int ARRAY_SIZE = 10000000;
    
    // Выделяем память для массива
    std::vector<Real> vec(ARRAY_SIZE);
    
   #ifdef USE_DOUBLE_PRECISION
    // Для double
    const double step = 2.0 * PI / ARRAY_SIZE;
    for (size_t i = 0; i < ARRAY_SIZE; ++i) 
    {
        vec[i] = std::sin(i * step);
    }
    #else
    // Для float  
    const float step = 2.0f * PI / ARRAY_SIZE; 
    for (size_t i = 0; i < ARRAY_SIZE; ++i) 
    {
        vec[i] = static_cast<float>(sinf(i * step));  
    }
    #endif
    
    // Вычисляем сумму
    Real sum = 0;
    for (size_t i = 0; i < ARRAY_SIZE; ++i) 
    {
        sum += vec[i];
    }
    
    
    // Выводим результат
    std::cout.precision(15);
    std::cout << "Sum: " << sum << std::endl;
    
    return 0;
}