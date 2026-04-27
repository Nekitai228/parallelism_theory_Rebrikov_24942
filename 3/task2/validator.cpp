#include "validator.h"


ValidationResult ResultValidator::validate_sin(const std::string& filename) {
    ValidationResult result = {0, 0, 0, 0, 0.0, true};
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open " << filename << "\n";
        result.success = false;
        return result;
    }
    
    std::string line;
    std::regex sin_pattern(R"(sin\((-?[0-9]+\.?[0-9]*(?:[eE][+-]?[0-9]+)?)\)\s*=\s*(-?[0-9]+\.?[0-9]*(?:[eE][+-]?[0-9]+)?))");
    std::smatch matches;
    
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        if (std::regex_search(line, matches, sin_pattern)) {
            result.total_tasks++;
            
            try {
                double input = std::stod(matches[1].str());
                double actual_result = std::stod(matches[2].str());
                double expected_result = std::sin(input);
                double error;
                
                if (is_close(actual_result, expected_result, EPSILON_SIN, error)) {
                    result.correct_tasks++;
                } else {
                    result.incorrect_tasks++;
                    result.max_error = std::max(result.max_error, error);
                    result.success = false;
                    
                    if (result.incorrect_tasks <= 5) {
                        std::cout << std::fixed << std::setprecision(10);
                        std::cout << "  MISMATCH: sin(" << input << ")\n";
                        std::cout << "    Expected: " << expected_result << "\n";
                        std::cout << "    Got:      " << actual_result << "\n";
                        std::cout << "    Error:    " << error << "\n\n";
                    }
                }
            } catch (...) {
                result.incorrect_tasks++;
                result.success = false;
            }
        }
    }
    
    return result;
}

ValidationResult ResultValidator::validate_sqrt(const std::string& filename) {
    ValidationResult result = {0, 0, 0, 0, 0.0, true};
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open " << filename << "\n";
        result.success = false;
        return result;
    }
    
    std::string line;
    std::regex sqrt_pattern(R"(sqrt\(([0-9]+\.?[0-9]*(?:[eE][+-]?[0-9]+)?)\)\s*=\s*([0-9]+\.?[0-9]*(?:[eE][+-]?[0-9]+)?))");
    std::smatch matches;
    
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        if (std::regex_search(line, matches, sqrt_pattern)) {
            result.total_tasks++;
            
            try {
                double input = std::stod(matches[1].str());
                double actual_result = std::stod(matches[2].str());
                double expected_result = std::sqrt(input);
                double error;
                
                if (is_close(actual_result, expected_result, EPSILON_SQRT, error)) {
                    result.correct_tasks++;
                } else {
                    result.incorrect_tasks++;
                    result.max_error = std::max(result.max_error, error);
                    result.success = false;
                    
                    if (result.incorrect_tasks <= 5) {
                        std::cout << std::fixed << std::setprecision(10);
                        std::cout << "  MISMATCH: sqrt(" << input << ")\n";
                        std::cout << "    Expected: " << expected_result << "\n";
                        std::cout << "    Got:      " << actual_result << "\n";
                        std::cout << "    Error:    " << error << "\n\n";
                    }
                }
            } catch (...) {
                result.incorrect_tasks++;
                result.success = false;
            }
        }
    }
    
    return result;
}

ValidationResult ResultValidator::validate_pow(const std::string& filename) {
    ValidationResult result = {0, 0, 0, 0, 0.0, true};
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open " << filename << "\n";
        result.success = false;
        return result;
    }
    
    std::string line;
    std::regex pow_pattern(R"(pow\((-?[0-9]+\.?[0-9]*(?:[eE][+-]?[0-9]+)?),\s*(-?[0-9]+\.?[0-9]*(?:[eE][+-]?[0-9]+)?)\)\s*=\s*(-?[0-9]+\.?[0-9]*(?:[eE][+-]?[0-9]+)?|nan|-nan|NaN|-NaN|inf|-inf|Inf|-Inf))");
    std::smatch matches;
    
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        if (std::regex_search(line, matches, pow_pattern)) {
            result.total_tasks++;
            
            try {
                double base = std::stod(matches[1].str());
                double exp = std::stod(matches[2].str());
                std::string result_str = matches[3].str();
                
                if (is_nan(result_str)) {
                    double expected_result = std::pow(base, exp);
                    if (std::isnan(expected_result)) {
                        result.nan_tasks++;
                        result.correct_tasks++;
                    } else {
                        result.incorrect_tasks++;
                        result.success = false;
                        if (result.incorrect_tasks <= 5) {
                            std::cout << "  UNEXPECTED NAN: pow(" << base << ", " << exp << ")\n";
                            std::cout << "    Expected: " << expected_result << "\n";
                            std::cout << "    Got:      nan\n\n";
                        }
                    }
                    continue;
                }
                
                double actual_result = std::stod(result_str);
                double expected_result = std::pow(base, exp);
                double error;
                
                if (std::isnan(actual_result) && std::isnan(expected_result)) {
                    result.nan_tasks++;
                    result.correct_tasks++;
                    continue;
                }
                
                if (is_close(actual_result, expected_result, EPSILON_POW, error)) {
                    result.correct_tasks++;
                } else {
                    result.incorrect_tasks++;
                    result.max_error = std::max(result.max_error, error);
                    result.success = false;
                    if (result.incorrect_tasks <= 5) {
                        std::cout << std::fixed << std::setprecision(10);
                        std::cout << "  MISMATCH: pow(" << base << ", " << exp << ")\n";
                        std::cout << "    Expected: " << expected_result << "\n";
                        std::cout << "    Got:      " << actual_result << "\n";
                        std::cout << "    Error:    " << error << "\n\n";
                    }
                }
            } catch (...) {
                result.incorrect_tasks++;
                result.success = false;
            }
        } else {
            if (result.total_tasks < 5) {
                std::cerr << "  UNPARSED: " << line << "\n";
            }
        }
    }
    
    return result;
}

void ResultValidator::print_validation_report(const std::string& test_name, 
                                               const ValidationResult& result) {
    std::cout << "\n=== Validation: " << test_name << " ===\n";
    std::cout << std::fixed << std::setprecision(0);
    std::cout << "Total tasks:     " << result.total_tasks << "\n";
    std::cout << "Correct:         " << result.correct_tasks << "\n";
    if (result.nan_tasks > 0) {
        std::cout << "  (incl. NaN:    " << result.nan_tasks << ")\n";
    }
    std::cout << "Incorrect:       " << result.incorrect_tasks << "\n";
    
    if (result.incorrect_tasks > 0) {
        std::cout << std::setprecision(10);
        std::cout << "Max error:       " << result.max_error << "\n";
    }
    
    std::cout << "Status:          " << (result.success ? "PASSED" : "FAILED") << "\n";
}

bool ResultValidator::run_all_tests(const std::vector<std::string>& files) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "           RESULTS VALIDATION\n";
    std::cout << std::string(60, '=') << "\n";
    
    // Определяем файлы для проверки
    std::vector<std::string> test_files = files;
    if (test_files.empty()) {
        test_files = {"sin_results.txt", "sqrt_results.txt", "pow_results.txt"};
    }
    
    std::vector<ValidationResult> results;
    std::vector<std::string> test_names = {"sin()", "sqrt()", "pow()"};
    
    for (size_t i = 0; i < test_files.size(); i++) {
        ValidationResult result;
        if (test_files[i].find("sin") != std::string::npos) {
            result = validate_sin(test_files[i]);
        } else if (test_files[i].find("sqrt") != std::string::npos) {
            result = validate_sqrt(test_files[i]);
        } else if (test_files[i].find("pow") != std::string::npos) {
            result = validate_pow(test_files[i]);
        }
        results.push_back(result);
        print_validation_report(test_names[i], result);
    }
    
    std::cout << "\n" << std::string(60, '=') << "\n";
    bool all_passed = true;
    for (const auto& r : results) {
        if (!r.success) {
            all_passed = false;
            break;
        }
    }
    
    if (all_passed) {
        std::cout << "ALL TESTS PASSED!\n";
    } else {
        std::cout << "SOME TESTS FAILED!\n";
    }
    std::cout << std::string(60, '=') << "\n";
    
    return all_passed;
}

// === ТОЧКА ВХОДА ===
int main(int argc, char* argv[]) {
    std::cout << "=== Standalone Result Validator ===\n";
    
    std::vector<std::string> files;
    
    // Если переданы аргументы — используем их как имена файлов
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            files.push_back(argv[i]);
        }
        std::cout << "Checking files: ";
        for (const auto& f : files) std::cout << f << " ";
        std::cout << "\n";
    } else {
        std::cout << "No files specified. Using defaults:\n";
        std::cout << "  - sin_results.txt\n";
        std::cout << "  - sqrt_results.txt\n";
        std::cout << "  - pow_results.txt\n";
    }
    
    bool success = ResultValidator::run_all_tests(files);
    
    return success ? 0 : 1;
}