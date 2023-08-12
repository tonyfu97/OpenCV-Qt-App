#!/bin/bash

# prepare smaples
ls background/* >bg.txt
opencv_createsamples -vec samples.vec -img no-entry.png -bg bg.txt \
                     -num 200 -bgcolor 0 -bgthresh 20 -maxidev 30 \
                     -maxxangle 0.3 -maxyangle 0.3 -maxzangle 0.3 \
                     -w 32 -h 32

# train
rm -fr classifier
mkdir -p classifier
opencv_traincascade -data classifier -numStages 10 -featureType HAAR -vec samples.vec -bg bg.txt -numPos 200 -numNeg 200 -h 32 -w 32


cmake -D CMAKE_BUILD_TYPE=RELEASE \
      -D CMAKE_INSTALL_PREFIX=/Users/tonyfu/Desktop/OnlineCourses/OpenCV-Qt-App/06_ObjectDetection/opencv-3.4.5/build \
      -D BUILD_opencv_apps=yes \
      ..


cmake -D CMAKE_BUILD_TYPE=RELEASE \
      -D CMAKE_INSTALL_PREFIX=/Users/tonyfu/Desktop/OnlineCourses/OpenCV-Qt-App/06_ObjectDetection/opencv-3.4.5/build \
      -D BUILD_opencv_apps=yes \
      -D JPEG_INCLUDE_DIR=/opt/homebrew/include \
      -D JPEG_LIBRARY=/path/to/jpeg/library \
      ..



