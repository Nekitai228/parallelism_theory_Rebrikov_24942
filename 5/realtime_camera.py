import cv2
import threading
import queue
from ultralytics import YOLO
import logging

class RealTimeCamera:
    """Real-time обработка с камеры"""
    
    def __init__(self, camera_id: int = 0):
        self.cap = cv2.VideoCapture(camera_id)
        self.model = YOLO('yolov8s-pose.pt')
        self.frame_queue = queue.Queue(maxsize=5)
        self.result_queue = queue.Queue()
        self.running = False
        
    def start(self):
        self.running = True
        threading.Thread(target=self._read_loop, daemon=True).start()
        threading.Thread(target=self._process_loop, daemon=True).start()
        
    def _read_loop(self):
        while self.running:
            ret, frame = self.cap.read()
            if not ret:
                break
            try:
                self.frame_queue.put(frame, timeout=0.1)
            except queue.Full:
                continue
    
    def _process_loop(self):
        while self.running:
            try:
                frame = self.frame_queue.get(timeout=0.1)
                results = self.model(frame, verbose=False)
                result_frame = results[0].plot()
                try:
                    self.result_queue.put(result_frame, timeout=0.1)
                except queue.Full:
                    continue
            except queue.Empty:
                continue
    
    def show(self):
        while self.running:
            try:
                frame = self.result_queue.get(timeout=0.1)
                cv2.imshow('YOLOv8-pose Real-time', frame)
                if cv2.waitKey(1) & 0xFF == ord('q'):
                    break
            except queue.Empty:
                continue
    
    def stop(self):
        self.running = False
        self.cap.release()
        cv2.destroyAllWindows()

if __name__ == "__main__":
    camera = RealTimeCamera(0)
    camera.start()
    camera.show()
    camera.stop()