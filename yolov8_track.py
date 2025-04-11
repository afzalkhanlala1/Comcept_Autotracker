import socket
import cv2
import numpy as np
from ultralytics import YOLO
import os
import torch
import signal
import sys
import traceback # Import traceback for detailed error printing

# --- Globals ---
server_socket = None
client_socket = None
os.makedirs("debug_frames", exist_ok=True)
frame_counter = 0
last_sent_bbox = None   # (x1, y1, x2, y2)
previous_roi = None     # (x, y, w, h)
IOU_THRESHOLD = 0.1     # Adjust as needed
last_sent_track_id = None

# YOLOv8 model setup
device = "cuda" if torch.cuda.is_available() else "cpu"
print(f"Using device: {device}")
try:
    # Check if the model path exists
    model_path = "TrackerModels/bestm.engine"
    if not os.path.exists(model_path):
        print(f"Error: Model file not found at {model_path}")
        # Attempt to load .pt if .engine fails or doesn't exist
        model_path_pt = "TrackerModels/bestm.pt" # Assuming .pt might exist
        if os.path.exists(model_path_pt):
            print(f"Attempting to load {model_path_pt} instead.")
            model_path = model_path_pt
        else:
            print("Neither .engine nor .pt model found. Exiting.")
            sys.exit(1) # Exit if no model found

    model = YOLO(model_path)
    print(f"Model loaded successfully from {model_path}")
except Exception as e:
    print(f"Error loading YOLO model: {e}")
    print("Please ensure the model file is correct and dependencies (like TensorRT for .engine) are met.")
    sys.exit(1) # Exit if model loading fails
# --- Helper function for IoU ---
def calculate_iou(box1, box2):
    """
    Calculates the Intersection over Union (IoU) between two bounding boxes.
    Boxes are expected in (x1, y1, x2, y2) format.
    """
    x1_inter = max(box1[0], box2[0])
    y1_inter = max(box1[1], box2[1])
    x2_inter = min(box1[2], box2[2])
    y2_inter = min(box1[3], box2[3])

    inter_area = max(0, x2_inter - x1_inter) * max(0, y2_inter - y1_inter)

    box1_area = (box1[2] - box1[0]) * (box1[3] - box1[1])
    box2_area = (box2[2] - box2[0]) * (box2[3] - box2[1])

    union_area = box1_area + box2_area - inter_area

    if union_area <= 0: # Handle zero union area
        return 0.0
    else:
        iou = inter_area / union_area
        # Clamp IoU to [0, 1] just in case of floating point issues
        return max(0.0, min(iou, 1.0))
# --- End Helper ---

# --- Signal Handler ---
def signal_handler(sig, frame):
    print('\nCaught interrupt signal! Shutting down gracefully...')
    global server_socket, client_socket
    if client_socket:
        try: client_socket.close(); print("Client socket closed.")
        except Exception as e: print(f"Error closing client socket: {e}")
    if server_socket:
        try: server_socket.close(); print("Server socket closed.")
        except Exception as e: print(f"Error closing server socket: {e}")
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)
signal.signal(signal.SIGTERM, signal_handler)
# --- End Signal Handler ---


# --- Main Function (mostly unchanged, includes state reset) ---
def main():
    global server_socket, client_socket
    server_socket = None
    client_socket = None

    try:
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.bind(("0.0.0.0", 30555))
        server_socket.listen(1)
        print('Server is listening on port 30555...')

        while True:
            print("Waiting for connection...")
            client_socket, address = server_socket.accept()
            print(f"Connected to - {address}")

            # Reset state for new connection
            global last_sent_bbox, previous_roi, frame_counter
            last_sent_bbox = None
            previous_roi = None
            frame_counter = 0 # Reset frame counter for debugging clarity

            try:
                while True:
                    frame_result = receive_frame(client_socket) # Renamed var for clarity
                    if frame_result is None:
                        print("Client disconnected or error receiving frame.")
                        break
            except socket.error as e:
                 print(f"Socket error during communication: {e}")
                 break
            except Exception as e:
                print(f"Error processing frames: {e}")
                traceback.print_exc()
                break
            finally:
                 if client_socket:
                    try: client_socket.close(); print(f"Client socket with {address} closed.")
                    except Exception as e: print(f"Error closing client socket: {e}")
                 client_socket = None # Reset client socket

    # ... (Keep rest of main and finally block as before) ...
    except OSError as e:
        print(f"Server setup error: {e}")
        if e.errno == 98: print("Port 30555 is likely still in use...")
    except Exception as e:
        print(f"An unexpected server error occurred in main loop: {e}")
        traceback.print_exc()
    finally:
        print("Executing main finally block...")
        if client_socket:
             try: client_socket.close(); print("Fallback: Client socket closed.")
             except Exception as e: print(f"Fallback: Error closing client socket: {e}")
        if server_socket:
             try: server_socket.close(); print("Fallback: Server socket closed.")
             except Exception as e: print(f"Fallback: Error closing server socket: {e}")
        print("Server shutdown complete.")


# --- receive_frame function (REVISED) ---
def receive_frame(client_socket):
    global frame_counter, last_sent_bbox, previous_roi, last_sent_track_id

    # Buffer initialization and marker definitions (unchanged)
    PN_01, PN_02, PN_03 = 1128889, 2269733, 3042161
    sof, soi, eof = b"~!SOF" + str(PN_01).encode() + b",", b"|~IMS" + str(PN_02).encode(), b"#EMI!" + str(PN_03).encode()
    buffer = b''
    MAX_BUFFER = 60 * 1024 * 1024
    max_keep_len = len(sof) + len(soi) + len(eof) + 100

    while True:
        try:
            data_chunk = client_socket.recv(4096 * 16)
            if not data_chunk:
                return None  # Connection closed
            buffer += data_chunk

            while True:  # Process buffer
                sof_index = buffer.find(sof)
                if sof_index == -1:
                    if len(buffer) > max_keep_len:
                        buffer = buffer[-max_keep_len:]
                    break  # Need more data

                eof_index = buffer.find(eof, sof_index + len(sof))
                if eof_index == -1:
                    if len(buffer) > MAX_BUFFER:
                        print(f"Warning: Buffer exceeded {MAX_BUFFER} bytes...")
                        last_sof = buffer.rfind(sof)
                        buffer = buffer[last_sof:] if last_sof != -1 else buffer[-MAX_BUFFER:]
                    break  # Need more data

                # Start Frame Processing
                frame_segment = buffer[sof_index : eof_index + len(eof)]
                image_start_index_rel = frame_segment.find(soi)
                if image_start_index_rel != -1:
                    image_start_index_abs = sof_index + image_start_index_rel
                    frame_metadata = buffer[sof_index : image_start_index_abs + len(soi)]
                    frame_headers = frame_metadata.split(b',')

                    if len(frame_headers) >= 8:
                        try:
                            # Parse Headers
                            width = int(frame_headers[2])
                            height = int(frame_headers[3])
                            roi_x, roi_y, roi_w, roi_h = map(int, frame_headers[4:8])
                            current_roi = None if roi_x == -1 else (roi_x, roi_y, roi_w, roi_h)

                            # State Logic
                            is_new_target = (current_roi != previous_roi)
                            if is_new_target:
                                print(f"ROI changed: New={current_roi}, Prev={previous_roi}. Resetting tracking.")
                                last_sent_track_id = None  # Reset tracking for new target

                            # Decode Image
                            image_data = buffer[image_start_index_abs + len(soi) : eof_index]
                            img_arr = np.frombuffer(image_data, dtype=np.uint8)
                            frame = cv2.imdecode(img_arr, cv2.IMREAD_COLOR)

                            if frame is None:
                                print("Failed to decode frame.")
                                buffer = buffer[eof_index + len(eof):]
                                continue

                            print(f"Processing frame {frame_counter} - ROI: {current_roi}, NewTarget: {is_new_target}, LastTrackID: {last_sent_track_id}")

                            # Run YOLO Tracking
                            result = model.track(frame, persist=True, device=device, verbose=False)
                            bbs = []  # All valid detections
                            if result and hasattr(result[0], 'boxes') and result[0].boxes is not None and result[0].boxes.id is not None:
                                valid_boxes = result[0].boxes[(result[0].boxes.conf > 0.5)]
                                for box in valid_boxes:
                                    bbox_coords = box.xyxy.cpu().numpy()[0]  # (x1, y1, x2, y2)
                                    confidence, class_id, track_id = box.conf.item(), int(box.cls.item()), int(box.id.item())
                                    bbs.append(((float(bbox_coords[0]), float(bbox_coords[1]), float(bbox_coords[2]), float(bbox_coords[3])), confidence, class_id, track_id))
                            print(f"YOLO found {len(bbs)} potential detections.")

                            # Filtering Logic
                            detection_to_send = None
                            closest_detection_data = None

                            # Find closest detection if ROI is valid (for new targets)
                            if current_roi is not None and is_new_target:
                                roi_x_coord, roi_y_coord, roi_w_coord, roi_h_coord = current_roi
                                if roi_w_coord > 0 and roi_h_coord > 0:
                                    roi_cx = roi_x_coord + roi_w_coord / 2
                                    roi_cy = roi_y_coord + roi_h_coord / 2
                                    min_distance_sq = float('inf')

                                    for detection in bbs:
                                        box_coords, confidence, class_id, track_id = detection
                                        x1, y1, x2, y2 = box_coords
                                        w = x2 - x1
                                        h = y2 - y1
                                        if w > 0 and h > 0:
                                            bbox_cx, bbox_cy = x1 + w / 2, y1 + h / 2
                                            distance_sq = (bbox_cx - roi_cx) ** 2 + (bbox_cy - roi_cy) ** 2
                                            if distance_sq < min_distance_sq:
                                                min_distance_sq = distance_sq
                                                closest_detection_data = detection

                            # Decide what to send
                            if is_new_target and closest_detection_data:
                                detection_to_send = closest_detection_data
                                last_sent_track_id = detection_to_send[3]  # Update track ID
                                print(f"New target: Sending ID {last_sent_track_id}")
                            elif last_sent_track_id is not None:
                                # Look for the same track ID
                                for detection in bbs:
                                    if detection[3] == last_sent_track_id:
                                        detection_to_send = detection
                                        print(f"Continuing track: Sending ID {last_sent_track_id}")
                                        break
                                if detection_to_send is None:
                                    print(f"Track ID {last_sent_track_id} not found. Sending NULL")

                            # Update last_sent_bbox
                            if detection_to_send:
                                last_sent_bbox = detection_to_send[0]  # bbox coordinates
                            # Do not clear last_sent_bbox if no detection is sent

                            # Update previous_roi
                            if previous_roi != current_roi:
                                print(f"Updating previous_roi from {previous_roi} to {current_roi}\n")
                            previous_roi = current_roi

                            # Prepare final list for sending
                            final_bbs_to_send = [detection_to_send] if detection_to_send else []

                            # Debug Frame Drawing (unchanged)
                            output_frame = frame.copy()
                            for detection in bbs:
                                box, confidence, class_id, track_id = detection
                                x1, y1, x2, y2 = box
                                label = f"ID:{track_id} C:{confidence:.2f}"
                                cv2.rectangle(output_frame, (int(x1), int(y1)), (int(x2), int(y2)), (255, 120, 0), 1)
                                cv2.putText(output_frame, label, (int(x1), int(y1) - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 120, 0), 1)
                            if detection_to_send:
                                box, confidence, class_id, track_id = detection_to_send
                                x1, y1, x2, y2 = box
                                label = f"SENT ID:{track_id} C:{confidence:.2f}"
                                cv2.rectangle(output_frame, (int(x1), int(y1)), (int(x2), int(y2)), (0, 255, 0), 2)
                                cv2.putText(output_frame, label, (int(x1), int(y1) - 25), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
                            if current_roi:
                                rx, ry, rw, rh = current_roi
                                cv2.rectangle(output_frame, (rx, ry), (rx + rw, ry + rh), (0, 0, 255), 1)
                            if last_sent_bbox:
                                lsx1, lsy1, lsx2, lsy2 = last_sent_bbox
                                cv2.rectangle(output_frame, (int(lsx1), int(lsy1)), (int(lsx2), int(lsy2)), (255, 0, 255), 1)
                            cv2.imwrite(f"debug_frames/output_{frame_counter}.jpg", output_frame)

                            print(f"Frame {frame_counter}: Sent {len(final_bbs_to_send)} detection(s).\n---\n")
                            frame_counter += 1

                            # Send the result
                            send_detections(client_socket, final_bbs_to_send)

                            # Cleanup and Continue
                            buffer = buffer[eof_index + len(eof):]
                            return True

                        except ValueError as e:
                            print(f"Header parsing error: {e}. Discarding.")
                            buffer = buffer[eof_index + len(eof):]
                            continue
                        except cv2.error as e:
                            print(f"OpenCV error: {e}. Discarding.")
                            buffer = buffer[eof_index + len(eof):]
                            continue
                        except Exception as e:
                            print(f"Unexpected error processing frame data: {e}.")
                            traceback.print_exc()
                            buffer = buffer[eof_index + len(eof):]
                            continue
                    else:
                        print(f"Incomplete headers ({len(frame_headers)}). Discarding.")
                        buffer = buffer[eof_index + len(eof):]
                        continue
                else:
                    print("SOI missing. Discarding.")
                    buffer = buffer[eof_index + len(eof):]
                    continue

        except socket.error as e:
            print(f"Socket error receiving data: {e}")
            return None
        except Exception as e:
            print(f"General error in receive_frame loop: {e}")
            traceback.print_exc()
            return None

# --- send_detections function (Unchanged from previous version) ---
def send_detections(client_socket, bbs):
    if not bbs:
        message = b'NULL'
        print("Sending: NULL")
    else:
        # Expects bbs[0] = ((x1, y1, x2, y2), confidence, class_id, track_id)
        box_coords, confidence, class_id, track_id = bbs[0]
        x1, y1, x2, y2 = box_coords
        line = f"{class_id},{confidence:.2f},{x1:.2f},{y1:.2f},{x2:.2f},{y2:.2f},{track_id}\n"
        message = line.encode()
        print(f"Sending: {line.strip()}")

    try:
        client_socket.sendall(message)
    except socket.error as e:
        print(f"Socket error sending detections: {e}")
        raise

# --- Main execution ---
if __name__ == "__main__":
    main()
