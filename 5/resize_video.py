import cv2
import argparse

def resize_video(input_path, output_path, new_width=640, new_height=480):
    """
    Изменение размера видео
    
    Args:
        input_path: Путь к входному видео
        output_path: Путь к выходному видео
        new_width: Новая ширина
        new_height: Новая высота
    """
    cap = cv2.VideoCapture(input_path)
    
    if not cap.isOpened():
        print(f"Error: Cannot open {input_path}")
        return
    
    # Получаем параметры видео
    fps = cap.get(cv2.CAP_PROP_FPS)
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    
    # Создаем VideoWriter
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out = cv2.VideoWriter(output_path, fourcc, fps, (new_width, new_height))
    
    print(f"Resizing video: {total_frames} frames...")
    
    frame_count = 0
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        
        # Изменяем размер кадра
        resized_frame = cv2.resize(frame, (new_width, new_height), 
                                   interpolation=cv2.INTER_AREA)
        
        out.write(resized_frame)
        frame_count += 1
        
        if frame_count % 30 == 0:
            print(f"Processed {frame_count}/{total_frames} frames")
    
    cap.release()
    out.release()
    print(f"Done! Saved to {output_path}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Resize video to 640x480')
    parser.add_argument('--input', type=str, required=True, 
                       help='Input video path')
    parser.add_argument('--output', type=str, required=True,
                       help='Output video path')
    parser.add_argument('--width', type=int, default=640,
                       help='New width (default: 640)')
    parser.add_argument('--height', type=int, default=480,
                       help='New height (default: 480)')
    
    args = parser.parse_args()
    resize_video(args.input, args.output, args.width, args.height)