#pragma once

#include <QObject>
#include <qqmlregistration.h>

class Explorer : public QObject {
  Q_OBJECT
  QML_ELEMENT

public:
  explicit Explorer(QObject *parent = nullptr);
  ~Explorer() override = default;
};
