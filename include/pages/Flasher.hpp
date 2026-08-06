#pragma once

#include <QObject>
#include <qqmlregistration.h>

class Flasher : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit Flasher(QObject *parent = nullptr);
    ~Flasher() override = default;
};
