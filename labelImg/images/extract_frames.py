import cv2
import os
import glob

def get_video_info(video_path):
    cap = cv2.VideoCapture(video_path)
    if not cap.isOpened():
        print(f"Error: Could not open video {video_path}")
        return None
    
    # Get basic video information
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    fps = cap.get(cv2.CAP_PROP_FPS)
    duration = total_frames / fps  # duration in seconds
    
    cap.release()
    return {
        'total_frames': total_frames,
        'fps': fps,
        'duration': duration
    }

def get_last_frame_number(output_folder):
    # Get all frame files in the output folder
    frame_files = glob.glob(os.path.join(output_folder, "frame_*.jpg"))
    if not frame_files:
        return 0
    
    # Extract numbers from filenames and find the highest
    numbers = []
    for file in frame_files:
        try:
            # Extract number between 'frame_' and '.jpg'
            num = int(os.path.basename(file).replace('frame_', '').replace('.jpg', ''))
            numbers.append(num)
        except ValueError:
            continue
    
    return max(numbers) + 1 if numbers else 0

def extract_frames(video_path, output_folder, frame_rate=1):
    # Get video information first
    video_info = get_video_info(video_path)
    if video_info is None:
        return
    
    # Display video information
    print(f"\nVideo Information:")
    print(f"Total frames: {video_info['total_frames']:,}")
    print(f"Frame rate: {video_info['fps']:.2f} fps")
    print(f"Duration: {video_info['duration']:.2f} seconds ({video_info['duration']/60:.2f} minutes)")
    print(f"Will extract approximately {int(video_info['duration'] * frame_rate)} frames at {frame_rate} fps\n")

    # Create output folder if it doesn't exist
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    # Get the starting frame number
    start_frame = get_last_frame_number(output_folder)
    
    # Open the video file
    cap = cv2.VideoCapture(video_path)
    if not cap.isOpened():
        print(f"Error: Could not open video {video_path}")
        return

    # Get the frame rate of the video
    fps = cap.get(cv2.CAP_PROP_FPS)
    frame_interval = int(fps / frame_rate)

    frame_count = 0
    saved_frame_count = start_frame

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        # Save frame at the specified interval
        if frame_count % frame_interval == 0:
            frame_filename = os.path.join(output_folder, f"frame_{saved_frame_count:04d}.jpg")
            cv2.imwrite(frame_filename, frame)
            saved_frame_count += 1

        frame_count += 1

    cap.release()
    print(f"Extracted {saved_frame_count - start_frame} frames from {video_path}")
    print(f"Frame numbers range from {start_frame:04d} to {saved_frame_count-1:04d}")

# Example usage
video_path = "batch_5/8.mp4"
output_folder = "frames"
extract_frames(video_path, output_folder, frame_rate=0.1)
