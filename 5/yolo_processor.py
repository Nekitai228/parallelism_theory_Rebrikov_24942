import cv2
import logging
from ultralytics import YOLO

class YOLOProcessor:
    """Класс для обработки кадров моделью YOLOv8-pose (RAII)"""
    
    def __init__(self, model_name: str = 'yolov8s-pose.pt'):
        """
        Инициализация модели (конструктор RAII)
        
        Args:
            model_name: Имя модели YOLOv8
        """
        self.model_name = model_name
        self.model = None
        
        try:
            # Загрузка модели
            logging.info(f"Loading model: {model_name}")
            self.model = YOLO(model_name)
            logging.info(f"Model {model_name} loaded successfully")
        except Exception as e:
            logging.error(f"Failed to load model: {e}")
            raise
    
    def process_frame(self, frame):
        """
        Обработка одного кадра
        
        Args:
            frame: Кадр изображения (numpy array)
            
        Returns:
            Обработанный кадр с keypoints
        """
        if self.model is None:
            raise RuntimeError("Model not initialized")
        
        # Инференс
        results = self.model(frame, verbose=False)
        
        # Отрисовка keypoints
        result_frame = results[0].plot()
        
        return result_frame
    
    def __del__(self):
        """Деструктор (RAII) — освобождение ресурсов"""
        if self.model is not None:
            logging.info("Releasing YOLO model resources")
            del self.model