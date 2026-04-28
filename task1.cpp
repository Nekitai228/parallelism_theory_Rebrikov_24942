#include <iostream>
#include <cmath>
#include <cstring>
#include <numeric>
#include <vector>

float sum(std::vector<float> &vec)
{
    //Функция для подсчёта суммы
    return std::accumulate(vec.cbegin(), vec.cend(), 0);
}

double sum(std::vector<double> &vec)
{
    //Функция для подсчёта суммы
    return std::accumulate(vec.cbegin(), vec.cend(), 0);
}



int main(int argc, char** argv)
{
    unsigned int size = 10000000;
    if(argc <= 1) return 0;
    if(strcmp(argv[1], "float"))
    {
        std::vector<float> v1;
        for(int i = 0; i < size; i++)
        {
            v1.push_back(std::sin());
        }
        std::cout << sum(v1) << std::endl;
    }

    return 0;
}