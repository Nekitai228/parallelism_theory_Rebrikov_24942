import argparse
import logging
import os
import time
from main import process_single_thread, process_multi_thread

def benchmark(input_path: str, output_base: str):
    """
    Бенчмарк для поиска оптимального числа workers
    """
    setup_logging()
    
    results = []
    
    # Однопоточный режим
    logging.info("=== Benchmark: Single Thread ===")
    output_path = f"{output_base}_single.mp4"
    single_time = process_single_thread(input_path, output_path)
    results.append(('single', 1, single_time))
    
    # Многопоточный режим (2, 4, 8, 16 workers)
    for workers in [2, 4, 8, 16]:
        logging.info(f"=== Benchmark: Threads ({workers}) ===")
        output_path = f"{output_base}_threads_{workers}.mp4"
        elapsed = process_multi_thread(input_path, output_path, workers, False)
        speedup = single_time / elapsed
        results.append(('threads', workers, elapsed, speedup))
    
    # Многопроцессорный режим
    for workers in [2, 4, 8]:
        logging.info(f"=== Benchmark: Processes ({workers}) ===")
        output_path = f"{output_base}_processes_{workers}.mp4"
        elapsed = process_multi_thread(input_path, output_path, workers, True)
        speedup = single_time / elapsed
        results.append(('processes', workers, elapsed, speedup))
    
    # Вывод результатов
    logging.info("\n=== BENCHMARK RESULTS ===")
    logging.info(f"{'Mode':<12} {'Workers':<10} {'Time (s)':<12} {'Speedup':<10}")
    logging.info("-" * 50)
    
    for r in results:
        if len(r) == 3:
            mode, workers, elapsed = r
            speedup = single_time / elapsed if elapsed > 0 else 0
        else:
            mode, workers, elapsed, speedup = r
        
        logging.info(f"{mode:<12} {workers:<10} {elapsed:<12.2f} {speedup:<10.2f}")
    
    # Находим лучший результат
    best = max(results[1:], key=lambda x: single_time / x[2] if len(x) > 2 else 0)
    logging.info(f"\nBest configuration: {best[0]} with {best[1]} workers")

def setup_logging():
    os.makedirs('log', exist_ok=True)
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(levelname)s - %(message)s',
        handlers=[
            logging.FileHandler(f'log/benchmark_{time.time()}.log'),
            logging.StreamHandler()
        ]
    )

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--input', type=str, required=True)
    parser.add_argument('--output', type=str, required=True)
    args = parser.parse_args()
    
    benchmark(args.input, args.output)