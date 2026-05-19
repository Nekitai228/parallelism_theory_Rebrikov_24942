#!/bin/bash
set -e

# По умолчанию запускаем всё
RUN_HOST=false
RUN_MULTICORE=false
RUN_GPU=true

# Парсинг аргументов
for arg in "$@"; do
    case $arg in
        --skip-host)    RUN_HOST=false ;;
        --skip-multicore) RUN_MULTICORE=false ;;
        --skip-gpu)     RUN_GPU=false ;;
        --only-gpu)
            RUN_HOST=false; RUN_MULTICORE=false; RUN_GPU=true ;;
        --only-cpu)
            RUN_GPU=false ;;
        --help|-h)
            echo "Использование: ./benchmark.sh [опции]"
            echo "Опции:"
            echo "  --skip-host       Пропустить CPU single-core (128-512)"
            echo "  --skip-multicore  Пропустить CPU multi-core"
            echo "  --skip-gpu        Пропустить GPU"
            echo "  --only-gpu        Запустить только GPU тесты"
            echo "  --only-cpu        Запустить только CPU тесты"
            echo "  --help            Показать эту справку"
            exit 0
            ;;
        *) echo " Неизвестная опция: $arg"; exit 1 ;;
    esac
done

# Размеры сеток по заданию
SIZES_HOST=(128 256 512)        # CPU-onecore только до 512
SIZES_OTHER=(128 256 512 1024)  # Multicore и GPU до 1024
OUTPUT_DIR="results"
mkdir -p "$OUTPUT_DIR"

echo "=== Heat Equation Solver Benchmark ==="
echo "Date: $(date)"
echo "Config: host=$RUN_HOST | multicore=$RUN_MULTICORE | gpu=$RUN_GPU"
echo ""

run_test() {
    local mode=$1
    local size=$2
    local build_dir="build_${mode}"
    
    echo -n "[$(date +%H:%M:%S)] Building ${mode} for ${size}x${size}... "
    rm -rf "$build_dir"
    cmake -S . -B "$build_dir" -DACC_MODE="${mode}" > /dev/null 2>&1
    cmake --build "$build_dir" --target heat_solver -j$(nproc) > /dev/null 2>&1
    echo "OK"
    
    echo -n "[$(date +%H:%M:%S)] Running ${size}x${size}... "
    ./"${build_dir}/heat_solver" --nx "${size}" --ny "${size}" --max-iter 1000000 --tol 1e-6 2>&1 | tee "${OUTPUT_DIR}/${mode}_${size}x${size}.log"
    echo "Done"
}

if $RUN_HOST; then
    echo -e " CPU Single Core (host)"
    for s in "${SIZES_HOST[@]}"; do run_test "host" "$s"; done
else
    echo -e " Пропускаем CPU Single Core (host)"
fi

if $RUN_MULTICORE; then
    echo -e "\n CPU Multicore (multicore)"
    for s in "${SIZES_OTHER[@]}"; do run_test "multicore" "$s"; done
else
    echo -e "\n Пропускаем CPU Multicore (multicore)"
fi

if $RUN_GPU; then
    echo -e "\n GPU (gpu)"
    for s in "${SIZES_OTHER[@]}"; do run_test "gpu" "$s"; done
else
    echo -e "\n Пропускаем GPU (gpu)"
fi

echo -e "\nBenchmark completed! Logs in: $OUTPUT_DIR/"