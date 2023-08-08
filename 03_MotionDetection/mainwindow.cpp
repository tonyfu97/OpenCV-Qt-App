#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QKeyEvent>
#include <QDebug>
#include <QPluginLoader>
#include <QGridLayout>

#include "mainwindow.h"
#include "opencv2/opencv.hpp"

MainWindow::MainWindow(QWidget *parent)
{
    initUI();
}

void MainWindow::initUI()
{
    this->resize(1000, 800);
    // setup menubar
    fileMenu = menuBar()->addMenu("&File");

    // Constructs the main content area
    QGridLayout *main_layout = new QGridLayout();

    // The layout which can either contain a QCamera viewfinder or an image view.
#ifdef GAZER_USE_QT_CAMERA
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    // I have two cemaras and use the second one here
    camera = new QCamera(cameras[1]);
    viewfinder = new QCameraViewfinder(this);
    QCameraViewfinderSettings settings;
    // the size must be compatible with the camera
    settings.setResolution(QSize(800, 600));
    camera->setViewfinder(viewfinder);
    camera->setViewfinderSettings(settings);
    main_layout->addWidget(viewfinder, 0, 0, 12, 1);
#else
    imageScene = new QGraphicsScene(this);
    imageView = new QGraphicsView(imageScene);
    main_layout->addWidget(imageView, 0, 0, 12, 1);
#endif

    // Adds a tools layout section (nested in the main_layout) containing a
    // checkbox for toggling monitor status, a record button, and a label.
    QGridLayout *tools_layout = new QGridLayout();
    main_layout->addLayout(tools_layout, 12, 0, 1, 1);

    monitorCheckBox = new QCheckBox(this);
    monitorCheckBox->setText("Monitor On/Off");
    tools_layout->addWidget(monitorCheckBox, 0, 0);
    
    recordButton = new QPushButton(this);
    recordButton->setText("Record");
    tools_layout->addWidget(recordButton, 0, 1, Qt::AlignHCenter);
    tools_layout->addWidget(new QLabel(this), 0, 2);

    // Establishes connections for the monitor checkbox and record button.
    connect(monitorCheckBox, SIGNAL(stateChanged(int)), this, SLOT(updateMonitorStatus(int)));
    connect(recordButton, SIGNAL(clicked(bool)), this, SLOT(recordingStartStop()));

    // Displays a list view of saved videos beneath the main content.
    saved_list = new QListView(this);
    saved_list->setViewMode(QListView::IconMode);
    saved_list->setResizeMode(QListView::Adjust);
    saved_list->setSpacing(5);
    saved_list->setWrapping(false);
    list_model = new QStandardItemModel(this);
    saved_list->setModel(list_model);
    main_layout->addWidget(saved_list, 13, 0, 4, 1);

    // Display the widgets and components have been added to main_layout.
    QWidget *widget = new QWidget();
    widget->setLayout(main_layout);
    setCentralWidget(widget);  // Sets the main content area of the window.

    // Sets up the status bar at the bottom, indicating the application's state.
    mainStatusBar = statusBar();
    mainStatusLabel = new QLabel(mainStatusBar);
    mainStatusBar->addPermanentWidget(mainStatusLabel);
    mainStatusLabel->setText("Gazer is Ready");

    // Populate actions and saved video list.
    // createActions();
    // populateSavedList();
}

