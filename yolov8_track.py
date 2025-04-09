import socket
import cv2
import numpy as np
from ultralytics import YOLO
import os
import torch
import signal # Import signal module for cleaner shutdown
import sys   # Import sys module for exit

# --- Globals ---
server_socket = None
client_socket = None
# Create output directory for debug images
os.makedirs("debug_frames", exist_ok=True)
frame_counter = 0

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


# --- Signal Handler for Graceful Shutdown ---
def signal_handler(sig, frame):
    print('\nCaught interrupt signal! Shutting down gracefully...')
    global server_socket, client_socket
    if client_socket:
        try:
            client_socket.close()
            print("Client socket closed.")
        except Exception as e:
            print(f"Error closing client socket: {e}")
    if server_socket:
        try:
            server_socket.close()
            print("Server socket closed.")
        except Exception as e:
            print(f"Error closing server socket: {e}")
    sys.exit(0)

# Register signal handlers for SIGINT (Ctrl+C) and SIGTERM
signal.signal(signal.SIGINT, signal_handler)
signal.signal(signal.SIGTERM, signal_handler)

# --- Main Function ---
def main():
    global server_socket, client_socket # Make them global to be accessible in signal_handler
    server_socket = None # Initialize
    client_socket = None # Initialize

    try:
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        # ===>>> ADD THIS LINE <<<===
        # Set option to reuse the address immediately
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        # ===========================

        server_socket.bind(("0.0.0.0", 30555))
        server_socket.listen(1)

        print('Server is listening on port 30555...')

        # Accept connections within the loop for potential reconnects (though current code exits)
        while True:
            print("Waiting for connection...")
            client_socket, address = server_socket.accept()
            print(f"Connected to - {address}")

            try:
                while True: # Inner loop to handle frames from the connected client
                    frame = receive_frame(client_socket)
                    if frame is None:
                        print("Client disconnected or error receiving frame.")
                        break # Exit inner loop, wait for new connection
            except socket.error as e:
                 print(f"Socket error during communication: {e}")
                 # Potentially log the error, then break to wait for a new connection
                 break
            except Exception as e:
                print(f"Error processing frames: {e}")
                # Decide if this error is fatal or if we can continue/wait for new connection
                break # Exit inner loop on general processing errors
            finally:
                 if client_socket:
                    try:
                        client_socket.close()
                        print(f"Client socket with {address} closed.")
                        client_socket = None # Reset client socket
                    except Exception as e:
                        print(f"Error closing client socket: {e}")
            # After client disconnects or error, loop back to server_socket.accept()


    except OSError as e:
        print(f"Server setup error: {e}")
        # Specific check for Address already in use, though SO_REUSEADDR should prevent it
        if e.errno == 98:
             print("Port 30555 is likely still in use. If the problem persists, check if another process is using it (e.g., using 'sudo netstat -tulnp | grep 30555').")
    except Exception as e:
        print(f"An unexpected server error occurred in main loop: {e}")
    finally:
        # This cleanup now primarily happens in the signal handler for Ctrl+C
        # But we keep it here as a fallback for other exit paths
        print("Executing main finally block...")
        if client_socket:
            try:
                client_socket.close()
                print("Fallback: Client socket closed.")
            except Exception as e:
                print(f"Fallback: Error closing client socket: {e}")
        if server_socket:
            try:
                server_socket.close()
                print("Fallback: Server socket closed.")
            except Exception as e:
                print(f"Fallback: Error closing server socket: {e}")
        print("Server shutdown complete.")


# --- receive_frame function (minor refinements) ---
def receive_frame(client_socket):
    global frame_counter
    frame_data = b''
    # Removed static indexes, they are calculated in the loop

    # Define constants clearly
    PN_01 = 1128889
    PN_02 = 2269733
    PN_03 = 3042161

    # Construct markers once
    sof = b"~!SOF" + str(PN_01).encode() + b","
    soi = b"|~IMS" + str(PN_02).encode()
    eof = b"#EMI!" + str(PN_03).encode()

    # Buffer to accumulate data
    buffer = b''

    while True:
        try:
            # Receive data in chunks
            data_chunk = client_socket.recv(4096 * 16) # Increased chunk size slightly
            if not data_chunk:
                print("Connection closed by client.")
                return None # Signal connection closed
            buffer += data_chunk

            # --- Frame Extraction Logic ---
            while True: # Process buffer for potentially multiple frames
                sof_index = buffer.find(sof)
                if sof_index == -1:
                    # If no SOF found, keep the last part of the buffer
                    # in case SOF is split across chunks. Avoid infinite buffer growth.
                    max_keep_len = len(sof) + len(soi) + len(eof) + 100 # Keep enough for markers + headers
                    if len(buffer) > max_keep_len:
                        # print(f"Discarding {len(buffer) - max_keep_len} bytes (no SOF)")
                        buffer = buffer[-max_keep_len:]
                    break # Need more data

                eof_index = buffer.find(eof, sof_index + len(sof))
                if eof_index == -1:
                    # Found SOF but not EOF yet, need more data
                    # If buffer gets excessively large without EOF, maybe discard old data?
                    # Example: Limit buffer size to prevent memory issues
                    MAX_BUFFER = 60 * 1024 * 1024 # 60MB limit (adjust as needed)
                    if len(buffer) > MAX_BUFFER:
                         print(f"Warning: Buffer exceeded {MAX_BUFFER} bytes without finding EOF. Discarding oldest data.")
                         # Find the last potential SOF before discarding
                         last_sof = buffer.rfind(sof)
                         if last_sof != -1:
                              buffer = buffer[last_sof:]
                         else: # Should not happen if sof_index != -1, but as failsafe
                              buffer = buffer[-MAX_BUFFER:] # Keep the newest part
                    break # Need more data


                # --- Found a potential frame (SOF to EOF) ---
                frame_segment = buffer[sof_index : eof_index + len(eof)]

                # Check for image start marker within the segment
                image_start_index_rel = frame_segment.find(soi) # Relative index within segment
                if image_start_index_rel != -1:
                    image_start_index_abs = sof_index + image_start_index_rel # Absolute index in buffer

                    # Extract metadata (SOF to SOI)
                    frame_metadata = buffer[sof_index : image_start_index_abs + len(soi)]
                    frame_headers = frame_metadata.split(b',')

                    # Validate headers before parsing
                    if len(frame_headers) >= 8:
                        try:
                            # Attempt to parse headers
                            # image_size = int(frame_headers[1]) # Image size header - useful for verification?
                            width = int(frame_headers[2])
                            height = int(frame_headers[3])
                            roi_x = int(frame_headers[4])
                            roi_y = int(frame_headers[5])
                            roi_w = int(frame_headers[6])
                            roi_h = int(frame_headers[7])

                            # Extract image data (SOI to EOF)
                            image_data = buffer[image_start_index_abs + len(soi) : eof_index]

                            # --- Verification (Optional but Recommended) ---
                            # if len(image_data) != image_size:
                            #    print(f"Warning: Header image size ({image_size}) does not match received data size ({len(image_data)}).")
                                # Decide how to handle: process anyway, discard, etc.

                            # --- Decode and Process ---
                            img_arr = np.frombuffer(image_data, dtype=np.uint8)
                            frame = cv2.imdecode(img_arr, cv2.IMREAD_COLOR)

                            if frame is None:
                                print("Failed to decode frame, possible corruption.")
                                # Don't return None, just discard this invalid frame
                            else:
                                # Successfully decoded frame
                                print(f"Received frame {frame_counter} - Size: {frame.shape[1]}x{frame.shape[0]}")

                                # Determine ROI
                                roi = None if roi_x == -1 else (roi_x, roi_y, roi_w, roi_h)

                                # Process the frame (moved logic here)
                                # cv2.imwrite(f"debug_frames/input_{frame_counter}.jpg", frame) # Optional debug save
                                result = model.track(frame, persist=True, device=device, verbose=False) # Added verbose=False
                                bbs = []

                                if result and hasattr(result[0], 'boxes') and result[0].boxes is not None and result[0].boxes.id is not None:
                                    # Combine filtering conditions
                                    valid_boxes = result[0].boxes[(result[0].boxes.conf > 0.5)]
                                    for box in valid_boxes:
                                        bbox = box.xyxy.cpu().numpy()[0]
                                        confidence = box.conf.item()
                                        class_id = int(box.cls.item())
                                        track_id = int(box.id.item())
                                        x1, y1, x2, y2 = bbox
                                        bbs.append(([float(x1), float(y1), float(x2 - x1), float(y2 - y1)], confidence, class_id, track_id))


                                # Filter detections based on ROI
                                filtered_bbs = []
                                if roi is not None:
                                    roi_x_coord, roi_y_coord, roi_w_coord, roi_h_coord = roi
                                    # Check if ROI dimensions are valid
                                    if roi_w_coord > 0 and roi_h_coord > 0:
                                        roi_cx = roi_x_coord + roi_w_coord / 2
                                        roi_cy = roi_y_coord + roi_h_coord / 2
                                        closest_detection = None
                                        min_distance_sq = float('inf') # Use squared distance to avoid sqrt

                                        for detection in bbs:
                                            box, confidence, class_id, track_id = detection
                                            x, y, w, h = box
                                            # Ensure box dimensions are valid
                                            if w > 0 and h > 0:
                                                bbox_cx = x + w / 2
                                                bbox_cy = y + h / 2
                                                distance_sq = (bbox_cx - roi_cx) ** 2 + (bbox_cy - roi_cy) ** 2
                                                if distance_sq < min_distance_sq:
                                                    min_distance_sq = distance_sq
                                                    closest_detection = detection
                                        
                                        if closest_detection:
                                            filtered_bbs = [closest_detection]
                                        # else: No detection found close to ROI

                                    else:
                                        print(f"Warning: Received invalid ROI dimensions W:{roi_w_coord} H:{roi_h_coord}. Sending no detections.")
                                        # filtered_bbs remains empty

                                else:
                                    # No ROI provided: send nothing as per original logic
                                    # If you wanted to send *all* detections when no ROI, you'd do:
                                    # filtered_bbs = bbs
                                    pass # filtered_bbs remains empty

                                # Save output frame for debugging
                                output_frame = frame.copy() # Work on a copy
                                for detection in filtered_bbs:
                                    box, confidence, class_id, track_id = detection
                                    x, y, w, h = box
                                    label = f"ID:{track_id} C:{confidence:.2f}"
                                    cv2.rectangle(output_frame, (int(x), int(y)), (int(x + w), int(y + h)), (0, 255, 0), 2)
                                    cv2.putText(output_frame, label, (int(x), int(y) - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 0, 0), 2)

                                # Also draw the ROI if it was provided
                                if roi is not None:
                                    rx, ry, rw, rh = roi
                                    cv2.rectangle(output_frame, (rx, ry), (rx + rw, ry + rh), (0, 0, 255), 1) # Red ROI box

                                cv2.imwrite(f"debug_frames/output_{frame_counter}.jpg", output_frame)
                                print(f"Processed frame {frame_counter} - Found: {len(bbs)} detections, Sent: {len(filtered_bbs)} detections.")
                                frame_counter += 1

                                # Send the *filtered* detections
                                send_detections(client_socket, filtered_bbs)

                                # Remove the processed frame from the buffer
                                buffer = buffer[eof_index + len(eof):]
                                # --- Successfully processed a frame ---
                                continue # Go back to check buffer for more frames

                        except ValueError as e:
                            print(f"Header parsing error: {e}. Discarding segment.")
                            # Invalid headers, remove the problematic segment
                            buffer = buffer[eof_index + len(eof):]
                            continue # Check buffer again
                        except cv2.error as e:
                             print(f"OpenCV error during decoding/processing: {e}. Discarding segment.")
                             buffer = buffer[eof_index + len(eof):]
                             continue # Check buffer again
                        except Exception as e:
                            print(f"Unexpected error processing frame data: {e}. Discarding segment.")
                            # Log traceback here if needed: import traceback; traceback.print_exc()
                            buffer = buffer[eof_index + len(eof):]
                            continue # Check buffer again
                    else:
                        # Headers are incomplete, likely a logic error or bad data
                        print(f"Incomplete headers received (got {len(frame_headers)}, expected >= 8). Discarding segment.")
                        buffer = buffer[eof_index + len(eof):]
                        continue # Check buffer again
                else:
                    # Found SOF and EOF, but SOI wasn't between them
                    print("Image start marker (SOI) not found between SOF and EOF. Discarding segment.")
                    buffer = buffer[eof_index + len(eof):]
                    continue # Check buffer again

            # --- End of inner buffer processing loop ---

        except socket.error as e:
            print(f"Socket error receiving data: {e}")
            return None # Signal error/disconnect
        except Exception as e:
            print(f"General error in receive_frame loop: {e}")
            # import traceback; traceback.print_exc() # Uncomment for detailed debug
            return None # Signal error

# --- send_detections function ---
def send_detections(client_socket, bbs):
    if not bbs:
        message = b'NULL'
        print("Sending: NULL")
    else:
        # Original logic sends only the first (closest) detection if ROI was used
        box, confidence, class_id, track_id = bbs[0]
        x, y, w, h = box
        # Format with 2 decimal places for floats
        line = f"{class_id},{confidence:.2f},{x:.2f},{y:.2f},{x + w:.2f},{y + h:.2f},{track_id}\n"
        message = line.encode()
        print(f"Sending: {line.strip()}")

    try:
        client_socket.sendall(message)
    except socket.error as e:
        print(f"Socket error sending detections: {e}")
        # Calling function should handle this (e.g., by returning None from receive_frame)
        raise # Re-raise the socket error to be caught in receive_frame

# --- spilling function (no longer needed with new recv logic) ---
# def spilling(data_chunk, client_socket):
#     if not data_chunk:
#         return False
#     return True

# --- Main execution ---
if __name__ == "__main__":
    main()
