#pragma once

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QStatusBar>
#include <QLabel>
#include <QGraphicsPixmapItem>

class MainWindow : public QMainWindow
{
    Q_OBJECT  // macro to enable Qt's meta-object system

public:
    explicit MainWindow(QWidget *parent=nullptr);
    ~MainWindow() = default;

private:
    void initUI();
    void createActions();
    void showImage(QString);
    void setupShortcuts();

private slots:
    /*
    In Qt, the signal and slot mechanism is used for communication between
    objects. It's a way of connecting a function (slot) to be called in
    response to an event or change in another object (signal).
    */
    void openImage();
    void zoomIn();
    void zoomOut();
    void prevImage();
    void nextImage();
    void saveAs();

private:
    QMenu *fileMenu;
    QMenu *viewMenu;

    QToolBar *fileToolBar;
    QToolBar *viewToolBar;

    QGraphicsScene *imageScene; // Place items on the Scene, and
    QGraphicsView *imageView;   // use the View to decide how to display.

    QStatusBar *mainStatusBar;
    QLabel *mainStatusLabel;

    QAction *openAction;
    QAction *saveAsAction;
    QAction *exitAction;
    QAction *zoomInAction;
    QAction *zoomOutAction;
    QAction *prevAction;
    QAction *nextAction;

    QString currentImagePath;
    QGraphicsPixmapItem *currentImage;
};