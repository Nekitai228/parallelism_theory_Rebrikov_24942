import cv2
import threading
import queue
import logging
import os

class SensorCam:
    """Класс для работы с USB-камерой"""
    
    def __init__(self, camera_name: str, resolution: tuple):
        self.camera_name = camera_name
        self.resolution = resolution
        self.queue = queue.Queue()
        self.thread = threading.Thread(target=self._read_loop, daemon=True)
        self.running = True
        self.last_frame = None
        
        # Инициализация камеры (RAII)
        try:
            if camera_name.isdigit():
                self.cap = cv2.VideoCapture(int(camera_name))
            else:
                self.cap = cv2.VideoCapture(camera_name)
            
            if not self.cap.isOpened():
                raise Exception(f"Cannot open camera {camera_name}")
            
            # Установка разрешения
            self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, resolution[0])
            self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, resolution[1])
            
            logging.info(f"Camera {camera_name} initialized at {resolution}")
            
        except Exception as e:
            logging.error(f"Failed to initialize camera: {e}")
            raise
    
    def start(self):
        """Запуск потока чтения камеры"""
        self.thread.start()
        logging.info("Started camera thread")
        
    def stop(self):
        """Остановка потока"""
        self.running = False
        self.thread.join(timeout=1.0)
        logging.info("Stopped camera thread")
    
    def _read_loop(self):
        """Постоянное чтение кадров"""
        while self.running:
            try:
                ret, frame = self.cap.read()
                if not ret:
                    raise Exception("Camera read failed")
                
                # Очищаем очередь от старых кадров
                try:
                    while True:
                        self.queue.get_nowait()
                except queue.Empty:
                    pass
                
                self.queue.put(frame)
                self.last_frame = frame
                
            except Exception as e:
                logging.error(f"Camera error: {e}")
                break
    
    def get_latest(self):
        """Получение последнего кадра"""
        try:
            return self.queue.get_nowait()
        except queue.Empty:
            return self.last_frame
    
    def __del__(self):
        """Деструктор (RAII) — освобождение ресурсов"""
        if hasattr(self, 'cap') and self.cap is not None:
            self.cap.release()
            logging.info("Camera released")