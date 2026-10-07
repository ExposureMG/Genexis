#pragma once

#include <QFileInfo>
#include <QString>
#include <QUrl>

namespace gxapi {

// QML file dialogs hand back file:// URLs; everything else is taken as a path.
inline QString toLocalPath(const QString &pathOrUrl) {
  if (pathOrUrl.startsWith(QStringLiteral("file://"))) {
    return QUrl(pathOrUrl).toLocalFile();
  }
  return pathOrUrl;
}

// CPLD timing files are played over JTAG instead of being written to NAND.
inline bool isJtagTimingFile(const QString &path) {
  const QString ext = QFileInfo(path).suffix().toLower();
  return ext == QStringLiteral("svf") || ext == QStringLiteral("xsvf");
}

} // namespace gxapi
