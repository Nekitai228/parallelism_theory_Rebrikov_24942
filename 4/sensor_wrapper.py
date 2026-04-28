# sensor_wrapper.py
import threading
import queue
import logging
from sensor_x import SensorX

class SensorWrapper:
    """Обертка для SensorX с потоком и очередью"""
    
    def __init__(self, name: str, sensor: SensorX):
        self.name = name
        self.sensor = sensor
        self.queue = queue.Queue()
        self.thread = threading.Thread(target=self._read_loop, daemon=True)
        self.running = True
        self.last_value = None
        
    def start(self):
        """Запуск потока чтения"""
        self.thread.start()
        logging.info(f"Started {self.name} thread")
        
    def stop(self):
        """Остановка потока"""
        self.running = False
        self.thread.join(timeout=1.0)
        logging.info(f"Stopped {self.name} thread")
        
    def _read_loop(self):
        """Постоянное чтение данных в отдельном потоке"""
        while self.running:
            try:
                data = self.sensor.get()
                # Очищаем очередь от старых данных
                try:
                    while True:
                        self.queue.get_nowait()
                except queue.Empty:
                    pass
                # Кладем новые данные
                self.queue.put(data)
                self.last_value = data
            except Exception as e:
                logging.error(f"Error in {self.name}: {e}")
                break
    
    def get_latest(self):
        """Получение последних данных (или предыдущих если нет новых)"""
        try:
            return self.queue.get_nowait()
        except queue.Empty:
            return self.last_value