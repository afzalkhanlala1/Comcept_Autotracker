# README_AI_AGENT

## PROJECT_METADATA
*   **Project Name:** AutoTracker-V7 (Inferred)
*   **Primary Language:** C++
*   **Framework:** Qt (Version 5 or 6, QML for UI)
*   **Core Functionality:** Real-time object tracking from video input using computer vision algorithms and deep learning models.
*   **Target Platform:** NVIDIA Jetson Orin AGX (JetPack 6 assumed based on user context).
*   **Deep Learning Inference:** On-device via ONNX Runtime (TensorRT Execution Provider) for YOLOv8.
*   **External Python Server:** **DEPRECATED**. Object detection is now handled internally by `YoloDetector`. `yolov8_track.py` and `yoloclienttracker.h/.cpp` are no longer primary components for detection.

## ARCHITECTURE_OVERVIEW
1.  **Main Controller:** `AutoTrackerController` orchestrates the application, manages threads, handles UI painting (via QQuickPaintedItem), receives configuration commands, and implements PID control logic based on tracking results.
2.  **Video Input:** `OpenCVVideoCapture` manages video sources (camera/file) using OpenCV. It performs initial frame acquisition and potentially cropping/scaling. It contains `OpenCVVideoDevice` instances for specific sources.
3.  **Image Processing Pipeline:** Frames flow from `OpenCVVideoCapture` through:
    *   `ImageEnhancer`: Applies visual enhancements (LUT via `ColorTransfer`, CLAHE, Noise Reduction).
    *   `ImageStabilization`: Optional frame stabilization using optical flow (`KalmanOpticalFlowStab`) and/or IMU data (`IMUStab`).
4.  **Tracking Core:** `ObjectTracker` manages multiple tracking algorithms. It receives the `TrackerFrame` (containing original, enhanced, stabilized frames) and distributes tasks to specific tracker implementations. It's intended to fuse results, although the current fusion logic might be basic or specific to certain tracker combinations.
5.  **Trackers:** Instantiated and managed by `ObjectTracker`:
    *   `DaSiamRPNTracker`: Implements DaSiamRPN tracking (likely via OpenCV Contrib). Processes enhanced/stabilized frames.
    *   `TemplateMatchTracker`: Implements template matching. Processes enhanced/stabilized frames.
    *   `YoloDetector`: **(Current Implementation)** Performs YOLOv8 object detection *locally* using ONNX Runtime. **Crucially, it processes the original, unenhanced frame (`TrackerFrame::frame`)**. Replaces the previous client-server approach.
6.  **Communication Modules:**
    *   `RemoteControl`: Handles TCP server/sockets for receiving commands and sending telemetry/settings to a remote GUI/controller.
    *   `CoProcessor`: Handles serial communication (likely `/dev/ttyUSB0`) with external hardware (e.g., gimbal controller), sending PID outputs and receiving sensor data (IMU, RF Remote).
    *   `DataDevice`: Appears to be a generic class for TCP/Serial communication, possibly used for telemetry or other data links.
    *   `FrameTransmitter`: Sends processed frames (likely `lutFrame` or `stabImage`) over TCP to a separate client/viewer (distinct from `RemoteControl`).
7.  **UI:** Defined in QML (`main.qml`), interacts with `AutoTrackerController`.
8.  **Threading:** Major components (`OpenCVVideoCapture`, `ImageEnhancer`, `ImageStabilization`, `ObjectTracker`, `YoloDetector`, `RemoteControl`, `CoProcessor`, etc.) run in separate `QThread` instances managed by `AutoTrackerController`. Communication is primarily via Qt Signals/Slots.

## KEY_COMPONENTS (File Path: Role)
*   `main.cpp`: Application entry point, Qt/QML setup, Metatype registration.
*   `autotrackercontroller.h/.cpp`: Central coordinator, UI paint, state management, PID logic, thread management, top-level signal/slot connections.
*   `opencvvideocapture.h/.cpp`: Video input management. Uses `opencvvideodevice.h/.cpp`.
*   `imageenhancer.h/.cpp`: Image enhancement pipeline step. Uses `colortransfer.h/.cpp`.
*   `imagestabilization.h/.cpp`: Image stabilization pipeline step. Uses `kalmanopticalflowstab.h/.cpp`, `imustab.h/.cpp`.
*   `objecttracker.h/.cpp`: Manages tracker instances (`DaSiamRPNTracker`, `TemplateMatchTracker`, `YoloDetector`), receives `TrackerFrame`, distributes work, intended fusion point.
*   `dasiamrpntracker.h/.cpp`: DaSiamRPN tracker implementation.
*   `templatematchtracker.h/.cpp`: Template Matching tracker implementation.
*   `yolodetector.h/.cpp`: **(Primary Detection)** Local YOLOv8 inference via ONNX Runtime. Handles preprocessing, inference, postprocessing (NMS). Processes `TrackerFrame::frame`.
*   `remotecontrol.h/.cpp`: TCP communication for remote control/telemetry. Parses incoming commands.
*   `coprocessor.h/.cpp`: Serial communication interface for hardware control.
*   `datadevice.h/.cpp`: Generic TCP/Serial communication helper.
*   `frametransmitter.h/.cpp`: TCP frame sender for external viewer.
*   `autotracker_types.h`: Defines key data structures: `TrackerFrame`, `YoloResult`, `PacketData`, Enums (`RFREMOTE`, `USERPANEL`, etc.).
*   `config.h`: Defines constants (Ports, IPs, PID defaults, `TRACKER_TYPE` enum). Some may be outdated.
*   `pid.h/.cpp`: PID controller implementation.
*   `*.qml`: QML files defining the user interface.
*   `CMakeLists.txt`: Project build configuration.
*   `yoloclienttracker.h/.cpp`: **(DEPRECATED)** Code for communicating with the external Python server.
*   `yolov8_track.py`: **(DEPRECATED)** External Python YOLO server script.

## DATA_STRUCTURES_REFERENCE
*   `TrackerFrame` (`autotracker_types.h`): Holds multiple versions of the frame (`frame`, `lutFrame`, `stabImage`), plus metadata (`dt`, `stab_win`, etc.). Key for understanding data flow to trackers.
*   `YoloResult` (`autotracker_types.h`): Standardized output for YOLO detections (bbox, confidence, label, labelID). Generated by `YoloDetector`.
*   `PacketData` (`PacketData.h`): Used for parsing structured data from serial/TCP (`CoProcessor`, `RemoteControl`, `DataDevice`). Contains header, module enum, values.
*   `Enums`: Several enums define constants for tracker types (`TRACKER_TYPE` in `config.h`), input devices (`CAP_DEV` in `opencvvideocapture.h`), remote controls (`RFREMOTE`, `USERPANEL` in `autotracker_types.h`), etc.

## BUILD_AND_DEPENDENCIES
*   **Build System:** CMake (`CMakeLists.txt`).
*   **Core Dependencies:**
    *   Qt (>= 5.15, likely compatible with 6.x). Modules: Core, Gui, Qml, Quick, Network, SerialPort, Multimedia.
    *   OpenCV (Version compatible with JetPack 6, requires `core`, `imgproc`, `video`, `tracking`, `dnn`, potentially `cuda*` modules).
    *   ONNX Runtime (C++ API, **must be built/installed with TensorRT Execution Provider enabled** for JetPack 6 / Orin AGX).
    *   TensorRT (Installed via JetPack 6, used by ONNX Runtime).
    *   CUDA Toolkit (Installed via JetPack 6).
*   **Platform:** NVIDIA Jetson Orin AGX with JetPack 6.
*   **Build Process:** Standard CMake flow (e.g., `mkdir build && cd build && cmake .. && make -jN`). Linker requires paths to Qt, OpenCV, ONNX Runtime.

## CONFIGURATION
*   **Model Path:** YOLO model path is specified during `YoloDetector` initialization (currently hardcoded example in `ObjectTracker::init` pointing to `TrackerModels/bestm.onnx`). Requires modification if path differs.
*   **Device Paths/Ports:** Serial port (`CP_SERIALPATH` in `config.h` / `CoProcessor::init`), TCP ports (`IMAGE_PORT`, `RC_IMAGE_PORT`, `RC_DATA_PORT`, etc. in `config.h`), IP addresses are often hardcoded in relevant files (`config.h`, `remotecontrol.cpp`, `frametransmitter.cpp`).
*   **Tracking Parameters:** PID gains (`config.h` defaults, settable via `RemoteControl`), YOLO thresholds (`confThreshold_`, `iouThreshold_` in `yolodetector.cpp`), tracker-specific parameters may exist.
*   **Remote Settings:** `RemoteControl` can receive commands to modify some settings (`setPID*`, `setLut*`, etc. See `RemoteControl::invokeUserCmd`).

## EXECUTION_FLOW_SUMMARY
1.  `main.cpp`: Starts `QApplication`, registers types, loads `main.qml`.
2.  QML instantiates `AutoTrackerController`.
3.  `AutoTrackerController::init()`:
    *   Creates component instances (`OpenCVVideoCapture`, `ImageEnhancer`, `ImageStabilization`, `ObjectTracker`, `YoloDetector`, communication modules).
    *   Moves components to their respective `QThread`s.
    *   Establishes signal/slot connections between components.
    *   Emits initialization signals (e.g., `initYoloDetector`).
4.  `OpenCVVideoCapture` starts capturing frames.
5.  Frames (`TrackerFrame` struct containing original frame) flow: `OpenCVVideoCapture` -> `ImageEnhancer` -> `ImageStabilization` -> `AutoTrackerController` -> `ObjectTracker`.
6.  `ObjectTracker::trackImageFeatures`: Receives `TrackerFrame`, emits `trackFeatures` signal *with the full `TrackerFrame`*.
7.  Signal `trackFeatures` triggers slots in individual trackers:
    *   `YoloDetector::processFrame`: Extracts `trackerFrame.frame`, preprocesses, runs ONNX Runtime inference, postprocesses, emits `detectionsReady(QVector<YoloResult>)`.
    *   `DaSiamRPNTracker::track`: Extracts processed frame (e.g., `stabImage`), runs tracking.
    *   `TemplateMatchTracker::track`: Extracts processed frame, runs tracking.
8.  `ObjectTracker` receives tracker results (`trackerFinished` for DaSiam/TM, `handleLocalDetections` for YOLO). Potentially fuses results (logic needs verification) and updates `objectTrackedRect`. Emits `imageTracked`.
9.  `AutoTrackerController::imageTracked`: Receives final tracking rect, calculates PID error (`trackingXErr`, `trackingYErr`), computes PID output (`trackingPIDxVal`, `trackingPIDyVal`).
10. `AutoTrackerController` sends PID output to `CoProcessor`.
11. `AutoTrackerController` updates UI (`paint` method).
12. Communication modules (`RemoteControl`, `FrameTransmitter`, `CoProcessor`, `DataDevice`) handle external IO concurrently.

## AGENT_NOTES
*   **Focus Areas:** `AutoTrackerController`, `ObjectTracker`, `YoloDetector` are central to the tracking logic. `OpenCVVideoCapture`, `ImageEnhancer`, `ImageStabilization` define the input pipeline.
*   **YOLO Implementation:** The system uses **local inference** via `YoloDetector` (ONNX Runtime + TensorRT). `YoloClientTracker` and `yolov8_track.py` are **DEPRECATED** and should be ignored for current detection logic.
*   **Frame Access:** `TrackerFrame` is the key data structure passed between pipeline stages. `YoloDetector` specifically accesses `trackerFrame.frame` for inference on the original image. Other trackers likely use `trackerFrame.lutFrame` or `trackerFrame.stabImage`.
*   **Configuration:** Critical paths (models) and network settings (ports, IPs) are often hardcoded. Identify and modify these directly in the source code as needed.
*   **Threading Model:** Be aware of the multi-threaded architecture. Interactions between major components occur via Qt Signals/Slots, many configured as `Qt::QueuedConnection` due to cross-thread communication. Use `QMutexLocker` where shared data access occurs outside of signals/slots (e.g., `ObjectTracker::trackerFinished`).
*   **Fusion Logic:** The tracker fusion logic within `ObjectTracker::combineTrackers` and `ObjectTracker::selectTrackedRect` might require careful analysis, especially regarding how results from the local `YoloDetector` are now integrated.