
import argparse
import logging
import os
import time
import numpy as np
from sensor_cam import SensorCam
from sensor_wrapper import SensorWrapper
from sensor_x import SensorX
from window_image import WindowImage
import cv2
os.environ['OPENCV_LOG_LEVEL'] = 'ERROR'  # Только ошибки OpenCV

def setup_logging():
    """Настройка логирования"""
    os.makedirs('log', exist_ok=True)
    
    # Создаем логгер
    logger = logging.getLogger()
    logger.setLevel(logging.INFO)  # В файл пишем всё
    
    # Очищаем старые обработчики
    logger.handlers.clear()
    
    # Файловый обработчик (пишем INFO и выше)
    file_handler = logging.FileHandler(f'log/sensor_{time.time()}.log')
    file_handler.setLevel(logging.INFO)
    file_format = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    file_handler.setFormatter(file_format)
    
    # Консольный обработчик (пишем только ERROR и выше)
    console_handler = logging.StreamHandler()
    console_handler.setLevel(logging.ERROR)  # Только ошибки в консоль
    console_format = logging.Formatter('%(levelname)s - %(message)s')
    console_handler.setFormatter(console_format)
    
    logger.addHandler(file_handler)
    logger.addHandler(console_handler)

def create_composite_image(cam_frame, sensor_data_list):
    """Создание составного изображения"""
    if cam_frame is None:
        height, width = 480, 640
        cam_frame = np.zeros((height, width, 3), dtype=np.uint8)
        cv2.putText(cam_frame, "No Camera", (200, 240), 
                   cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
    
    sensor_height = 100
    total_height = cam_frame.shape[0] + sensor_height
    composite = np.zeros((total_height, cam_frame.shape[1], 3), dtype=np.uint8)
    
    composite[:cam_frame.shape[0], :] = cam_frame
    
    for i, (name, value) in enumerate(sensor_data_list):
        text = f"{name}: {value}"
        x = 10 + i * 200
        y = cam_frame.shape[0] + 30
        cv2.putText(composite, text, (x, y), 
                   cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)
    
    return composite

def main():
    parser = argparse.ArgumentParser(description='Sensor Data Display')
    parser.add_argument('--camera', type=str, default='0', 
                       help='Camera name/device')
    parser.add_argument('--resolution', type=str, default='1280x720',
                       help='Camera resolution (e.g., 1280x720)')
    parser.add_argument('--display-freq', type=float, default=30.0,
                       help='Display frequency in Hz')
    
    args = parser.parse_args()
    setup_logging()
    logging.info("Starting sensor display application")
    
    try:
        width, height = map(int, args.resolution.split('x'))
        resolution = (width, height)
    except:
        logging.error(f"Invalid resolution: {args.resolution}")
        return
    
    cam = None  #  Инициализируем заранее
    sensors = []
    window = None
    
    try:
        # Создание камеры
        cam = SensorCam(args.camera, resolution)
        
        # Создание трех датчиков SensorX с разной частотой
        sensor0 = SensorWrapper("Sensor0 (100Hz)", SensorX(0.01))
        sensor1 = SensorWrapper("Sensor1 (10Hz)", SensorX(0.1))
        sensor2 = SensorWrapper("Sensor2 (1Hz)", SensorX(1))
        
        sensors = [sensor0, sensor1, sensor2]
        
        # Запуск всех потоков
        cam.start()
        for sensor in sensors:
            sensor.start()
        
        logging.info("All sensors started")
        
        # Создание окна
        window = WindowImage(args.display_freq)
        period = 1.0 / args.display_freq
        
        # Главный цикл
        while window.running:
            start_time = time.time()

            # Проверка на ошибку камеры
            if cam.is_error():
                logging.error("Camera error detected - stopping application")
                break
            
            # Получение данных
            cam_frame = cam.get_latest()
            
            sensor_data = []
            for sensor in sensors:
                value = sensor.get_latest()
                sensor_data.append((sensor.name, value if value is not None else 0))
            
            # Создание и отображение изображения
            composite_img = create_composite_image(cam_frame, sensor_data)
            
            if not window.show(composite_img):
                break
            
            # Контроль частоты
            elapsed = time.time() - start_time
            sleep_time = period - elapsed
            if sleep_time > 0:
                time.sleep(sleep_time)
        
        # Корректная остановка
        logging.info("Stopping sensors...")
        cam.stop()
        for sensor in sensors:
            sensor.stop()
        
        logging.info("Application stopped")
        
    except KeyboardInterrupt:
        logging.info("Keyboard interrupt received - shutting down gracefully")
    except Exception as e:
        logging.error(f"Unexpected error: {e}")

    finally:
        # Всегда освобождаем ресурсы (даже если cam не создан)
        logging.info("Cleaning up resources...")
        
        # Добавляем проверку на None
        if cam is not None:
            cam.stop()
        
        for sensor in sensors:
            sensor.stop()
        
        logging.info("All resources released")

if __name__ == "__main__":
    main()