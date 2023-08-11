#include <QApplication>
#include <QImage>
#include <QTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <stdexcept>

#include "utilities.h"
#include "capture_thread.h"

CaptureThread::CaptureThread(int camera, QMutex *lock) : running(false), cameraID(camera), videoPath(""), data_lock(lock)
{
    frame_width = frame_height = 0;
    taking_photo = false;

    loadOrnaments();
}

CaptureThread::CaptureThread(QString videoPath, QMutex *lock) : running(false), cameraID(-1), videoPath(videoPath), data_lock(lock)
{
    frame_width = frame_height = 0;
    taking_photo = false;

    loadOrnaments();
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
    mark_detector = cv::face::createFacemarkLBF();
    QString model_path = QApplication::instance()->applicationDirPath() + "/../../../models/lbfmodel.yaml";
    if (!QFile::exists(model_path))
    {
        throw std::runtime_error("Model file not found: " + model_path.toStdString() + ". Please run the command `curl -O https://raw.githubusercontent.com/kurnianggoro/GSOC2017/master/data/lbfmodel.yaml` and move the downloaded file to the `models` folder.");
    }
    mark_detector->loadModel(model_path.toStdString());

    while (running)
    {
        cap >> tmp_frame;
        if (tmp_frame.empty())
        {
            break;
        }
        if (masks_flag > 0)
            detectFaces(tmp_frame);

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
    if (isMaskOn(RECTANGLE))
    {
        for (size_t i = 0; i < faces.size(); i++)
        {
            cv::rectangle(frame, faces[i], color, 1);
        }
    }

    vector<vector<cv::Point2f>> shapes;
    if (mark_detector->fit(frame, faces, shapes))
    {
        // Draw facial land marks
        for (unsigned long i = 0; i < faces.size(); i++)
        {
            if (isMaskOn(LANDMARKS))
            {
                for (unsigned long k = 0; k < shapes[i].size(); k++)
                {
                    cv::circle(frame, shapes[i][k], 2, color, cv::FILLED);
                }
            }
            if (isMaskOn(GLASSES))
                drawGlasses(frame, shapes[i]);
            if (isMaskOn(MUSTACHE))
                drawMustache(frame, shapes[i]);
            if (isMaskOn(MOUSE_NOSE))
                drawMouseNose(frame, shapes[i]);
        }
    }
}

void CaptureThread::loadOrnaments()
{
    QImage image;
    image.load(":/ornaments/glasses.jpg");
    image = image.convertToFormat(QImage::Format_RGB888);
    glasses = cv::Mat(
                  image.height(), image.width(), CV_8UC3,
                  image.bits(), image.bytesPerLine())
                  .clone();

    image.load(":/ornaments/mustache.jpg");
    image = image.convertToFormat(QImage::Format_RGB888);
    mustache = cv::Mat(
                   image.height(), image.width(), CV_8UC3,
                   image.bits(), image.bytesPerLine())
                   .clone();

    image.load(":/ornaments/mouse-nose.jpg");
    image = image.convertToFormat(QImage::Format_RGB888);
    mouse_nose = cv::Mat(
                     image.height(), image.width(), CV_8UC3,
                     image.bits(), image.bytesPerLine())
                     .clone();
}

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
    cv::Mat rotateMatrix = cv::getRotationMatrix2D(center, angle * 180 / 3.14, 1.0);

    cv::Mat rotated;
    cv::warpAffine(
        ornament, rotated, rotateMatrix, ornament.size(),
        cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));

    // paint
    center = cv::Point((left_eye_end.x + right_eye_end.x) / 2, (left_eye_end.y + right_eye_end.y) / 2);
    cv::Rect rec(center.x - rotated.cols / 2, center.y - rotated.rows / 2, rotated.cols, rotated.rows);

    if (rec.x < 0 || rec.y < 0 || (rec.x + rec.width) > frame.cols || (rec.y + rec.height) > frame.rows)
    {
        qWarning() << "Invalid painting rectangle!";
        return;
    }

    frame(rec) &= rotated;
}

void CaptureThread::drawMustache(cv::Mat &frame, vector<cv::Point2f> &marks)
{
    // resize
    cv::Mat ornament;
    cv::Point2d left_mouth_corner = marks[54];
    cv::Point2d right_mouth_corner = marks[48];
    double distance = cv::norm(left_mouth_corner - right_mouth_corner) * 1.5;
    cv::resize(mustache, ornament, cv::Size(0, 0), distance / mustache.cols, distance / mustache.cols, cv::INTER_NEAREST);

    // rotate
    double angle = -atan((left_mouth_corner.y - right_mouth_corner.y) / (left_mouth_corner.x - right_mouth_corner.x));
    cv::Point2f center = cv::Point(ornament.cols / 2, ornament.rows / 2);
    cv::Mat rotateMatrix = cv::getRotationMatrix2D(center, angle * 180 / 3.14, 1.0);

    cv::Mat rotated;
    cv::warpAffine(
        ornament, rotated, rotateMatrix, ornament.size(),
        cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));

    // paint
    cv::Point2f nose_bottom = marks[33];
    cv::Point2f mouth_top = marks[51];
    center = cv::Point((nose_bottom.x + mouth_top.x) / 2, (nose_bottom.y + mouth_top.y) / 2);
    cv::Rect rec(center.x - rotated.cols / 2, center.y - rotated.rows / 2, rotated.cols, rotated.rows);

    if (rec.x < 0 || rec.y < 0 || (rec.x + rec.width) > frame.cols || (rec.y + rec.height) > frame.rows)
    {
        qWarning() << "Invalid painting rectangle!";
        return;
    }

    frame(rec) &= rotated;
}

void CaptureThread::drawMouseNose(cv::Mat &frame, vector<cv::Point2f> &marks)
{
    // resize
    cv::Mat ornament;
    cv::Point2d left_ear_lobe = marks[13];
    cv::Point2d right_ear_lobe = marks[3];
    double distance = cv::norm(left_ear_lobe - right_ear_lobe);
    cv::resize(mouse_nose, ornament, cv::Size(0, 0), distance / mouse_nose.cols, distance / mouse_nose.cols, cv::INTER_NEAREST);

    // rotate
    cv::Point2d left_ear_top = marks[16];
    cv::Point2d right_ear_top = marks[0];
    double angle = -atan((left_ear_top.y - right_ear_top.y) / (left_ear_top.x - right_ear_top.x));
    cv::Point2f center = cv::Point(ornament.cols / 2, ornament.rows / 2);
    cv::Mat rotateMatrix = cv::getRotationMatrix2D(center, angle * 180 / 3.14, 1.0);

    cv::Mat rotated;
    cv::warpAffine(
        ornament, rotated, rotateMatrix, ornament.size(),
        cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));

    // paint
    center = marks[30];
    cv::Rect rec(center.x - rotated.cols / 2, center.y - rotated.rows / 2, rotated.cols, rotated.rows);

    if (rec.x < 0 || rec.y < 0 || (rec.x + rec.width) > frame.cols || (rec.y + rec.height) > frame.rows)
    {
        qWarning() << "Invalid painting rectangle!";
        return;
    }

    frame(rec) &= rotated;
}
