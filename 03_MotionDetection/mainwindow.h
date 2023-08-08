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
#include <QMap>
#include <QCheckBox>
#include <QPushButton>
#include <QListView>
#include <QStandardItemModel>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent=nullptr);
    ~MainWindow() = default;

private:
    void initUI();
    void createActions();

private:
    QMenu *fileMenu;

    QAction *cameraInfoAction;
    QAction *openCameraAction;
    QAction *exitAction;

    QGraphicsScene *imageScene;
    QGraphicsView *imageView;

    QCheckBox *monitorCheckBox;
    QPushButton *recordButton;

    QListView *saved_list;
    QStandardItemModel *list_model;

    QStatusBar *mainStatusBar;
    QLabel *mainStatusLabel;
};
