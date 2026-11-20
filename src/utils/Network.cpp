#include "utils/Network.hpp"

#include <QHttpHeaders>
#include <QJsonParseError>
#include <QRestReply>

namespace genexis::utils::network {

NetworkClient::NetworkClient(QObject *parent) : QObject(parent), m_rest(&m_nam, this) {}

void NetworkClient::getJson(const QUrl &baseUrl, const QString &path, const QUrlQuery &query,
                            JsonCallback callback, std::chrono::milliseconds timeout) {
    if (!callback) {
        return;
    }

    if (!baseUrl.isValid() || baseUrl.isEmpty()) {
        callback(std::unexpected(QStringLiteral("Invalid base URL")));
        return;
    }

    QNetworkRequestFactory factory(baseUrl);
    QHttpHeaders headers;
    headers.append(QHttpHeaders::WellKnownHeader::UserAgent, QStringLiteral("Genexis"));
    factory.setCommonHeaders(headers);
    factory.setTransferTimeout(timeout);

    const QNetworkRequest request =
        query.isEmpty() ? factory.createRequest(path) : factory.createRequest(path, query);

    m_rest.get(request, this, [cb = std::move(callback)](QRestReply &reply) {
        if (!reply.isSuccess()) {
            const QString err = reply.errorString().isEmpty()
                                    ? QStringLiteral("HTTP request failed (%1)")
                                          .arg(reply.httpStatus())
                                    : reply.errorString();
            cb(std::unexpected(err));
            return;
        }

        QJsonParseError parseError;
        const auto doc = reply.readJson(&parseError);
        if (!doc) {
            cb(std::unexpected(
                QStringLiteral("JSON parse error: %1").arg(parseError.errorString())));
            return;
        }

        cb(*doc);
    });
}

} // namespace genexis::utils::network
