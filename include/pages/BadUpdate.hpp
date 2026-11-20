#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>

namespace genexis::pages::badupdate {

/// QML singleton shell for the BadUpdate page.
class BadUpdateController : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(BadUpdateController)
    QML_SINGLETON

public:
    static BadUpdateController &instance();
    static BadUpdateController *create(QQmlEngine *engine, QJSEngine *scriptEngine);

Q_SIGNALS:
    void errorOccurred(const QString &errorMessage);

private:
    explicit BadUpdateController(QObject *parent = nullptr);
};

} // namespace genexis::pages::badupdate
