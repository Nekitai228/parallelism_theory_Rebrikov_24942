#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <vector>
#include <iomanip>
#include <regex>

struct ValidationResult 
{
    size_t total_tasks;
    size_t correct_tasks;
    size_t incorrect_tasks;
    size_t nan_tasks;
    double max_error;
    bool success;
};

class ResultValidator 
{
private:
    static constexpr double EPSILON_SIN = 1e-5;
    static constexpr double EPSILON_SQRT = 1e-5;
    static constexpr double EPSILON_POW = 1e-2;
    
    static bool is_close(double a, double b, double epsilon, double& error) 
    {
        double max_val = std::max(std::abs(a), std::abs(b));
        if (max_val > 100.0) 
        {
            error = std::abs(a - b) / max_val;
        } 
        else 
        {
            error = std::abs(a - b);
        }
        return error < epsilon;
    }
    
    static bool is_nan(const std::string& str) 
    {
        return str == "nan" || str == "-nan" || str == "NaN" || str == "-NaN";
    }
    
public:
    static ValidationResult validate_sin(const std::string& filename);
    static ValidationResult validate_sqrt(const std::string& filename);
    static ValidationResult validate_pow(const std::string& filename);
    static void print_validation_report(const std::string& test_name, 
                                        const ValidationResult& result);
    static bool run_all_tests(const std::vector<std::string>& files = {});
};