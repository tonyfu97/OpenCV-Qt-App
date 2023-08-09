#pragma once

#include <QString>
#include <QThread>
#include <QMutex>
#include "opencv2/opencv.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/video/background_segm.hpp"

using namespace std;

class CaptureThread : public QThread
{
    Q_OBJECT

public:
    // Constructors for capturing from a camera or from a video file
    CaptureThread(int camera, QMutex *lock);
    CaptureThread(QString videoPath, QMutex *lock);

    ~CaptureThread() = default;

    // Setters for thread controls and video capture configurations
    void setRunning(bool run);
    void startCalcFPS();

    // Enumeration to handle video saving status
    enum VideoSavingStatus
    {
        STARTING,
        STARTED,
        STOPPING,
        STOPPED
    };

    void setVideoSavingStatus(VideoSavingStatus status);
    void setMotionDetectingStatus(bool status);

protected:
    void run() override; // Main loop for capturing and processing video frames

signals:
    // Signals to notify other Qt components about frame capture, FPS changes, and video saving status
    void frameCaptured(cv::Mat *data);
    void fpsChanged(float fps);
    void videoSaved(QString name);

private:
    // Internal helper functions for FPS calculation, video saving, and motion detection
    void calculateFPS(cv::VideoCapture &cap);
    void startSavingVideo(cv::Mat &firstFrame);
    void stopSavingVideo();
    void motionDetect(cv::Mat &frame);

    bool running;
    int cameraID;
    QString videoPath;
    QMutex *data_lock; // Mutex for thread-safe data access
    cv::Mat frame;

    // FPS variables
    bool fps_calculating;
    float fps;

    // Video saving variables
    int frame_width, frame_height;
    VideoSavingStatus video_saving_status;
    QString saved_video_name;
    cv::VideoWriter *video_writer;

    // Motion detection variables
    bool motion_detecting_status;
    bool motion_detected;
    cv::Ptr<cv::BackgroundSubtractorMOG2> segmentor; // OpenCV's MOG2 background subtractor
};