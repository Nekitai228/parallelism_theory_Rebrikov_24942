import argparse
import logging
import os
import time
import cv2
from yolo_processor import YOLOProcessor
from video_processor import VideoProcessor
import os
os.environ['CUDA_VISIBLE_DEVICES'] = '-1'  # Отключаем GPU, используем только CPU

def setup_logging():
    """Настройка логирования"""
    os.makedirs('log', exist_ok=True)
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(levelname)s - %(message)s',
        handlers=[
            logging.FileHandler(f'log/yolo_{time.time()}.log'),
            logging.StreamHandler()
        ]
    )

def process_single_thread(input_path: str, output_path: str) -> float:
    """
    Однопоточная обработка видео
    
    Args:
        input_path: Путь к входному видео
        output_path: Путь к выходному видео
        
    Returns:
        Время обработки
    """
    logging.info("Starting single-threaded processing")
    
    cap = cv2.VideoCapture(input_path)
    if not cap.isOpened():
        raise ValueError(f"Cannot open video: {input_path}")
    
    fps = cap.get(cv2.CAP_PROP_FPS)
    width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out = cv2.VideoWriter(output_path, fourcc, fps, (width, height))
    
    processor = YOLOProcessor()
    
    start_time = time.time()
    frame_count = 0
    
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        
        processed_frame = processor.process_frame(frame)
        out.write(processed_frame)
        frame_count += 1
        
        if frame_count % 30 == 0:
            logging.info(f"Processed {frame_count}/{total_frames} frames")
    
    elapsed_time = time.time() - start_time
    
    cap.release()
    out.release()
    
    logging.info(f"Single-threaded processing completed: "
                f"{frame_count} frames in {elapsed_time:.2f}s "
                f"({frame_count/elapsed_time:.2f} FPS)")
    
    return elapsed_time

def process_multi_thread(input_path: str, output_path: str, 
                        num_workers: int, use_processes: bool) -> float:
    """
    Многопоточная/многопроцессорная обработка
    
    Args:
        input_path: Путь к входному видео
        output_path: Путь к выходному видео
        num_workers: Количество workers
        use_processes: True для процессов
        
    Returns:
        Время обработки
    """
    mode = "processes" if use_processes else "threads"
    logging.info(f"Starting multi-{mode} processing with {num_workers} workers")
    
    processor = VideoProcessor(input_path, output_path, 
                              num_workers, use_processes)
    
    elapsed_time = processor.process()
    
    # Считаем FPS
    cap = cv2.VideoCapture(input_path)
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    cap.release()
    
    logging.info(f"Multi-{mode} processing completed: "
                f"{total_frames} frames in {elapsed_time:.2f}s "
                f"({total_frames/elapsed_time:.2f} FPS)")
    
    return elapsed_time

def main():
    parser = argparse.ArgumentParser(
        description='YOLOv8-pose Video Processing with Parallelization'
    )
    parser.add_argument('--input', type=str, required=True,
                       help='Path to input video (640x480)')
    parser.add_argument('--output', type=str, required=True,
                       help='Path to output video')
    parser.add_argument('--mode', type=str, choices=['single', 'multi'],
                       default='single',
                       help='Processing mode: single or multi-threaded')
    parser.add_argument('--workers', type=int, default=4,
                       help='Number of workers (for multi mode)')
    parser.add_argument('--processes', action='store_true',
                       help='Use processes instead of threads')
    
    args = parser.parse_args()
    
    setup_logging()
    logging.info("=== YOLOv8-pose Video Processor ===")
    logging.info(f"Input: {args.input}")
    logging.info(f"Output: {args.output}")
    logging.info(f"Mode: {args.mode}")
    
    if args.mode == 'single':
        elapsed_time = process_single_thread(args.input, args.output)
    else:
        elapsed_time = process_multi_thread(
            args.input, args.output,
            args.workers, args.processes
        )
    
    logging.info(f"Total processing time: {elapsed_time:.2f} seconds")
    logging.info(f"Output saved to: {args.output}")

if __name__ == "__main__":
    main()