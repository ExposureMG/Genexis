#pragma once

#include <QObject>
#include <qqmlregistration.h>

class Home : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit Home(QObject *parent = nullptr);
    ~Home() override = default;
};
