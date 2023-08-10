#include <QObject>
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

#include "utilities.h"

// The Utilities class provides utility methods related to file paths and naming.

QString Utilities::getDataPath()
{
    // Get the standard path for the user's pictures directory
    QString user_pictures_path = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation)[0];
    
    // Create a QDir object for operations in the pictures directory
    QDir pictures_dir(user_pictures_path);
    
    // Ensure a directory named "OpenCV-Qt-App-04-Face-Detection" exists in the pictures directory. If it doesn't, create it.
    pictures_dir.mkpath("OpenCV-Qt-App-04-Face-Detection");
    
    // Return the absolute path to the "OpenCV-Qt-App-04-Face-Detection" directory
    return pictures_dir.absoluteFilePath("OpenCV-Qt-App-04-Face-Detection");
}

QString Utilities::newPhotoName()
{
    // Get the current date and time
    QDateTime time = QDateTime::currentDateTime();
    
    // Return the current date and time as a string formatted in the given pattern.
    // This will be used as the new name for the photo.
    return time.toString("yyyy-MM-dd+HH:mm:ss");
}

QString Utilities::getPhotoPath(QString name, QString postfix)
{
    // Construct the full path for a photo.
    // It combines the directory path from getDataPath(), the given photo name, and the provided file extension (postfix).
    // For example, it might return something like: "/Users/user/Pictures/Facetious/2023-08-09+22:15:23.jpg"
    return QString("%1/%2.%3").arg(Utilities::getDataPath(), name, postfix);
}
