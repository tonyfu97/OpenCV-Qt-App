#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QSplitter>
#include <QDebug>

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), currentImage(nullptr), tesseractAPI(nullptr)
{
    initUI();
}

MainWindow::~MainWindow()
{
    if (tesseractAPI != nullptr)
    {
        tesseractAPI->End();
        delete tesseractAPI;
    }
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
    ocrAction = new QAction("OCR", this);
    fileToolBar->addAction(ocrAction);
    detectAreaCheckBox = new QCheckBox("Detect Text Areas", this);
    fileToolBar->addWidget(detectAreaCheckBox);

    // connect the signals and slots
    connect(exitAction, SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));
    connect(openAction, SIGNAL(triggered(bool)), this, SLOT(openImage()));
    connect(saveImageAsAction, SIGNAL(triggered(bool)), this, SLOT(saveImageAs()));
    connect(saveTextAsAction, SIGNAL(triggered(bool)), this, SLOT(saveTextAs()));
    connect(ocrAction, SIGNAL(triggered(bool)), this, SLOT(extractText()));

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

void MainWindow::openImage()
{
    QFileDialog dialog(this);
    dialog.setWindowTitle("Open Image");
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Images (*.png *.bmp *.jpg)"));
    QStringList filePaths;
    if (dialog.exec())
    {
        filePaths = dialog.selectedFiles();
        showImage(filePaths.at(0));
    }
}

void MainWindow::showImage(QString path)
{
    imageScene->clear();
    imageView->resetTransform();
    QPixmap image(path);
    currentImage = imageScene->addPixmap(image);
    imageScene->update();
    imageView->setSceneRect(image.rect());
    QString status = QString("%1, %2x%3, %4 Bytes").arg(path).arg(image.width()).arg(image.height()).arg(QFile(path).size());
    mainStatusLabel->setText(status);
    currentImagePath = path;
}

void MainWindow::showImage(cv::Mat mat)
{
    QImage image(
        mat.data,
        mat.cols,
        mat.rows,
        mat.step,
        QImage::Format_RGB888);

    QPixmap pixmap = QPixmap::fromImage(image);
    imageScene->clear();
    imageView->resetMatrix();
    currentImage = imageScene->addPixmap(pixmap);
    imageScene->update();
    imageView->setSceneRect(pixmap.rect());
}

void MainWindow::saveImageAs()
{
    if (currentImage == nullptr)
    {
        QMessageBox::information(this, "Information", "Noting to save.");
        return;
    }
    QFileDialog dialog(this);
    dialog.setWindowTitle("Save Image As ...");
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter(tr("Images (*.png *.bmp *.jpg)"));
    QStringList fileNames;
    if (dialog.exec())
    {
        fileNames = dialog.selectedFiles();
        if (QRegExp(".+\\.(png|bmp|jpg)").exactMatch(fileNames.at(0)))
        {
            currentImage->pixmap().save(fileNames.at(0));
        }
        else
        {
            QMessageBox::information(this, "Error", "Save error: bad format or filename.");
        }
    }
}

void MainWindow::saveTextAs()
{
    QFileDialog dialog(this);
    dialog.setWindowTitle("Save Text As ...");
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter(tr("Text files (*.txt)"));
    QStringList fileNames;
    if (dialog.exec())
    {
        fileNames = dialog.selectedFiles();
        if (QRegExp(".+\\.(txt)").exactMatch(fileNames.at(0)))
        {
            QFile file(fileNames.at(0));
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QMessageBox::information(this, "Error", "Can't save text.");
                return;
            }
            QTextStream out(&file);
            out << editor->toPlainText() << "\n";
        }
        else
        {
            QMessageBox::information(this, "Error", "Save error: bad format or filename.");
        }
    }
}

void MainWindow::extractText()
{
    // Check if an image is currently loaded
    if (currentImage == nullptr)
    {
        QMessageBox::information(this, "Information", "No opened image.");
        return;
    }

    // Save the current locale and set it to the standard "C" locale
    char *old_ctype = strdup(setlocale(LC_ALL, NULL));
    setlocale(LC_ALL, "C");

    // Check if Tesseract API is already initialized
    if (tesseractAPI == nullptr)
    {
        tesseractAPI = new tesseract::TessBaseAPI();
        // Initialize tesseract-ocr with English, specifying the tessdata path
        if (tesseractAPI->Init(TESSDATA_PREFIX, "eng"))
        {
            QMessageBox::information(this, "Error", "Could not initialize tesseract.");
            return;
        }
    }

    // Convert the current image to a QImage in RGB888 format
    QPixmap pixmap = currentImage->pixmap();
    QImage image = pixmap.toImage();
    image = image.convertToFormat(QImage::Format_RGB888);

    // Set the image data to the Tesseract API
    tesseractAPI->SetImage(image.bits(), image.width(), image.height(),
                           3, image.bytesPerLine());

    if (detectAreaCheckBox->checkState() == Qt::Checked)
    {
        std::vector<cv::Rect> areas;
        cv::Mat newImage = detectTextAreas(image, areas);
        showImage(newImage);
        editor->setPlainText("");
        for (cv::Rect &rect : areas)
        {
            tesseractAPI->SetRectangle(rect.x, rect.y, rect.width, rect.height);
            char *outText = tesseractAPI->GetUTF8Text();
            editor->setPlainText(editor->toPlainText() + outText);
            delete[] outText;
        }
    }
    else
    {
        char *outText = tesseractAPI->GetUTF8Text();
        editor->setPlainText(outText);
        delete[] outText;
    }

    // Get the recognized text from the image
    char *outText = tesseractAPI->GetUTF8Text();

    // Set the recognized text to the text editor
    editor->setPlainText(outText);

    // Clean up the allocated text
    delete[] outText;

    // Restore the saved locale
    setlocale(LC_ALL, old_ctype);
    free(old_ctype);
}

cv::Mat MainWindow::detectTextAreas(QImage &image, std::vector<cv::Rect> &areas)
{
    // Define threshold values and input dimensions for the DNN model
    float confThreshold = 0.5;
    float nmsThreshold = 0.4;
    int inputWidth = 320;
    int inputHeight = 320;
    std::string model = QApplication::instance()->applicationDirPath().toStdString() + "/../../../models/frozen_east_text_detection.pb";

    // Load DNN network if it's not already loaded
    if (net.empty())
    {
        net = cv::dnn::readNet(model);
    }

    std::vector<cv::Mat> outs;
    // Define the names of the layers to be used
    std::vector<std::string> layerNames(2);
    layerNames[0] = "feature_fusion/Conv_7/Sigmoid";
    layerNames[1] = "feature_fusion/concat_3";

    // Convert QImage to cv::Mat
    cv::Mat frame = cv::Mat(
                        image.height(),
                        image.width(),
                        CV_8UC3,
                        image.bits(),
                        image.bytesPerLine())
                        .clone();
    cv::Mat blob; // blob = binary large object

    // Preprocess the image to be fed into the neural network
    cv::dnn::blobFromImage(
        frame, blob,
        1.0, cv::Size(inputWidth, inputHeight),
        cv::Scalar(123.68, 116.78, 103.94), true, false);
    // Set the processed image as the input to the neural network
    net.setInput(blob);
    // Run forward pass to get outputs of the specified layers
    net.forward(outs, layerNames);

    // Retrieve scores and geometry from the network's output
    cv::Mat scores = outs[0];
    cv::Mat geometry = outs[1];

    std::vector<cv::RotatedRect> boxes;
    std::vector<float> confidences;
    // Decode the scores and geometries to bounding boxes and confidence scores
    decode(scores, geometry, confThreshold, boxes, confidences);

    std::vector<int> indices;
    // Perform non-maximum suppression to remove overlapping boxes
    cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);

    // Render detections
    cv::Point2f ratio((float)frame.cols / inputWidth, (float)frame.rows / inputHeight);
    cv::Scalar green = cv::Scalar(0, 255, 0);

    // Iterate through the indices of the detected areas, draw rectangles and annotate them
    for (size_t i = 0; i < indices.size(); ++i)
    {
        cv::RotatedRect &box = boxes[indices[i]];
        cv::Rect area = box.boundingRect();
        area.x *= ratio.x;
        area.width *= ratio.x;
        area.y *= ratio.y;
        area.height *= ratio.y;
        areas.push_back(area);                // Add area to the provided vector
        cv::rectangle(frame, area, green, 1); // Draw rectangle around detected area
        QString index = QString("%1").arg(i);
        // Annotate the area with its index
        cv::putText(
            frame, index.toStdString(), cv::Point2f(area.x, area.y - 2),
            cv::FONT_HERSHEY_SIMPLEX, 0.5, green, 1);
    }
    return frame; // Return the frame with annotations
}

void MainWindow::decode(const cv::Mat &scores, const cv::Mat &geometry, float scoreThresh,
                        std::vector<cv::RotatedRect> &detections, std::vector<float> &confidences)
{
    // Assert statements to ensure the dimensions of the scores and geometry matrices are correct
    CV_Assert(scores.dims == 4);
    CV_Assert(geometry.dims == 4);
    CV_Assert(scores.size[0] == 1);
    CV_Assert(scores.size[1] == 1);
    CV_Assert(geometry.size[0] == 1);
    CV_Assert(geometry.size[1] == 5);
    CV_Assert(scores.size[2] == geometry.size[2]);
    CV_Assert(scores.size[3] == geometry.size[3]);

    // Clear any previous detections
    detections.clear();

    // Get the height and width of the scores matrix (this represents the spatial layout of detections)
    const int height = scores.size[2];
    const int width = scores.size[3];
    for (int y = 0; y < height; ++y)
    {
        // Pointers to data for this row (y) in the scores and geometry matrices
        const float *scoresData = scores.ptr<float>(0, 0, y);
        const float *x0_data = geometry.ptr<float>(0, 0, y);
        const float *x1_data = geometry.ptr<float>(0, 1, y);
        const float *x2_data = geometry.ptr<float>(0, 2, y);
        const float *x3_data = geometry.ptr<float>(0, 3, y);
        const float *anglesData = geometry.ptr<float>(0, 4, y);
        for (int x = 0; x < width; ++x)
        {
            float score = scoresData[x];
            // Skip detections with scores lower than threshold
            if (score < scoreThresh)
                continue;

            // Decode a prediction, transforming it into a rotated rectangle
            // Multiply by 4 because feature maps are 4 times smaller than the input image
            float offsetX = x * 4.0f, offsetY = y * 4.0f;
            float angle = anglesData[x];
            float cosA = std::cos(angle);
            float sinA = std::sin(angle);
            float h = x0_data[x] + x2_data[x];
            float w = x1_data[x] + x3_data[x];

            // Calculate offset and points to define the rotated rectangle
            cv::Point2f offset(offsetX + cosA * x1_data[x] + sinA * x2_data[x],
                               offsetY - sinA * x1_data[x] + cosA * x2_data[x]);
            cv::Point2f p1 = cv::Point2f(-sinA * h, -cosA * h) + offset;
            cv::Point2f p3 = cv::Point2f(-cosA * w, sinA * w) + offset;
            cv::RotatedRect r(0.5f * (p1 + p3), cv::Size2f(w, h), -angle * 180.0f / (float)CV_PI);

            // Add the detection and confidence score to the result vectors
            detections.push_back(r);
            confidences.push_back(score);
        }
    }
}
