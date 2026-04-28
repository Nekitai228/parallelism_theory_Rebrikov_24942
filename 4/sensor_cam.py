import cv2
import threading
import queue
import logging

class SensorCam:
    def __init__(self, camera_name: str, resolution: tuple):
        self.camera_name = camera_name
        self.resolution = resolution
        self.queue = queue.Queue()
        self.thread = threading.Thread(target=self._read_loop, daemon=True)
        self.running = True
        self.last_frame = None
        self.cap = None
        self.error_occurred = False  # 1. Добавляем флаг ошибки
        
        # Инициализация камеры (RAII)
        try:
            if camera_name.isdigit():
                self.cap = cv2.VideoCapture(int(camera_name))
            else:
                self.cap = cv2.VideoCapture(camera_name)
            
            if not self.cap.isOpened():
                raise Exception(f"Cannot open camera {camera_name}")
            
            self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, resolution[0])
            self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, resolution[1])
            
            logging.info(f"Camera {camera_name} initialized at {resolution}")
            
        except Exception as e:
            logging.error(f"Failed to initialize camera: {e}")
            self.error_occurred = True
            raise
    
    def start(self):
        if not self.error_occurred:
            self.thread.start()
            logging.info("Started camera thread")
        
    def stop(self):
        """Остановка потока с ожиданием завершения"""
        self.running = False
        if self.thread.is_alive():
            self.thread.join(timeout=2.0)
        logging.info("Stopped camera thread")
    
    def _read_loop(self):
        """Постоянное чтение кадров с обработкой ошибок"""
        while self.running:
            try:
                ret, frame = self.cap.read()
                
                # Проверка на успешное чтение
                if not ret or frame is None:
                    logging.error("Camera read failed - device may be disconnected")
                    self.error_occurred = True  # 2. Устанавливаем флаг при ошибке <---
                    break
                
                # Очищаем очередь от старых кадров
                try:
                    while True:
                        self.queue.get_nowait()
                except queue.Empty:
                    pass
                
                self.queue.put(frame)
                self.last_frame = frame
                
            except cv2.error as e:
                logging.error(f"OpenCV error (camera disconnected?): {e}")
                self.error_occurred = True
                break
            except Exception as e:
                logging.error(f"Camera error: {e}")
                self.error_occurred = True
                break
    
    def get_latest(self):
        """Получение последнего кадра"""
        if self.error_occurred:
            return None
        try:
            return self.queue.get_nowait()
        except queue.Empty:
            return self.last_frame
    
    def is_error(self):  # 3. Добавляем метод проверки <---
        """Проверка наличия ошибки"""
        return self.error_occurred
    
    def __del__(self):
        """Деструктор (RAII) — освобождение ресурсов"""
        if hasattr(self, 'cap') and self.cap is not None:
            self.cap.release()
            logging.info("Camera released")