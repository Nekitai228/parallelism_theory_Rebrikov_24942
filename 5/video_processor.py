import cv2
import queue
import threading
import multiprocessing as mp
from concurrent.futures import ThreadPoolExecutor, ProcessPoolExecutor
import logging
import time
from collections import OrderedDict

def _process_frame_thread(frame_data):
    """
    Обработка кадра в потоке (каждый поток создает СВОЮ модель)
    """
    from ultralytics import YOLO
    
    frame_id, frame = frame_data
    
    try:
        # Создаем НОВУЮ модель для каждого кадра (медленно, но работает)
        model = YOLO('yolov8s-pose.pt')
        results = model(frame, verbose=False)
        processed_frame = results[0].plot()
        
        return (frame_id, processed_frame)
        
    except Exception as e:
        logging.error(f"Error processing frame {frame_id}: {e}")
        return (frame_id, None)


def _process_frame_process(frame_data):
    """
    Обработка кадра в процессе (каждый процесс загружает модель один раз)
    """
    # Глобальная модель для процесса (инициализируется в _init_worker_process)
    global _process_model
    
    frame_id, frame = frame_data
    
    try:
        results = _process_model(frame, verbose=False)
        processed_frame = results[0].plot()
        
        return (frame_id, processed_frame)
        
    except Exception as e:
        logging.error(f"Error processing frame {frame_id}: {e}")
        return (frame_id, None)


def _init_worker_process():
    """Инициализация worker процесса (загружает модель один раз)"""
    global _process_model
    from ultralytics import YOLO
    _process_model = YOLO('yolov8s-pose.pt')
    logging.info(f"Process worker initialized with model")


class VideoProcessor:
    def __init__(self, input_path: str, output_path: str, 
                 num_workers: int = 4, use_processes: bool = False):
        self.input_path = input_path
        self.output_path = output_path
        self.num_workers = num_workers
        self.use_processes = use_processes
        
        self.input_queue = mp.Queue(maxsize=10) if use_processes else queue.Queue(maxsize=10)
        self.output_queue = mp.Queue() if use_processes else queue.Queue()
        self.running = False
        
        self.pending_frames = OrderedDict()
        self.next_frame_id = 0
        
        self.cap = None
        self.out = None
        
        logging.info(f"VideoProcessor initialized: workers={num_workers}, "
                    f"mode={'processes' if use_processes else 'threads'}")
    
    def _read_frames(self):
        """Поток чтения кадров"""
        try:
            self.cap = cv2.VideoCapture(self.input_path)
            
            if not self.cap.isOpened():
                logging.error(f"Cannot open video: {self.input_path}")
                return
            
            fps = self.cap.get(cv2.CAP_PROP_FPS)
            total_frames = int(self.cap.get(cv2.CAP_PROP_FRAME_COUNT))
            logging.info(f"Video opened: {total_frames} frames, {fps:.2f} FPS")
            
            frame_id = 0
            while self.running:
                ret, frame = self.cap.read()
                if not ret:
                    break
                
                try:
                    self.input_queue.put((frame_id, frame), timeout=1.0)
                    frame_id += 1
                except queue.Full:
                    continue
            
            logging.info(f"Finished reading {frame_id} frames")
            
        except Exception as e:
            logging.error(f"Error reading video: {e}")
        finally:
            if self.cap is not None:
                self.cap.release()
    
    def _write_frames(self, expected_frames: int):
        """Поток записи кадров"""
        try:
            cap = cv2.VideoCapture(self.input_path)
            fps = cap.get(cv2.CAP_PROP_FPS)
            width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
            height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
            cap.release()
            
            fourcc = cv2.VideoWriter_fourcc(*'mp4v')
            self.out = cv2.VideoWriter(self.output_path, fourcc, fps, (width, height))
            
            if not self.out.isOpened():
                logging.error("Cannot create video writer")
                return
            
            written_frames = 0
            self.pending_frames = OrderedDict()
            
            while written_frames < expected_frames:
                try:
                    frame_id, processed_frame = self.output_queue.get(timeout=1.0)
                    
                    if processed_frame is None:
                        continue
                    
                    self.pending_frames[frame_id] = processed_frame
                    
                    while self.next_frame_id in self.pending_frames:
                        frame = self.pending_frames.pop(self.next_frame_id)
                        self.out.write(frame)
                        written_frames += 1
                        self.next_frame_id += 1
                    
                    if written_frames % 30 == 0:
                        logging.info(f"Written {written_frames}/{expected_frames} frames")
                        
                except queue.Empty:
                    continue
            
            logging.info(f"Finished writing {written_frames} frames")
            
        except Exception as e:
            logging.error(f"Error writing video: {e}")
        finally:
            if self.out is not None:
                self.out.release()
    
    def process(self):
        """Основной метод обработки"""
        self.running = True
        
        cap = cv2.VideoCapture(self.input_path)
        total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
        cap.release()
        
        start_time = time.time()
        
        read_thread = threading.Thread(target=self._read_frames)
        write_thread = threading.Thread(target=self._write_frames, args=(total_frames,))
        
        read_thread.start()
        write_thread.start()
        
        # ВАЖНО: Для потоков создаем отдельную модель для каждого вызова
        # Для процессов используем initializer
        if self.use_processes:
            executor = ProcessPoolExecutor(
                max_workers=self.num_workers,
                initializer=_init_worker_process
            )
            process_func = _process_frame_process
        else:
            # Для потоков - каждый вызов создает свою модель
            executor = ThreadPoolExecutor(max_workers=self.num_workers)
            process_func = _process_frame_thread
        
        # Отправляем задачи
        futures = []
        while read_thread.is_alive() or not self.input_queue.empty():
            try:
                frame_data = self.input_queue.get(timeout=1.0)
                future = executor.submit(process_func, frame_data)
                futures.append(future)
            except queue.Empty:
                continue
            except Exception as e:
                logging.error(f"Error submitting task: {e}")
        
        # Собираем результаты
        for future in futures:
            try:
                frame_id, processed_frame = future.result()
                self.output_queue.put((frame_id, processed_frame))
            except Exception as e:
                logging.error(f"Task failed: {e}")
        
        read_thread.join()
        write_thread.join()
        executor.shutdown(wait=True)
        
        elapsed_time = time.time() - start_time
        logging.info(f"Processing completed in {elapsed_time:.2f} seconds")
        
        return elapsed_time
    
    def __del__(self):
        """Деструктор (RAII)"""
        self.running = False
        if self.cap is not None:
            self.cap.release()
        if self.out is not None:
            self.out.release()