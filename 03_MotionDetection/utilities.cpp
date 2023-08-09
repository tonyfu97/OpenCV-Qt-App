#include <QObject>
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHostInfo>
#include <QDebug>

#include "utilities.h"

QString Utilities::getDataPath()
{
    QString user_movie_path = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation)[0];
    QDir movie_dir(user_movie_path);
    movie_dir.mkpath("Gazer");
    return movie_dir.absoluteFilePath("Gazer");
}

QString Utilities::newSavedVideoName()
{
    QDateTime time = QDateTime::currentDateTime();
    return time.toString("yyyy-MM-dd+HH:mm:ss");
}

QString Utilities::getSavedVideoPath(QString name, QString postfix)
{
    return QString("%1/%2.%3").arg(Utilities::getDataPath(), name, postfix);
}
