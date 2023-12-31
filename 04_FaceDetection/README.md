# Face Detection with Qt and OpenCV: Chapter 04 Reflections

**Author**: Tony Fu  
**Date**: August 9, 2023  
**Device**: MacBook Pro 16 inch, Late 2021 (M1 Pro)  

**Reference**: Chapter 4 of [*Qt 5 and OpenCV 4 Computer Vision Projects*](https://github.com/PacktPublishing/Qt-5-and-OpenCV-4-Computer-Vision-Projects/tree/master) by Zhuo Qingliang

## Core Concepts

### 1. Issues Encountered & Solutions:

- **`resetMatrix` is deprecated:**  
  **Solution**: Swap `imageView->resetMatrix();` with `imageView->resetTransform();`. I had encountered this deprecation warning in earlier chapters but had previously overlooked it.

- **Post-closure app crash:**  
  The following error emerged after shutting down the app:
  ```
  Thread 6 Crashed:: CaptureThread
  0   libopencv_core.4.8.0.dylib    	       0x10493b4e8 cv::parallel_for_(cv::Range const&, cv::ParallelLoopBody const&, double) + 516
  1   libopencv_core.4.8.0.dylib    	       0x10493b4dc cv::parallel_for_(cv::Range const&, cv::ParallelLoopBody const&, double) + 504
  2   libopencv_imgproc.4.8.0.dylib 	       0x104e5db10 cv::hal::cvtBGRtoBGR(unsigned char const*, unsigned long, unsigned char*, unsigned long, int, int, int, int, int, bool) + 196
  3   libopencv_imgproc.4.8.0.dylib 	       0x104e60c6c cv::cvtColorBGR2BGR(cv::_InputArray const&, cv::_OutputArray const&, int, bool) + 432
  4   libopencv_imgproc.4.8.0.dylib 	       0x104e4796c cv::cvtColor(cv::_InputArray const&, cv::_OutputArray const&, int, int) + 2212
  5   04_FaceDetection              	       0x1044b94d4 CaptureThread::run() + 216
  6   QtCore                        	       0x106849e44 0x106828000 + 138820
  7   libsystem_pthread.dylib       	       0x1a2c2a06c _pthread_start + 148
  8   libsystem_pthread.dylib       	       0x1a2c24e2c thread_start + 8
  ```
  This points to a potential issue in `CaptureThread::run()`, suggesting inadequate cleanup.  
  **Solution**: Still under investigation.

- **Unable to load facial landmark detector**: Got the following error
  ```
  libc++abi: terminating with uncaught exception of type cv::Exception: OpenCV(4.8.0) /tmp/opencv-20230706-51996-4zw318/opencv-4.8.0/opencv_contrib/modules/face/src/facemarkLBF.cpp:487: error: (-5:Bad argument) No valid input file was given, please check the given filename. in function 'loadModel'

  zsh: abort      ./04_FaceDetection.app/Contents/MacOS/04_FaceDetection
  ```
  **Solution**: Again, this is because the executable on macOS is nested. We need to define the model path like this: `QString model_path = QApplication::instance()->applicationDirPath() + "/../../../models/lbfmodel.yaml";`

- **App crashed after streaming with facial landmark detector**:
  Got the following error
  ```
  libc++abi: terminating with uncaught exception of type cv::Exception: OpenCV(4.8.0) /tmp/opencv-20230706-51996-4zw318/opencv-4.8.0/opencv_contrib/modules/face/src/facemarkLBF.cpp:430: error: (-5:Bad argument) The LBF model is not trained yet. Please provide a trained model. in function 'fitImpl'

  zsh: abort      ./04_FaceDetection.app/Contents/MacOS/04_FaceDetection
  ```
  **Solution**: Forget to load the model using `mark_detector->loadModel(model_path.toStdString());`.

- **App crashed due to `CaptureThread::drawGlasses()`**: Got the following error
  ```
  libc++abi: terminating with uncaught exception of type cv::Exception: OpenCV(4.8.0) /tmp/opencv-20230706-51996-4zw318/opencv-4.8.0/modules/imgproc/src/resize.cpp:4062: error: (-215:Assertion failed) !ssize.empty() in function 'resize'

  zsh: abort      ./04_FaceDetection.app/Contents/MacOS/04_FaceDetection
  ```
  **Solution**: I forgot to call `CaptureThread::loadOrnaments()` in the constructors.

- **App crashed due to invalid rectanlge**: Got the follwing error
  ```
  libc++abi: terminating with uncaught exception of type cv::Exception: OpenCV(4.8.0) /tmp/opencv-20230706-51996-4zw318/opencv-4.8.0/modules/core/src/matrix.cpp:809: error: (-215:Assertion failed) 0 <= roi.x && 0 <= roi.width && roi.x + roi.width <= m.cols && 0 <= roi.y && 0 <= roi.height && roi.y + roi.height <= m.rows in function 'Mat'

  zsh: abort      ./04_FaceDetection.app/Contents/MacOS/04_FaceDetection
  ```
  **Solution**: Check if the rectangle is valid before plotting:
  ```
  cv::Rect rec(center.x - rotated.cols / 2, center.y - rotated.rows / 2, rotated.cols, rotated.rows);

  if (rec.x < 0 || rec.y < 0 || (rec.x + rec.width) > frame.cols || (rec.y + rec.height) > frame.rows) {
    qWarning() << "Invalid painting rectangle!";
    return;
  }

  frame(rec) &= rotated;
  ```

### 2. Face Detection with Haar Cascade:

- **Classifier Storage**:  
  Haar cascade classifiers are located in `/opt/homebrew/share/opencv4/haarcascades/` in XML format. There are also Local Binary Patterns (LBP) cascades located in `/opt/homebrew/share/opencv4/lbpcascades/`. The latter is faster but tends to be less precise.

- **Haar Cascade Implementation**:  
  To integrate the Haar Cascade classifiers:
  - Append `-lopencv_objdetect` to the LIBS within the .pro file.
  - Incorporate the `DEFINES += OPENCV_DATA_DIR=\\\"/opt/homebrew/share/opencv4/\\\"` macro. This will be referenced later during classifier loading.

  Here's a simple guide to face detection:
  ```cpp
  #include "opencv2/objdetect.hpp"

  cv::CascadeClassifier *classifier = new cv::CascadeClassifier(OPENCV_DATA_DIR "haarcascades/haarcascade_frontalface_default.xml");
  
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

**Results** (sorry I looked very tired): 

![face_detection_example](face_detection_example.png)


### 3. Detecting Facial Landmarks with OpenCV's `face` Module:

- **Installation**: The `opencv2/face/facemark.hpp` library appears to be pre-installed on my laptop. I did not need to undergo the additional build process mentioned in the book. However, ensure that this library is included in the .pro file.

- **Downloading the Pretrained Model**: Use the following command to obtain the pretrained model: 
  ```
  curl -O https://raw.githubusercontent.com/kurnianggoro/GSOC2017/master/data/lbfmodel.yaml
  ```
  After downloading, move it to the `models` directory.

- **Implementing Facial Landmark Detection**:
  To utilize the Haar Cascade classifiers:
  - Add `-lopencv_face` to the LIBS section of the .pro file.

  Below is a brief guide on facial detection and landmarking:
  ```cpp
  cv::Ptr<cv::face::Facemark> mark_detector = cv::face::createFacemarkLBF();
  QString model_path = QApplication::instance()->applicationDirPath() + "/../../../models/lbfmodel.yaml";
  mark_detector->loadModel(model_path.toStdString());

  while (running)
  {
      cap >> tmp_frame;

      // Face detection process (details not shown)
      ...

      // Detecting facial landmarks
      vector<vector<cv::Point2f>> shapes;
      if (mark_detector->fit(frame, faces, shapes))
      {
          // Drawing facial landmarks
          for (unsigned long i = 0; i < faces.size(); i++)
          {
              for (unsigned long k = 0; k < shapes[i].size(); k++)
              {
                  cv::circle(frame, shapes[i][k], 2, color, cv::FILLED);
              }
          }
      }
  }
  ```
  Note that each `cv::Point2f` (a.k.a. `shape`) represents a single landmark point's x and y coordinates on the image. The loop is nested because there can be multiple faces, each of which can have multiple landmark points.

**Results**:

![facial_landmark_example](facial_landmark_example.png)

### 4. Qt's Resource System
- **Resource files (.qrc)**: These are XML-based files that list the assets you wish to bundle with your application. Here's a sample template: 
```xml
<!DOCTYPE RCC>
<RCC version="1.0">
    <qresource prefix="/images">
        <file>icon.png</file>
        <file>background.jpg</file>
    </qresource>
</RCC>
```

- **Link the source file to the project**: To integrate the resource into your application, add the following to the .pro file:
```
RESOURCES += file_name.qrc
```
Afterwards, run `qmake -makefile`.

- **Loading images**: With the resource system in place, images can be loaded in this manner:
```cpp
QImage image;
image.load(":/images/icon.jpg");
image = image.convertToFormat(QImage::Format_RGB888);
icon = cv::Mat(image.height(), image.width(), CV_8UC3,
               image.bits(), image.bytesPerLine()).clone();
```

### 5. Adding Face Filters

- **Integrating filter images**: Use the resource system as described above to link and load the filter images.

- **Scale, Rotate, and Paint the filters**: To fit the filters (e.g., glasses) onto a face, we must scale, rotate, and then paint them. We determine the scale of the glasses by measuring the distance between the outer eyes. To match the tilt of the face, the glasses are rotated using the `cv::warpAffine` method. The angle of rotation is calculated as the negative arctangent of the slope formed by the line connecting the outer ends of the eyes. The operation `frame(rec) &= rotated;` is a bitwise AND operation that combines the pixels of the `frame` and `rotated` matrices where the mask of `rotated` is non-zero. (`frame(rec)` selects a region of interest (ROI) from the frame matrix that corresponds to the rectangle defined by rec.)

```cpp
void CaptureThread::drawGlasses(cv::Mat &frame, vector<cv::Point2f> &marks)
{
    // resize
    cv::Mat ornament;
    cv::Point2f left_eye_end = marks[45];
    cv::Point2f right_eye_end = marks[36];

    double distance = cv::norm(left_eye_end - right_eye_end) * 1.5;
    cv::resize(glasses, ornament, cv::Size(0, 0), distance / glasses.cols, distance / glasses.cols, cv::INTER_NEAREST);

    // rotate
    double angle = -atan((left_eye_end.y - right_eye_end.y) / (left_eye_end.x - right_eye_end.x));

    cv::Point2f center = cv::Point(ornament.cols / 2, ornament.rows / 2);
    cv::Mat rotateMatrix = cv::getRotationMatrix2D(center, angle * 180 / M_PI, 1.0);

    cv::Mat rotated;
    cv::warpAffine(
        ornament, rotated, rotateMatrix, ornament.size(),
        cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));

    // paint
    center = cv::Point((left_eye_end.x + right_eye_end.x) / 2, (left_eye_end.y + right_eye_end.y) / 2);
    cv::Rect rec(center.x - rotated.cols / 2, center.y - rotated.rows / 2, rotated.cols, rotated.rows);

    if (rec.x < 0 || rec.y < 0 || (rec.x + rec.width) > frame.cols || (rec.y + rec.height) > frame.rows) {
        qWarning() << "Invalid painting rectangle!";
        return;
    }

    frame(rec) &= rotated;
}
```

**Results**:

![glasses_ornament_example](glasses_ornament_example.png)


### 6. QCheckBox

- **Step 1: Setup**
  Set up our constants and variables:
  ```cpp
  #include <QCheckBox>

  const int NUM_MASKS = 3;
  QCheckBox *mask_checkboxes[NUM_MASKS];
  uint8_t masks_flag = 0;

  enum MASK_TYPE
  {
      MASK_A = 0,
      MASK_B,
      MASK_C
  };
  ```
  - `NUM_MASKS`: The total number of mask checkboxes we wish to have.
  - `mask_checkboxes`: An array to store the pointers to our checkboxes.
  - `masks_flag`: A flag to represent the state of the checkboxes.
  - `MASK_TYPE`: An enumeration to give each mask a distinct type.

- **Step 2: Creating and Initializing the QCheckBoxes**
  Create the checkboxes, add them to our layout, and set their initial text:

  ```cpp
  for (int i = 0; i < NUM_MASKS; i++)
  {
      mask_checkboxes[i] = new QCheckBox(this);
      masks_layout->addWidget(mask_checkboxes[i], 0, i + 1);
      connect(mask_checkboxes[i], &QCheckBox::stateChanged, this, &YourClassName::updateMasks);
  }

  mask_checkboxes[0]->setText("Mask A");
  mask_checkboxes[1]->setText("Mask B");
  mask_checkboxes[2]->setText("Mask C");
  ```

  We loop through the number of masks, creating a new `QCheckBox` for each. Each checkbox is then added to `masks_layout` and connected to the `updateMasks` slot to handle its state change.

- **Step 3: Setting the Initial State**
  By default, we'll set all checkboxes to the unchecked state:
  ```cpp
  for (int i = 0; i < NUM_MASKS; i++)
  {
      mask_checkboxes[i]->setCheckState(Qt::Unchecked);
  }
  ```

- **Step 4: Handling Checkbox State Changes**
  Whenever a checkbox's state changes, the `updateMasks` slot is triggered. Inside this slot, we determine which checkbox was triggered and update the mask's flag accordingly:

  ```cpp
  QCheckBox *box = qobject_cast<QCheckBox *>(sender());
  for (int i = 0; i < NUM_MASKS; i++)
  {
      if (mask_checkboxes[i] == box)
      {
          capturer->updateMasksFlag(static_cast<MASK_TYPE>(i), box->isChecked());
      }
  }
  ```
  - We first determine which checkbox triggered the slot using `qobject_cast`.
  - We then loop through our `mask_checkboxes` array to find the matching checkbox.
  - Once found, we call `updateMasksFlag`, updating the mask's flag based on whether the checkbox is checked.

**Final Results**

![final_example](final_example.png)
