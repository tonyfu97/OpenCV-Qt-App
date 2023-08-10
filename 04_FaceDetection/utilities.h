#pragma once

#include <QString>

class Utilities
{
 public:
    static QString getDataPath();
    static QString newPhotoName();
    static QString getPhotoPath(QString name, QString postfix);
};
