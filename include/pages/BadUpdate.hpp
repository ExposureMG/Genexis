#pragma once

#include <QObject>
#include <qqmlregistration.h>

class BadUpdate : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit BadUpdate(QObject *parent = nullptr);
    ~BadUpdate() override = default;
};
