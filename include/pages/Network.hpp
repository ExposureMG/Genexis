#pragma once

#include <QObject>
#include <qqmlregistration.h>

class Network : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit Network(QObject *parent = nullptr);
    ~Network() override = default;
};
