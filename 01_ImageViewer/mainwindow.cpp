#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QKeyEvent>
#include <QDebug>
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), fileMenu(nullptr), viewMenu(nullptr), currentImage(nullptr)
{
    initUI();
}

void MainWindow::initUI()
{
    this->resize(800, 600);
    // setup menubar
    fileMenu = menuBar()->addMenu("&File");
    viewMenu = menuBar()->addMenu("&View");

    // setup toolbar
    fileToolBar = addToolBar("File");
    viewToolBar = addToolBar("View");

    // main area for image display
    imageScene = new QGraphicsScene(this);
    imageView = new QGraphicsView(imageScene);
    setCentralWidget(imageView);

    // setup status bar
    mainStatusBar = statusBar();
    mainStatusLabel = new QLabel(mainStatusBar);
    mainStatusBar->addPermanentWidget(mainStatusLabel);
    mainStatusLabel->setText("Image Information will be here!");

    createActions();
}

void MainWindow::createActions()
{

    // create actions, add them to menus
    openAction = new QAction("&Open", this);
    fileMenu->addAction(openAction);
    saveAsAction = new QAction("&Save as", this);
    fileMenu->addAction(saveAsAction);
    exitAction = new QAction("E&xit", this);
    fileMenu->addAction(exitAction);

    zoomInAction = new QAction("Zoom in", this);
    viewMenu->addAction(zoomInAction);
    zoomOutAction = new QAction("Zoom Out", this);
    viewMenu->addAction(zoomOutAction);
    prevAction = new QAction("&Previous Image", this);
    viewMenu->addAction(prevAction);
    nextAction = new QAction("&Next Image", this);
    viewMenu->addAction(nextAction);

    // add actions to toolbars
    fileToolBar->addAction(openAction);
    viewToolBar->addAction(zoomInAction);
    viewToolBar->addAction(zoomOutAction);
    viewToolBar->addAction(prevAction);
    viewToolBar->addAction(nextAction);

    // connect the signals and slots
    connect(exitAction, SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));
    connect(openAction, SIGNAL(triggered(bool)), this, SLOT(openImage()));
    connect(saveAsAction, SIGNAL(triggered(bool)), this, SLOT(saveAs()));
    connect(zoomInAction, SIGNAL(triggered(bool)), this, SLOT(zoomIn()));
    connect(zoomOutAction, SIGNAL(triggered(bool)), this, SLOT(zoomOut()));
    connect(prevAction, SIGNAL(triggered(bool)), this, SLOT(prevImage()));
    connect(nextAction, SIGNAL(triggered(bool)), this, SLOT(nextImage()));

    setupShortcuts();
}

void MainWindow::openImage()
{
    // Initialize a file dialog for opening an image.
    QFileDialog dialog(this);
    dialog.setWindowTitle("Open Image"); // Set the title of the dialog.
    dialog.setFileMode(QFileDialog::ExistingFile); // Ensure only existing files can be selected.
    dialog.setNameFilter(tr("Images (*.png *.bmp *.jpg)")); // Only allow image files with these extensions to be shown.

    QStringList filePaths;

    // Show the dialog and wait for user interaction.
    if (dialog.exec())
    {
        // Get the selected file path(s) from the dialog.
        filePaths = dialog.selectedFiles();

        // Display the chosen image in the application. Because we use ExistingFile
        // option, only one file can be selected.
        showImage(filePaths.at(0));
    }
}

void MainWindow::showImage(QString path)
{
    // Clear any previous images from the scene.
    imageScene->clear();

    // Reset the view.
    imageView->resetTransform();

    // Load the image from the given path.
    QPixmap image(path);

    // Add the loaded image to the scene and get a reference to it.
    currentImage = imageScene->addPixmap(image);

    // Update the scene to reflect changes.
    imageScene->update();

    // Set the view's scene rectangle to match the image dimensions.
    imageView->setSceneRect(image.rect());

    // Construct a status string with image path, dimensions, and file size.
    QString status = QString("%1, %2x%3, %4 Bytes").arg(path).arg(image.width()).arg(image.height()).arg(QFile(path).size());

    // Display the status string in the main status label.
    mainStatusLabel->setText(status);

    // Update the current image path if everything was successful.
    currentImagePath = path;
}

void MainWindow::zoomIn()
{
    imageView->scale(1.2, 1.2);
}

void MainWindow::zoomOut()
{
    imageView->scale(1 / 1.2, 1 / 1.2);
}

void MainWindow::prevImage()
{
    // Get file info.
    QFileInfo current(currentImagePath);

    // Get the directory containing the current image.
    QDir dir = current.absoluteDir();

    // Define the image file extensions to filter.
    QStringList nameFilters;
    nameFilters << "*.png"
                << "*.bmp"
                << "*.jpg";

    // Retrieve the list of image files in the directory, sorted by name.
    QStringList fileNames = dir.entryList(nameFilters, QDir::Files, QDir::Name);

    // Find the index of the current image in the sorted list.
    int idx = fileNames.indexOf(QRegExp(QRegExp::escape(current.fileName())));

    // Check if the current image isn't the first one.
    if (idx > 0)
    {
        // Display the previous image.
        showImage(dir.absoluteFilePath(fileNames.at(idx - 1)));
    }
    else
    {
        // Inform the user that the current image is the first one.
        QMessageBox::information(this, "Information", "Current image is the first one.");
    }
}

void MainWindow::nextImage()
{
    // Get file info.
    QFileInfo current(currentImagePath);

    // Get the directory containing the current image.
    QDir dir = current.absoluteDir();

    // Define the image file extensions to filter.
    QStringList nameFilters;
    nameFilters << "*.png"
                << "*.bmp"
                << "*.jpg";

    // Retrieve the list of image files in the directory, sorted by name.
    QStringList fileNames = dir.entryList(nameFilters, QDir::Files, QDir::Name);

    // Find the index of the current image in the sorted list.
    int idx = fileNames.indexOf(QRegExp(QRegExp::escape(current.fileName())));

    // Check if the current image isn't the last one.
    if (idx < fileNames.size() - 1)
    {
        // Display the next image.
        showImage(dir.absoluteFilePath(fileNames.at(idx + 1)));
    }
    else
    {
        // Inform the user that the current image is the last one.
        QMessageBox::information(this, "Information", "Current image is the last one.");
    }
}

void MainWindow::saveAs()
{
    // Check if there's an image loaded in the application.
    if (currentImage == nullptr)
    {
        // Display a message box informing the user that there's no image to save.
        QMessageBox::information(this, "Information", "Nothing to save.");
        return;
    }

    // Initialize a file dialog for saving an image.
    QFileDialog dialog(this);
    dialog.setWindowTitle("Save Image As ...");
    dialog.setFileMode(QFileDialog::AnyFile); // Allows any type of file to be selected.
    dialog.setAcceptMode(QFileDialog::AcceptSave); // Set the mode to "Save" rather than "Open".
    dialog.setNameFilter(tr("Images (*.png *.bmp *.jpg)")); // Only show image files with these extensions.

    QStringList fileNames; // List to store the selected file name(s) from the dialog.

    // Show the dialog and wait for user interaction.
    if (dialog.exec())
    {
        // Get the selected file name(s) from the dialog.
        fileNames = dialog.selectedFiles();

        // Validate the file name and its extension.
        if (QRegExp(".+\\.(png|bmp|jpg)").exactMatch(fileNames.at(0)))
        {
            // Save the current image to the selected path with the appropriate format.
            currentImage->pixmap().save(fileNames.at(0));
        }
        else
        {
            // Display a message box if the file format or name is invalid.
            QMessageBox::information(this, "Information", "Save error: bad format or filename.");
        }
    }
}

void MainWindow::setupShortcuts()
{
    QList<QKeySequence> shortcuts;
    shortcuts << Qt::Key_Plus << Qt::Key_Equal;
    zoomInAction->setShortcuts(shortcuts);

    shortcuts.clear();
    shortcuts << Qt::Key_Minus << Qt::Key_Underscore;
    zoomOutAction->setShortcuts(shortcuts);

    shortcuts.clear();
    shortcuts << Qt::Key_Up << Qt::Key_Left;
    prevAction->setShortcuts(shortcuts);

    shortcuts.clear();
    shortcuts << Qt::Key_Down << Qt::Key_Right;
    nextAction->setShortcuts(shortcuts);
}