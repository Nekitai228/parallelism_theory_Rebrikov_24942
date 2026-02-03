#include <iostream>
#include <cmath>
#include <cstring>
#include <vector>
#define PI 3.1415926535

float sum(std::vector<float>& vec)
{
    float total = 0.0f; 
    for (auto it = vec.cbegin(); it != vec.cend(); ++it)
    {
        total += (*it);
    }
    return total; 
}

double sum(std::vector<double>& vec)
{
    double total = 0.0;
    for (auto it = vec.cbegin(); it != vec.cend(); ++it)
    {
        total += (*it);
    }
    return total; 
}

int main(int argc, char** argv)
{
    const unsigned int size = 10000000;
    if (argc <= 1) return 0;

    if (strcmp(argv[1], "float") == 0) 
    {
        std::vector<float> v1;
        v1.reserve(size); // Оптимизация: резервирование памяти

        for (unsigned int i = 0; i < size; i++) 
        {
            v1.push_back(static_cast<float>(std::sin(static_cast<double>(i)/size * 2.0 * PI)));
        }
        std::cout << sum(v1) << std::endl;
    }
    else if (strcmp(argv[1], "double") == 0) 
    {
        std::vector<double> v2;
        v2.reserve(size);

        for (unsigned int i = 0; i < size; i++)
        {
            v2.push_back(std::sin(static_cast<double>(i)/size * 2.0 * PI));
        }
        std::cout << sum(v2) << std::endl;
    }

    return 0;
}