#pragma once

#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkRequestFactory>
#include <QObject>
#include <QRestAccessManager>
#include <QString>
#include <QUrl>
#include <QUrlQuery>

#include <chrono>
#include <expected>
#include <functional>

namespace genexis::utils::network {

/// Non-blocking HTTP/REST JSON client built on QNetworkRequestFactory and QRestAccessManager.
class NetworkClient : public QObject {
    Q_OBJECT

public:
    using JsonCallback = std::function<void(std::expected<QJsonDocument, QString>)>;

    explicit NetworkClient(QObject *parent = nullptr);

    /// Performs an asynchronous HTTP GET expecting a JSON payload.
    void getJson(const QUrl &baseUrl, const QString &path, const QUrlQuery &query,
                 JsonCallback callback,
                 std::chrono::milliseconds timeout = std::chrono::seconds(8));

private:
    QNetworkAccessManager m_nam;
    QRestAccessManager m_rest;
};

} // namespace genexis::utils::network
