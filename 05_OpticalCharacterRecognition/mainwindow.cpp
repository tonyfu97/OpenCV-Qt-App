#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QSplitter>
#include <QDebug>

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    initUI();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUI()
{
    this->resize(800, 600);
    // setup menubar
    fileMenu = menuBar()->addMenu("&File");

    // setup toolbar
    fileToolBar = addToolBar("File");

    // main area (split vertically into two parts)
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);

    imageScene = new QGraphicsScene(this);
    imageView = new QGraphicsView(imageScene);
    splitter->addWidget(imageView);

    editor = new QTextEdit(this);
    splitter->addWidget(editor);

    QList<int> sizes = {400, 400};
    splitter->setSizes(sizes);

    // built-in function of QMainWindow to set the main widget
    setCentralWidget(splitter);

    // setup status bar
    mainStatusBar = statusBar();
    mainStatusLabel = new QLabel(mainStatusBar);
    mainStatusBar->addPermanentWidget(mainStatusLabel);
    mainStatusLabel->setText("Application Information will be here!");

    createActions();
}

void MainWindow::createActions()
{
    // create actions, add them to menus
    openAction = new QAction("&Open", this);
    fileMenu->addAction(openAction);
    saveImageAsAction = new QAction("Save &Image as", this);
    fileMenu->addAction(saveImageAsAction);
    saveTextAsAction = new QAction("Save &Text as", this);
    fileMenu->addAction(saveTextAsAction);
    exitAction = new QAction("E&xit", this);
    fileMenu->addAction(exitAction);

    // add actions to toolbars
    fileToolBar->addAction(openAction);

    setupShortcuts();
}

void MainWindow::setupShortcuts()
{
    QList<QKeySequence> shortcuts;
    shortcuts << (Qt::CTRL + Qt::Key_O);
    openAction->setShortcuts(shortcuts);

    shortcuts.clear();
    shortcuts << (Qt::CTRL + Qt::Key_Q);
    exitAction->setShortcuts(shortcuts);
}
