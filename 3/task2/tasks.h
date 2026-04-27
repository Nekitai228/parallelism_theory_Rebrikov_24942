
#pragma once

#include "client.h"
#include <cmath>
#include <sstream>
#include <iomanip>

// Клиент для sin
class SinClient : public Client<double, double> 
{
public:
    SinClient(size_t n_tasks, const std::string& filename)
        : Client<double, double>("SinClient", n_tasks, filename) {}

    double generate_task() override 
    {
        std::uniform_real_distribution<double> dist(-10.0, 10.0);
        return dist(rng);
    }

    std::string format_result(const double& task, const double& result) override 
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(6);
        oss << "sin(" << task << ") = " << result;
        return oss.str();
    }
};

// Клиент для sqrt
class SqrtClient : public Client<double, double> 
{
public:
    SqrtClient(size_t n_tasks, const std::string& filename)
        : Client<double, double>("SqrtClient", n_tasks, filename) {}

    double generate_task() override 
    {
        std::uniform_real_distribution<double> dist(0.0, 100.0);
        return dist(rng);
    }

    std::string format_result(const double& task, const double& result) override {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(6);
        oss << "sqrt(" << task << ") = " << result;
        return oss.str();
    }
};

// Клиент для pow
class PowClient : public Client<std::pair<double, double>, double> 
{
public:
    PowClient(size_t n_tasks, const std::string& filename)
        : Client<std::pair<double, double>, double>("PowClient", n_tasks, filename) {}

    std::pair<double, double> generate_task() override 
    {
        std::uniform_real_distribution<double> dist_base(-5.0, 5.0);
        std::uniform_real_distribution<double> dist_exp(-3.0, 3.0);
        return {dist_base(rng), dist_exp(rng)};
    }

    std::string format_result(const std::pair<double, double>& task, 
                              const double& result) override 
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(6);
        oss << "pow(" << task.first << ", " << task.second << ") = " << result;
        return oss.str();
    }
};