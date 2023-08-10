#pragma once

#include <QString>
#include <QThread>
#include <QMutex>
#include "opencv2/opencv.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/face/facemark.hpp"

using namespace std;

class CaptureThread : public QThread
{
    Q_OBJECT

public:
    // Constructors for capturing from a camera or from a video file
    CaptureThread(int camera, QMutex *lock);
    CaptureThread(QString videoPath, QMutex *lock);

    ~CaptureThread();

    // Setters for thread controls and video capture configurations
    void setRunning(bool run);
    void takePhoto() {taking_photo = true; }

protected:
    void run() override; // Main loop for capturing and processing video frames

signals:
    // Signals to notify other Qt components about frame capture, FPS changes, and video saving status
    void frameCaptured(cv::Mat *data);
    void photoTaken(QString name);

private:
    void takePhoto(cv::Mat &frame);
    void detectFaces(cv::Mat &frame);

private:
    bool running;
    int cameraID;
    QString videoPath;
    QMutex *data_lock; // Mutex for thread-safe data access
    cv::Mat frame;

    int frame_width, frame_height;

    // take photos
    bool taking_photo;

    // face detection
    cv::CascadeClassifier *classifier;
    cv::Ptr<cv::face::Facemark> mark_detector;
};