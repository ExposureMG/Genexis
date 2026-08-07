#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <qqmlregistration.h>

class Flasher : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool isBusy READ isBusy NOTIFY busyStateChanged)

public:
    explicit Flasher(QObject *parent = nullptr);
    ~Flasher() override = default;

    static Flasher &instance();

    bool isBusy() const;

    /// Perform NAND read/write or CPLD timing flash operation based on file extension and genexis.ini backend
    Q_INVOKABLE void performOperation(const QString &filePath, const QString &operation, const QVariantMap &options = {});

Q_SIGNALS:
    void busyStateChanged(bool busy);
    void logOutput(const QString &message);
    void progressUpdated(qreal progress, const QString &status);
    void operationFinished(bool success, const QString &message);

private:
    bool m_isBusy{false};
};
