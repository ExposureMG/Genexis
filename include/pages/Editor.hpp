#pragma once

#include <QObject>
#include <qqmlregistration.h>

class Editor : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit Editor(QObject *parent = nullptr);
    ~Editor() override = default;
};
