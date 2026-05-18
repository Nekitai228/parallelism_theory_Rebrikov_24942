#!/bin/bash


# Параметры
SIZES=(128 256 512 1024)
MAX_ITER=1000000
TOL=1e-6
OUTPUT_DIR="results"

# Создаём папку для результатов
mkdir -p "$OUTPUT_DIR"

echo "=== Heat Equation Solver Benchmark ==="
echo "Date: $(date)"
echo "Output: $OUTPUT_DIR/"
echo ""

# Функция для запуска теста
run_test() 
{
    local mode=$1
    local size=$2
    local output_file="${OUTPUT_DIR}/${mode}_${size}x${size}.log"
    
    echo -n "Testing ${mode} ${size}x${size}... "
    
    # Собираем и запускаем
    make clean > /dev/null 2>&1
    make "$mode" > /dev/null 2>&1
    
    if [ ! -f heat_solver ]; then
        echo "Build failed"
        return 1
    fi
    
    # Запускаем с замером времени
    ./heat_solver --nx "$size" --ny "$size" --max-iter "$MAX_ITER" --tol "$TOL" 2>&1 | tee "$output_file"
    
    echo " Done -> $output_file"
}

# === CPU Single Core ===
echo -e "\n CPU Single Core (-acc=host)"
echo "-----------------------------------"
for size in "${SIZES[@]}"; do
    run_test "cpu" "$size"
done

# === CPU Multicore ===
echo -e "\n CPU Multicore (-acc=multicore)"
echo "--------------------------------------"
for size in "${SIZES[@]}"; do
    run_test "multi" "$size"
done

# === GPU ===
echo -e "\n GPU (-acc=gpu)"
echo "--------------------"
for size in "${SIZES[@]}"; do
    run_test "gpu" "$size"
done

echo -e "\nBenchmark completed!"
echo "Results saved in: $OUTPUT_DIR/"