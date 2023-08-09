#pragma once

#include <QMainWindow>
#include <QMenuBar>
#include <QAction>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QStatusBar>
#include <QLabel>
#include <QListView>
#include <QCheckBox>
#include <QPushButton>
#include <QGraphicsPixmapItem>
#include <QMutex>
#include <QStandardItemModel>

#include "opencv2/opencv.hpp"
#include "capture_thread.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

private:
    void initUI();
    void createActions();
    void populateSavedList();

private slots:
    void showCameraInfo();
    void openCamera();
    void updateFrame(cv::Mat *);
    void calculateFPS();
    void updateFPS(float);
    void recordingStartStop();
    void appendSavedVideo(QString name);
    void updateMonitorStatus(int status);

private:
    QMenu *fileMenu;

    QAction *cameraInfoAction;
    QAction *openCameraAction;
    QAction *calcFPSAction;
    QAction *exitAction;

    QGraphicsScene *imageScene;
    QGraphicsView *imageView;

    QCheckBox *monitorCheckBox;
    QPushButton *recordButton;

    QListView *saved_list;
    QStandardItemModel *list_model;

    QStatusBar *mainStatusBar;
    QLabel *mainStatusLabel;

    cv::Mat currentFrame;

    // for capture thread
    QMutex *data_lock;
    CaptureThread *capturer;
};
