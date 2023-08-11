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
    void takePhoto() { taking_photo = true; }

    enum MASK_TYPE
    {
        RECTANGLE = 0,
        LANDMARKS,
        GLASSES,
        MUSTACHE,
        MOUSE_NOSE,
        MASK_COUNT,
    };

    void updateMasksFlag(MASK_TYPE type, bool on_or_off)
    {
        uint8_t bit = 1 << type;
        if (on_or_off)
        {
            masks_flag |= bit;
        }
        else
        {
            masks_flag &= ~bit;
        }
    };

protected:
    void run() override; // Main loop for capturing and processing video frames

signals:
    // Signals to notify other Qt components about frame capture, FPS changes, and video saving status
    void frameCaptured(cv::Mat *data);
    void photoTaken(QString name);

private:
    void takePhoto(cv::Mat &frame);
    void detectFaces(cv::Mat &frame);
    void loadOrnaments();
    void drawGlasses(cv::Mat &frame, vector<cv::Point2f> &marks);
    void drawMustache(cv::Mat &frame, vector<cv::Point2f> &marks);
    void drawMouseNose(cv::Mat &frame, vector<cv::Point2f> &marks);
    bool isMaskOn(MASK_TYPE type) { return (masks_flag & (1 << type)) != 0; };

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

    // mask ornaments
    cv::Mat glasses;
    cv::Mat mustache;
    cv::Mat mouse_nose;
    uint8_t masks_flag;
};