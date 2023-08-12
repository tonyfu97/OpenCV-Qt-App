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

void CaptureThread::run()
{
    running = true;
    cv::VideoCapture cap(cameraID);
    // cv::VideoCapture cap("/home/kdr2/Videos/WIN_20190123_20_14_56_Pro.mp4");
    cv::Mat tmp_frame;

    frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);

    // Cat face detection
    classifier = new cv::CascadeClassifier(OPENCV_DATA_DIR "haarcascades/haarcascade_frontalcatface_extended.xml");

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

        detectObjects(tmp_frame);

        cvtColor(tmp_frame, tmp_frame, cv::COLOR_BGR2RGB);
        data_lock->lock();
        frame = tmp_frame;
        data_lock->unlock();
        emit frameCaptured(&frame);
    }
    cap.release();
    delete classifier;
    classifier = nullptr;
    running = false;
}

void CaptureThread::takePhoto(cv::Mat &frame)
{
    QString photo_name = Utilities::newPhotoName();
    QString photo_path = Utilities::getPhotoPath(photo_name, "jpg");
    cv::imwrite(photo_path.toStdString(), frame);
    emit photoTaken(photo_name);
    taking_photo = false;
}

void CaptureThread::detectObjects(cv::Mat &frame)
{
    vector<cv::Rect> objects;
    int minNeighbors = 5; // 3 for no-entry-sign; 5-for others.
    classifier->detectMultiScale(frame, objects, 1.3, minNeighbors);

    cv::Scalar color = cv::Scalar(0, 0, 255); // red

    // draw the circumscribe rectangles
    for(size_t i = 0; i < objects.size(); i++) {
        cv::rectangle(frame, objects[i], color, 2);
    }
}
