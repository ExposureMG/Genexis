#pragma once

#include <QObject>
#include <qqmlregistration.h>

class Settings : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit Settings(QObject *parent = nullptr);
    ~Settings() override = default;
};
