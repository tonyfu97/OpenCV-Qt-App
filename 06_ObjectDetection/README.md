# Real-Time Object Detection with Qt and OpenCV: Chapter 06 Reflections

**Author**: Tony Fu  
**Date**: August 12, 2023  
**Device**: MacBook Pro 16-inch, Late 2021 (M1 Pro)  

**Reference**: Chapter 6 of [*Qt 5 and OpenCV 4 Computer Vision Projects*](https://github.com/PacktPublishing/Qt-5-and-OpenCV-4-Computer-Vision-Projects/tree/master) by Zhuo Qingliang

## Core Concepts

### 1. Issues Encountered & Solutions

- **Camera Privacy Error on Startup** (Repeated from Chapter 3):
  - Problem: App crash with an error about accessing privacy-sensitive data.
  - Solution: Added the following to the `Info.plist` file:
    ```xml
    <key>NSCameraUsageDescription</key>
    <string>We need access to the camera to capture video for motion detection.</string>
    ```

- **Failure to Build OpenCV 3.4.5**: This step builds the executables: `opencv_createsamples` and `opencv_traincascade`, which are needed to train the Haar Cascade classifier for no-entry sign detection. I first downloaded OpenCV 3.4.5 from this [link](https://github.com/opencv/opencv/releases/tag/3.4.5). 

    Then, I created a `build` directory.

    Inside the directory, I ran the following command:
    ```bash
    cmake -D CMAKE_BUILD_TYPE=RELEASE \
          -D CMAKE_INSTALL_PREFIX=/Users/tonyfu/Desktop/OnlineCourses/OpenCV-Qt-App/06_ObjectDetection/opencv-3.4.5/build \
          -D BUILD_opencv_apps=yes \
          -D JPEG_INCLUDE_DIR=/opt/homebrew/include \
          -D JPEG_LIBRARY=/path/to/jpeg/library \
          ..
    ```
    Next, I ran `make` and got the error:
    ```
    [ 50%] Linking CXX shared library ../../lib/libopencv_imgcodecs.dylib
    Undefined symbols for architecture arm64:
    "_jpeg_default_qtables", referenced from:
        cv::JpegEncoder::write(cv::Mat const&, std::__1::vector<int, std::__1::allocator<int> > const&) in grfmt_jpeg.cpp.o
    ld: symbol(s) not found for architecture arm64
    clang: error: linker command failed with exit code 1 (use -v to see invocation)
    make[2]: *** [lib/libopencv_imgcodecs.3.4.5.dylib] Error 1
    make[1]: *** [modules/imgcodecs/CMakeFiles/opencv_imgcodecs.dir/all] Error 2
    make: *** [all] Error 2
    ```

    **Solution**: Currently under investigation.

### 2. Cat Detection with Haar Cascade:
This is very similar to Face Detection in Chapter 4. The only difference is that we are using a different classifier.

- **Classifier Storage**:  
  Haar cascade classifiers are located in `/opt/homebrew/share/opencv4/haarcascades/` in XML format.

- **Haar Cascade Implementation**:  
  To integrate the Haar Cascade classifiers:
  - Append `-lopencv_objdetect` to the LIBS within the .pro file.
  - Incorporate the `DEFINES += OPENCV_DATA_DIR=\\\"/opt/homebrew/share/opencv4/\\\"` macro. This will be referenced later during classifier loading.

  Here's a simple guide to face detection:
  ```cpp
  #include "opencv2/objdetect.hpp"

  cv::CascadeClassifier *classifier = new cv::CascadeClassifier(OPENCV_DATA_DIR "haarcascades/haarcascade_frontalcatface_extended.xml");
  
  while (running)
  {
      cap >> tmp_frame;

      // Face detection process
      vector<cv::Rect> faces;
      cv::Mat gray_frame;
      cv::cvtColor(tmp_frame, gray_frame, cv::COLOR_BGR2GRAY);
      classifier->detectMultiScale(gray_frame, faces, 1.3, 5);

      // Drawing red bounding boxes around detected faces
      cv::Scalar color = cv::Scalar(0, 0, 255); // red
      for (size_t i = 0; i < faces.size(); i++)
      {
          cv::rectangle(tmp_frame, faces[i], color, 1);
      }

      // Continuation of the code (frame update, signal emission, etc.)
  }
  ```

**Results**:

![cat_detection_example](images/cat_detection_example.png)


