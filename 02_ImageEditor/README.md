# Chapter 02: Image Editor - Learning Reflections

**Author**: Tony Fu  
**Date**: August 7, 2023

**Reference**: [Qt 5 and OpenCV 4 Computer Vision Projects](https://github.com/PacktPublishing/Qt-5-and-OpenCV-4-Computer-Vision-Projects/tree/master) by Zhuo Qingliang

## Setting Up the Development Environment

- **VS Code Integration**: 
  - Should VS Code fail to recognize OpenCV, modify `c_cpp_properties.json` and append `"/opt/homebrew/include/opencv4/**"` (or the appropriate directory) to `includePath`.

## Core Concepts

- **Debugging**:
  - Use `qDebug() << "Debug message here."` to print messages to the terminal. For running the application in macOS terminal, you have to do `./02_ImageEditor.app/Contents/MacOS/02_ImageEditor`. Simply executing `./02_ImageEditor.app` in the root directory will result in a permission denied error, even if execution permissions are granted. This behavior contrasts with the `QMessageBox::information()` method, which is designed for user notifications, not debugging.

- **Linking OpenCV in Project File**:
  - In `02_ImageEditor.pro`, to link OpenCV, the following is necessary:
    ```cpp
    unix: mac {
    INCLUDEPATH += /opt/homebrew/include/opencv4
    LIBS += -L/opt/homebrew/opt/opencv/lib -lopencv_core -lopencv_imgproc
    }
    ```
    This deviates from the book. For reasons unknown, my macOS version of OpenCV lacks the opencv_world library the book references.

  - My Mac's Qt installation, located in `/Users/tonyfu/opt/anaconda3/`, was built for x86_64, causing issues when linking with the OpenCV library at `/opt/homebrew/`, designed for arm64. As a remedy, I installed Qt (5.15.10) using `brew install qt@5`. I then adjusted `~/.zshrc` using `open -a TextEdit ~/.zshrc` followed by `source ~/.zshrc` to prioritize the homebrew (arm64) version of Qt over the anaconda (x86_64) one. This change, however, introduces the following warnings:
  ```
  ld: warning: dylib (/opt/homebrew/opt/opencv/lib/libopencv_core.dylib) was built for a newer macOS version (13.0) than being linked (11.0)
  ld: warning: dylib (/opt/homebrew/opt/opencv/lib/libopencv_imgproc.dylib) was built for a newer macOS version (13.0) than being linked (11.0)
  ```
  In spite of these warnings, I'll adhere to Qt5 due to the limited online resources for Qt6.

- **Image Format Conversion**:
  - The objective is to transition from `QPixmap` -> `QImage` -> `cv::Mat` for OpenCV processing, as detailed in `image.convert`. While `QPixmap` serves display purposes, `QImage` is tailored for direct pixel access and modification.
  - Extracting the pixmap from the scene can be achieved with:
    ```cpp
    QPixmap pixmap = currentImage->pixmap();
    QImage image = pixmap.toImage();
    ```
  - The conversion back from `QImage` to `QPixmap` can be done using: `QPixmap pixmap = QPixmap::fromImage(image)`.

- **OpenCV Quirks**:
  - By default, OpenCV uses the BGR color order. Yet, if the end goal is to convert back to a `QImage` or if channel manipulations are symmetrical, this difference can be overlooked.
  - Creating a matrix is done with `Mat mat(rows, columns, CV_<depth><type>C<channels>)`. The third argument specifies the pixel data format during matrix creation. For instance, `CV_8UC3` denotes each pixel having 8 bits, being unsigned, and possessing 3 color channels.

- **Qt Plugin Setup**: 
  - Qt's plugin system enhances code modularity. Fundamentally, a plugin is a library constructed according to a known format (specified by an interface) that is recognized by both the plugin and the primary application. This library is then loaded during runtime. The steps for plugin creation are as follows:

    1. An interface (a class with at least one pure virtual function) must be defined, establishing the plugin class format:
    ```cpp
    class MyPluginInterface {
    public:
        virtual ~MyPluginInterface() {}
        virtual QString name() const = 0;
        virtual void performAction() = 0;
    };
    ```

    2. Still in the same file, declare this class as an interface:
    ```cpp
    #define MY_PLUGIN_INTERFACE_IID "com.mycompany.MyPluginInterface"
    Q_DECLARE_INTERFACE(MyPluginInterface, MY_PLUGIN_INTERFACE_IID)
    ```

    3. Create a new directory for your plugin and implement it:
    ```cpp
    #include <QObject>
    #include <QtPlugin>
    #include "my_plugin_interface.h"

    class SamplePlugin : public QObject, public MyPluginInterface {
        Q_OBJECT
        Q_PLUGIN_METADATA(IID MY_PLUGIN_INTERFACE_IID)
        Q_INTERFACES(MyPluginInterface)

    public:
        QString name() const override {
            return "Sample Plugin";
        }
        void performAction() override {
            // Perform some action
        }
    };
    ```
    This class should inherit from two base classes and set the interface IID. Additionally, ensure the `INCLUDEPATH` in the `.pro` file is updated so `my_plugin_interface.h` can be located by qmake.

    4. To load the plugin in the main application:
    ```cpp
    #include <QPluginLoader>

    void MainWindow::loadPlugins() {
        // Retrieve the list of all plugin files in the plugins directory.
        QDir pluginsDir(QDir::currentPath() + "/plugins");
        QStringList nameFilters;
        nameFilters << "*.so" << "*.dylib" << "*.dll";
        QFileInfoList plugins = pluginsDir.entryInfoList(
            nameFilters, QDir::NoDotAndDotDot | QDir::Files, QDir::Name);
        
        // Iterate over each plugin file.
        foreach(QFileInfo plugin, plugins) {
            QPluginLoader pluginLoader(plugin.absoluteFilePath(), this);
            MyPluginInterface *plugin_ptr = dynamic_cast<MyPluginInterface*>(pluginLoader.instance());
            if(plugin_ptr) {
                // Create a new action for the plugin and add it to the edit menu and toolbar.
                QAction *action = new QAction(plugin_ptr->name(), this);
                editMenu->addAction(action);
                editToolBar->addAction(action);

                // Store the plugin pointer in a map for later reference.
                editPlugins[plugin_ptr->name()] = plugin_ptr;

                // Connect the action's triggered signal to the pluginPerform slot
                connect(action, &QAction::triggered, this, &MainWindow::pluginPerform);
            } else {
                qDebug() << "Failed to load plugin: " << plugin.absoluteFilePath();
            }
        }
    }
    ```
    This function should be invoked during the application's startup. It presumes the presence of a `/plugins` directory containing the built plugin binaries. It also assumes `MainWindow` contains a member variable `QMap<QString, MyPluginInterface*> editPlugins`.

    5. To actually carry out the plugin action, define a function:
    ```cpp
    void MainWindow::pluginPerform() {
        if (currentImage == nullptr) {
            QMessageBox::information(this, "Information", "No image available for editing.");
            return;
        }

        // Fetch the corresponding plugin using the action's text 
        QAction *active_action = qobject_cast<QAction*>(sender());
        MyPluginInterface *plugin_ptr = editPlugins[active_action->text()];
        if(!plugin_ptr) {
            QMessageBox::information(this, "Information", "No corresponding plugin found.");
            return;
        }

        // Convert the image format to RGB888 for processing.
        QPixmap pixmap = currentImage->pixmap();
        QImage image = pixmap.toImage().convertToFormat(QImage::Format_RGB888);
        cv::Mat mat = cv::Mat(
            image.height(),
            image.width(),
            CV_8UC3,
            image.bits(),
            image.bytesPerLine());

        // Apply the selected plugin's editing method.
        plugin_ptr->edit(mat, mat);

        // Convert the edited cv::Mat back to QPixmap.
        QImage image_edited(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        pixmap = QPixmap::fromImage(image_edited);

        // Show the image
        imageScene->clear();
        imageView->resetTransform();
        currentImage = imageScene->addPixmap(pixmap);
        imageScene->update();
        imageView->setSceneRect(pixmap.rect());

        // Update the status label
        QString status = QString("(Edited Image) %1x%2")
            .arg(pixmap.width()).arg(pixmap.height());
        mainStatusLabel->setText(status);
    }
    ```
    Here, `pluginPerform()` should be a private slot.
