# OpenCV-Qt-App
GUI implementation for computer vision apps.

**Reference**: [*Qt 5 and OpenCV 4 Computer Vision Projects*](https://github.com/PacktPublishing/Qt-5-and-OpenCV-4-Computer-Vision-Projects/tree/master) by Zhuo Qingliang. What an awesome book!

**Device**: MacBook Pro 16 inch, Late 2021 (M1 Pro)  
**Operating System**: macOS Ventura 13.0

## Core Concepts

| Chapter | Topic                                          | Description                                                                                      | Key Ideas                                                                                                  |
|---------|------------------------------------------------|--------------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------|
| 1       | [Image Viewer](./01_ImageViewer)               | A simple image viewer allowing users to open and save images.                                    | `qmake`, Scene vs. View, Slot-Signal-Action System, Loading images, Directory navigation                  |
| 2       | [Image Editor](./02_ImageEditor)               | A simple image editor allowing users to blur, erode, sharpen, add cartoon effects, rotate, etc.   | Linking OpenCV to Qt, Image processing, Qt Plugin Setup                                                    |
| 3       | [Motion Detection](./03_MotionDetection/)      | A simple motion detection app for detecting motion in a video stream.                            | Video processing, Motion detection, Background subtraction, Qt Layout System, Accessing webcam, Multi-threading |
| 4       | [Face Detection](./04_FaceDetection/)          | A simple face detection app for detecting faces and facial landmarks with face filters.           | Face detection, Facial landmark detection, Haar Cascade, Qt Resource System, Adding Face Filters, `QtCheckBox`|
| 5       | [Optical Character Recognition](./05_OpticalCharacterRecognition/) | An OCR app to detect text in images of text or scenes.                                            | OCR with Tesseract, Efficient and Accurate Scene Text (EAST) Detection                                     |
| 6       | [Object Detection](./06_ObjectDetection/)      | A simple object detection app for detecting objects in a video stream.                            | Object detection, YOLOv3, OpenCV DNN                                                                        |
| 7       | [Car Distance](./07_CarDistance/)              | An app to detect cars and calculate the distance between them and from the camera.                | Car detection, Distance calculation                                                                         |
