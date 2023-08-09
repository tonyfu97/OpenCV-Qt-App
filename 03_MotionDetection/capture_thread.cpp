#include <QTime>
#include <QtConcurrent>
#include <QDebug>

#include "utilities.h"
#include "capture_thread.h"

CaptureThread::CaptureThread(int camera, QMutex *lock) : running(false), cameraID(camera), videoPath(""), data_lock(lock)
{
    fps_calculating = false;
    fps = 0.0;

    frame_width = frame_height = 0;
    video_saving_status = STOPPED;
    saved_video_name = "";
    video_writer = nullptr;

    motion_detecting_status = false;
}

CaptureThread::CaptureThread(QString videoPath, QMutex *lock) : running(false), cameraID(-1), videoPath(videoPath), data_lock(lock)
{
    fps_calculating = false;
    fps = 0.0;

    frame_width = frame_height = 0;
    video_saving_status = STOPPED;
    saved_video_name = "";
    video_writer = nullptr;

    motion_detecting_status = false;
}

// Main loop for capturing and processing video frames
void CaptureThread::run()
{
    running = true;
    cv::VideoCapture cap(cameraID);
    // cv::VideoCapture cap("path/to/video/file");
    cv::Mat tmp_frame;

    // Update video frame dimensions
    frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);

    // Initialize a background subtractor for motion detection
    segmentor = cv::createBackgroundSubtractorMOG2(500, 16, true);

    while (running)
    {
        cap >> tmp_frame;
        if (tmp_frame.empty())
        {
            break;
        }
        if (motion_detecting_status)
        {
            motionDetect(tmp_frame);
        }
        if (video_saving_status == STARTING)
        {
            startSavingVideo(tmp_frame);
        }
        if (video_saving_status == STARTED)
        {
            video_writer->write(tmp_frame);
        }
        if (video_saving_status == STOPPING)
        {
            stopSavingVideo();
        }

        // Convert frame color from BGR to RGB
        cvtColor(tmp_frame, tmp_frame, cv::COLOR_BGR2RGB);

        // Thead-safe update
        data_lock->lock();
        frame = tmp_frame;
        data_lock->unlock();

        // Emit a signal indicating a new frame has been captured
        emit frameCaptured(&frame);
        if (fps_calculating)
        {
            calculateFPS(cap);
        }
    }

    // Cleanup
    cap.release();
    running = false;
}

/*
 * Calculate the FPS by reading 100 frames from the video capture device, then
 * divide the number of frames by the elapsed time in seconds.
 */
void CaptureThread::calculateFPS(cv::VideoCapture &cap)
{
    const int count_to_read = 100;
    cv::Mat tmp_frame;
    QTime timer;
    timer.start();
    for (int i = 0; i < count_to_read; i++)
    {
        cap >> tmp_frame;
    }
    int elapsed_ms = timer.elapsed();
    fps = count_to_read / (elapsed_ms / 1000.0);
    fps_calculating = false;

    // Emit a signal to inform about the updated FPS
    emit fpsChanged(fps);
}

void CaptureThread::startSavingVideo(cv::Mat &firstFrame)
{
    saved_video_name = Utilities::newSavedVideoName();

    QString cover = Utilities::getSavedVideoPath(saved_video_name, "jpg");
    cv::imwrite(cover.toStdString(), firstFrame);

    video_writer = new cv::VideoWriter(
        Utilities::getSavedVideoPath(saved_video_name, "avi").toStdString(),
        cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
        fps ? fps : 30,
        cv::Size(frame_width, frame_height));
    video_saving_status = STARTED;
}

void CaptureThread::stopSavingVideo()
{
    video_saving_status = STOPPED;
    video_writer->release();
    delete video_writer;
    video_writer = nullptr;
    emit videoSaved(saved_video_name);
}

void CaptureThread::motionDetect(cv::Mat &frame)
{
    // Create an empty matrix to store the foreground mask (result of background subtraction)
    cv::Mat fgmask;
    
    // Apply the background subtraction method on the given frame. The result is stored in fgmask.
    segmentor->apply(frame, fgmask);

    // If fgmask is empty (no foreground detected), exit the function
    if (fgmask.empty())
    {
        return;
    }

    // Threshold the foreground mask. All pixel values below 25 become 0 (black)
    // and values above become 255 (white).
    cv::threshold(fgmask, fgmask, 25, 255, cv::THRESH_BINARY);

    // Define the size for the structuring element, which is used for morphological operations. The name "noise_size" can be misleading. Here it is used to remove noise in the foreground mask.
    int noise_size = 9;

    // Create a rectangular structuring element.
    // cv::MORPH_RECT results in a matrix filled with ones, just like creating a cv::Mat of ones manually. We use cv::getStructuringElement() here for clarity.
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(noise_size, noise_size));

    // Erode the image to reduce speckles and small blobs
    cv::erode(fgmask, fgmask, kernel);
    
    // Dilate the image to consolidate white regions and fill small holes
    cv::dilate(fgmask, fgmask, kernel, cv::Point(-1, -1), 3);

    // Find contours in the foreground mask. These contours represent potential motion regions.
    vector<vector<cv::Point>> contours;
    cv::findContours(fgmask, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    // If there are contours, it indicates motion
    bool has_motion = contours.size() > 0;

    // If motion is newly detected, start saving the video and send a notification
    if (!motion_detected && has_motion)
    {
        motion_detected = true;
        setVideoSavingStatus(STARTING);
        qDebug() << "new motion detected, should send a notification.";
    }
    // If motion was previously detected but now disappeared, stop saving the video
    else if (motion_detected && !has_motion)
    {
        motion_detected = false;
        setVideoSavingStatus(STOPPING);
        qDebug() << "detected motion disappeared.";
    }

    // Set the color for drawing contours (red in this case)
    cv::Scalar color = cv::Scalar(0, 0, 255); 

    // Iterate through detected contours and draw rectangles around them on the original frame
    for (size_t i = 0; i < contours.size(); i++)
    {
        cv::Rect rect = cv::boundingRect(contours[i]);
        cv::rectangle(frame, rect, color, 1);
        // The following line can be used to draw the contours directly, instead of rectangles
        // cv::drawContours(frame, contours, (int)i, color, 1);
    }
}

// Setters for thread controls and video capture configurations
void CaptureThread::setRunning(bool run)
{
    running = run;
}

void CaptureThread::startCalcFPS()
{
    fps_calculating = true;
}

void CaptureThread::setVideoSavingStatus(VideoSavingStatus status)
{
    video_saving_status = status;
}

void CaptureThread::setMotionDetectingStatus(bool status)
{
    motion_detecting_status = status;
}
