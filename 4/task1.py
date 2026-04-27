import threading
import queue
import logging
from abc import ABC, abstractmethod

class Sensor(ABC):
    def __init__(self, name: str, frequency: float):
        self.name = name
        self.frequency = frequency
        self.queue = queue.Queue()
        self.thread = threading.Thread(target=self._read_loop, daemon=True)
        self.running = True
        self.last_value = None
        
    def start(self):
        """Запуск потока чтения"""
        self.thread.start()
        
    def stop(self):
        """Остановка потока"""
        self.running = False
        
    def _read_loop(self):
        """Цикл постоянного чтения данных"""
        import time
        period = 1.0 / self.frequency
        
        while self.running:
            try:
                data = self._read_data()
                if data is not None:
                    self.last_value = data
                    # Очищаем очередь от старых данных
                    try:
                        while True:
                            self.queue.get_nowait()
                    except queue.Empty:
                        pass
                    self.queue.put(data)
            except Exception as e:
                logging.error(f"Error reading {self.name}: {e}")
            
            time.sleep(period)
    
    @abstractmethod
    def _read_data(self):
        """Метод для чтения данных (реализуется в наследниках)"""
        pass
    
    def get_latest(self):
        """Получение последних данных (или предыдущих если нет новых)"""
        try:
            return self.queue.get_nowait()
        except queue.Empty:
            return self.last_value

import cv2

class SensorCam(Sensor):
    def __init__(self, camera_name: str, resolution: tuple):
        super().__init__("Camera", frequency=30)  # Например, 30 FPS
        
        self.camera_name = camera_name
        self.resolution = resolution
        self.cap = None
        
        # Инициализация камеры
        try:
            # Пробуем открыть камеру
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
    
    def _read_data(self):
        """Чтение кадра с камеры"""
        if self.cap is None:
            return None
            
        ret, frame = self.cap.read()
        if not ret:
            raise Exception("Camera read failed")
        
        return frame
    
    def __del__(self):
        """Освобождение ресурсов камеры (RAII)"""
        if self.cap is not None:
            self.cap.release()
            logging.info("Camera released")


class SensorX(Sensor):
    def __init__(self, name: str, frequency: float):
        super().__init__(name, frequency)
        # Инициализация датчика
        
    def _read_data(self):
        """Чтение данных с датчика"""
        # Здесь должна быть логика чтения
        # Например, случайные данные для теста
        import random
        return random.uniform(0, 100)
    
    def __del__(self):
        """Освобождение ресурсов"""
        logging.info(f"Sensor {self.name} released")


class WindowImage:
    def __init__(self, display_frequency: float):
        self.display_frequency = display_frequency
        self.window_name = "Sensor Display"
        self.running = True
        
        try:
            cv2.namedWindow(self.window_name)
            logging.info(f"Window created with frequency {display_frequency} Hz")
        except Exception as e:
            logging.error(f"Failed to create window: {e}")
            raise
    
    def show(self, img):
        """Отображение изображения"""
        try:
            cv2.imshow(self.window_name, img)
            # Обработка нажатий клавиш
            key = cv2.waitKey(1) & 0xFF
            if key == ord('q'):
                self.running = False
                return False
            return True
        except Exception as e:
            logging.error(f"Error displaying image: {e}")
            return False
    
    def __del__(self):
        """Освобождение ресурсов окна (RAII)"""
        cv2.destroyWindow(self.window_name)
        cv2.destroyAllWindows()
        logging.info("Window destroyed")


import argparse
import logging
import os
import time
import numpy as np

def setup_logging():
    """Настройка логирования"""
    os.makedirs('log', exist_ok=True)
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
        handlers=[
            logging.FileHandler(f'log/sensor_{time.time()}.log'),
            logging.StreamHandler()
        ]
    )

def create_composite_image(cam_frame, sensor_data_list):
    """Создание составного изображения с данными датчиков"""
    if cam_frame is None:
        # Создаем черное изображение если нет кадра
        height, width = 480, 640
        cam_frame = np.zeros((height, width, 3), dtype=np.uint8)
    
    # Создаем область для датчиков
    sensor_height = 100
    total_height = cam_frame.shape[0] + sensor_height
    composite = np.zeros((total_height, cam_frame.shape[1], 3), dtype=np.uint8)
    
    # Копируем кадр с камеры
    composite[:cam_frame.shape[0], :] = cam_frame
    
    # Отображаем данные датчиков
    for i, (name, value) in enumerate(sensor_data_list):
        text = f"{name}: {value:.2f}"
        x = 10 + i * 200
        y = cam_frame.shape[0] + 30
        cv2.putText(composite, text, (x, y), 
                   cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)
    
    return composite

def main():
    # Парсинг аргументов командной строки
    parser = argparse.ArgumentParser(description='Sensor Data Display')
    parser.add_argument('--camera', type=str, default='0', 
                       help='Camera name/device (e.g., 0 or /dev/video0)')
    parser.add_argument('--resolution', type=str, default='1280x720',
                       help='Camera resolution (e.g., 1280x720)')
    parser.add_argument('--display-freq', type=float, default=30.0,
                       help='Display frequency in Hz')
    
    args = parser.parse_args()
    
    # Настройка логирования
    setup_logging()
    logging.info("Starting sensor display application")
    
    # Парсинг разрешения
    try:
        width, height = map(int, args.resolution.split('x'))
        resolution = (width, height)
    except:
        logging.error(f"Invalid resolution format: {args.resolution}")
        return
    
    try:
        # Создание датчиков
        cam = SensorCam(args.camera, resolution)
        
        # Три датчика SensorX с разной частотой
        sensor1 = SensorX("Sensor1", frequency=100)  # 100 Hz
        sensor2 = SensorX("Sensor2", frequency=10)   # 10 Hz
        sensor3 = SensorX("Sensor3", frequency=1)    # 1 Hz
        
        sensors = [sensor1, sensor2, sensor3]
        
        # Запуск всех потоков
        cam.start()
        for sensor in sensors:
            sensor.start()
        
        logging.info("All sensors started")
        
        # Создание окна отображения
        window = WindowImage(args.display_freq)
        
        # Главный цикл
        period = 1.0 / args.display_freq
        
        while window.running:
            start_time = time.time()
            
            # Получение данных с камеры
            cam_frame = cam.get_latest()
            
            # Получение данных с датчиков
            sensor_data = []
            for sensor in sensors:
                value = sensor.get_latest()
                sensor_data.append((sensor.name, value if value is not None else 0))
            
            # Создание составного изображения
            composite_img = create_composite_image(cam_frame, sensor_data)
            
            # Отображение
            if not window.show(composite_img):
                break
            
            # Контроль частоты отображения
            elapsed = time.time() - start_time
            sleep_time = period - elapsed
            if sleep_time > 0:
                time.sleep(sleep_time)
        
        # Корректная остановка
        logging.info("Stopping sensors...")
        cam.stop()
        for sensor in sensors:
            sensor.stop()
        
        # Ждем завершения потоков
        cam.thread.join(timeout=1.0)
        for sensor in sensors:
            sensor.thread.join(timeout=1.0)
        
        logging.info("Application stopped successfully")
        
    except Exception as e:
        logging.error(f"Application error: {e}")
        raise

if __name__ == "__main__":
    main()