#pragma once

#include <QObject>
#include <qqmlregistration.h>

class Nand : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit Nand(QObject *parent = nullptr);
    ~Nand() override = default;
};
