#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QKeyEvent>
#include <QDebug>
#include <QPluginLoader>

#include "mainwindow.h"
#include "opencv2/opencv.hpp"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), fileMenu(nullptr), viewMenu(nullptr), currentImage(nullptr)
{
    initUI();
    loadPlugins();
}

void MainWindow::initUI()
{
    this->resize(800, 600);
    // setup menubar
    fileMenu = menuBar()->addMenu("&File");
    viewMenu = menuBar()->addMenu("&View");
    editMenu = menuBar()->addMenu("&Edit");

    // setup toolbar
    fileToolBar = addToolBar("File");
    viewToolBar = addToolBar("View");
    editToolBar = addToolBar("Edit");

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

    blurAction = new QAction("Blur", this);
    editMenu->addAction(blurAction);

    // add actions to toolbars
    fileToolBar->addAction(openAction);
    viewToolBar->addAction(zoomInAction);
    viewToolBar->addAction(zoomOutAction);
    viewToolBar->addAction(prevAction);
    viewToolBar->addAction(nextAction);
    editToolBar->addAction(blurAction);

    // connect the signals and slots
    connect(exitAction, SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));
    connect(openAction, SIGNAL(triggered(bool)), this, SLOT(openImage()));
    connect(saveAsAction, SIGNAL(triggered(bool)), this, SLOT(saveAs()));
    connect(zoomInAction, SIGNAL(triggered(bool)), this, SLOT(zoomIn()));
    connect(zoomOutAction, SIGNAL(triggered(bool)), this, SLOT(zoomOut()));
    connect(prevAction, SIGNAL(triggered(bool)), this, SLOT(prevImage()));
    connect(nextAction, SIGNAL(triggered(bool)), this, SLOT(nextImage()));
    connect(blurAction, SIGNAL(triggered(bool)), this, SLOT(blurImage()));

    setupShortcuts();
}

void MainWindow::openImage()
{
    // Initialize a file dialog for opening an image.
    QFileDialog dialog(this);
    dialog.setWindowTitle("Open Image");                    // Set the title of the dialog.
    dialog.setFileMode(QFileDialog::ExistingFile);          // Ensure only existing files can be selected.
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
    imageView->resetMatrix();

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
    dialog.setFileMode(QFileDialog::AnyFile);               // Allows any type of file to be selected.
    dialog.setAcceptMode(QFileDialog::AcceptSave);          // Set the mode to "Save" rather than "Open".
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

void MainWindow::blurImage()
{
    // Check if there's a current image loaded in the window
    if (currentImage == nullptr)
    {
        QMessageBox::information(this, "Information", "No image to edit.");
        return;
    }

    // QPixmap -> QImage -> cv::Mat
    QPixmap pixmap = currentImage->pixmap();
    QImage image = pixmap.toImage();
    image = image.convertToFormat(QImage::Format_RGB888);
    cv::Mat mat = cv::Mat(
        image.height(),
        image.width(),
        CV_8UC3,               // pixel format is 8 bit unsigned integer, 3 channels
        image.bits(),          // the actual image data
        image.bytesPerLine()); // bytes per line (width * number of channels)

    // Declare a temporary OpenCV matrix for the blurred result
    cv::Mat tmp;

    // cv::blue(src, dst, cv::Size(width, height)).
    // The cv::Size(8, 8) means the blur will be applied 8x8 pixels at a time.
    cv::blur(mat, tmp, cv::Size(8, 8));
    mat = tmp;

    // Convert the OpenCV matrix back to a QImage
    QImage image_blurred(
        mat.data,
        mat.cols,
        mat.rows,
        mat.step, // bytes per line
        QImage::Format_RGB888);

    // QImage -> QPixmap
    pixmap = QPixmap::fromImage(image_blurred);

    // Clear the scene and reset the view
    imageScene->clear();
    imageView->resetMatrix();
    currentImage = imageScene->addPixmap(pixmap);
    imageScene->update();
    imageView->setSceneRect(pixmap.rect());

    // Update the status label with the size of the edited image
    QString status = QString("(editted image), %1x%2")
                         .arg(pixmap.width())
                         .arg(pixmap.height());
    mainStatusLabel->setText(status);
}

void MainWindow::loadPlugins()
{
    // Retrieve the list of all plugin files in the plugins directory.
    // QDir pluginsDir(QDir::currentPath() + "/plugins");
    QString appDirPath = QCoreApplication::applicationDirPath();
    QDir pluginsDir(appDirPath + "/../../../plugins");

    QStringList nameFilters;
    nameFilters << "*.so"
                << "*.dylib"
                << "*.dll";
    QFileInfoList plugins = pluginsDir.entryInfoList(
        nameFilters, QDir::NoDotAndDotDot | QDir::Files, QDir::Name);

    // Iterate over each plugin file.
    foreach (QFileInfo plugin, plugins)
    {
        // Load the plugin.
        QPluginLoader pluginLoader(plugin.absoluteFilePath(), this);

        // Try casting the loaded object to our EditorPluginInterface.
        EditorPluginInterface *plugin_ptr = dynamic_cast<EditorPluginInterface *>(pluginLoader.instance());

        // If the cast is successful, this is a valid plugin.
        if (plugin_ptr)
        {
            // Create a new action for the plugin and add it to the edit menu and toolbar.
            QAction *action = new QAction(plugin_ptr->name());
            editMenu->addAction(action);
            editToolBar->addAction(action);

            // Store the plugin pointer in a map for later reference.
            editPlugins[plugin_ptr->name()] = plugin_ptr;

            // Connect the action's triggered signal to the pluginPerform slot
            // to perform the specific plugin operation when the action is activated.
            connect(action, SIGNAL(triggered(bool)), this, SLOT(pluginPerform()));

            // Note: The pluginLoader.unload() line is commented out. Typically,
            // you'd unload the plugin once done, but since the application might
            // need the plugin again, it remains loaded.
            // pluginLoader.unload();
        }
        else
        {
            // If the cast fails, this is not a valid plugin.
            qDebug() << "bad plugin: " << plugin.absoluteFilePath();
        }
    }
}

void MainWindow::pluginPerform()
{
    // Check if an image is currently loaded.
    if (currentImage == nullptr)
    {
        QMessageBox::information(this, "Information", "No image to edit.");
        return;
    }

    // Get the triggered QAction from which this function was called.
    QAction *active_action = qobject_cast<QAction *>(sender());

    // Fetch the corresponding plugin using the action's text (presumably the plugin's name).
    EditorPluginInterface *plugin_ptr = editPlugins[active_action->text()];

    // Check if the fetched plugin is valid.
    if (!plugin_ptr)
    {
        QMessageBox::information(this, "Information", "No plugin is found.");
        return;
    }

    // Convert the current image (QPixmap) to a QImage.
    QPixmap pixmap = currentImage->pixmap();
    QImage image = pixmap.toImage();

    // Convert the image format to RGB888 for processing.
    image = image.convertToFormat(QImage::Format_RGB888);

    // Convert QImage to OpenCV's Mat format for editing.
    cv::Mat mat = cv::Mat(
        image.height(),
        image.width(),
        CV_8UC3,
        image.bits(),
        image.bytesPerLine());

    // Apply the selected plugin's editing method.
    plugin_ptr->edit(mat, mat);

    // Convert the edited OpenCV Mat back to a QImage.
    QImage image_edited(
        mat.data,
        mat.cols,
        mat.rows,
        mat.step,
        QImage::Format_RGB888);

    // Convert the edited QImage back to QPixmap.
    pixmap = QPixmap::fromImage(image_edited);

    // Clear the current image and add the edited image to the scene.
    imageScene->clear();
    imageView->resetMatrix();
    currentImage = imageScene->addPixmap(pixmap);

    // Update the scene and set the display rectangle.
    imageScene->update();
    imageView->setSceneRect(pixmap.rect());

    // Update the status label to indicate the edited image and its dimensions.
    QString status = QString("(editted image), %1x%2")
                         .arg(pixmap.width())
                         .arg(pixmap.height());
    mainStatusLabel->setText(status);
}
