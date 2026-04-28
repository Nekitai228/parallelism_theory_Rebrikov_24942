import cv2
import logging

class WindowImage:
    """Класс для отображения изображения"""
    
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
            key = cv2.waitKey(1) & 0xFF
            if key == ord('q'):
                self.running = False
                return False
            return True
        except Exception as e:
            logging.error(f"Error displaying image: {e}")
            return False
    
    def __del__(self):
        """Деструктор (RAII) — закрытие окна"""
        cv2.destroyWindow(self.window_name)
        cv2.destroyAllWindows()
        logging.info("Window destroyed")