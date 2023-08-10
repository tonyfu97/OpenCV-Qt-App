#include <QApplication>
#include <QImage>
#include <QTime>
#include <QDebug>

#include "utilities.h"
#include "capture_thread.h"

CaptureThread::CaptureThread(int camera, QMutex *lock) : running(false), cameraID(camera), videoPath(""), data_lock(lock)
{
    frame_width = frame_height = 0;
    taking_photo = false;
}

CaptureThread::CaptureThread(QString videoPath, QMutex *lock) : running(false), cameraID(-1), videoPath(videoPath), data_lock(lock)
{
    frame_width = frame_height = 0;
    taking_photo = false;
}

CaptureThread::~CaptureThread()
{
}

// Main loop for capturing and processing video frames
void CaptureThread::run()
{
    running = true;
    cv::VideoCapture cap(cameraID);
    if (!cap.isOpened())
    {
        std::cerr << "Error opening camera!" << std::endl;
        return;
    }

    // cv::VideoCapture cap("path/to/video/file");
    cv::Mat tmp_frame;

    // Update video frame dimensions
    frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);

    // Face detection
    classifier = new cv::CascadeClassifier(OPENCV_DATA_DIR "haarcascades/haarcascade_frontalface_default.xml");

    while (running)
    {
        cap >> tmp_frame;
        if (tmp_frame.empty())
        {
            break;
        }
        if (taking_photo)
        {
            takePhoto(tmp_frame);
        }

        detectFaces(tmp_frame);

        // Convert frame color from BGR to RGB
        cvtColor(tmp_frame, tmp_frame, cv::COLOR_BGR2RGB);

        // Thead-safe update
        data_lock->lock();
        frame = tmp_frame;
        data_lock->unlock();

        // Emit a signal indicating a new frame has been captured
        emit frameCaptured(&frame);
    }

    // Cleanup
    cap.release();
    delete classifier;
    classifier = nullptr;
    running = false;
}

// Setters for thread controls and video capture configurations
void CaptureThread::setRunning(bool run)
{
    running = run;
}

void CaptureThread::takePhoto(cv::Mat &frame)
{
    QString photo_name = Utilities::newPhotoName();
    QString photo_path = Utilities::getPhotoPath(photo_name, "jpg");
    cv::imwrite(photo_path.toStdString(), frame);
    emit photoTaken(photo_name);
    taking_photo = false;
}

void CaptureThread::detectFaces(cv::Mat &frame)
{
    vector<cv::Rect> faces;
    cv::Mat gray_frame;
    cv::cvtColor(frame, gray_frame, cv::COLOR_BGR2GRAY);
    classifier->detectMultiScale(gray_frame, faces, 1.3, 5);

    cv::Scalar color = cv::Scalar(0, 0, 255); // red

    // draw the circumscribe rectangles
    for (size_t i = 0; i < faces.size(); i++)
    {
        cv::rectangle(frame, faces[i], color, 1);
    }
}
