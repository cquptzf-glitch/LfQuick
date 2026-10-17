#include "IpcFrameCodec_p.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QtEndian>

#include <cstring>

namespace LfQuick::IpcFrameCodec {
namespace {
constexpr quint32 kMaxFrameBytes = 8u * 1024u * 1024u;
}

QByteArray encode(const QVariantMap &message, QString *error)
{
    if (error)
        error->clear();

    const QJsonDocument document = QJsonDocument::fromVariant(message);
    if (!document.isObject()) {
        if (error)
            *error = QStringLiteral("IPC message must serialize to a JSON object");
        return {};
    }

    const QByteArray payload = document.toJson(QJsonDocument::Compact);
    if (payload.size() > kMaxFrameBytes) {
        if (error)
            *error = QStringLiteral("IPC message exceeds the 8 MiB frame limit");
        return {};
    }

    QByteArray frame(4, Qt::Uninitialized);
    qToBigEndian<quint32>(static_cast<quint32>(payload.size()),
                          reinterpret_cast<uchar *>(frame.data()));
    frame.append(payload);
    return frame;
}

DecodeResult takeFrame(QByteArray &buffer,
                       QVariantMap &message,
                       QString *error)
{
    if (error)
        error->clear();
    message.clear();

    if (buffer.size() < 4)
        return DecodeResult::Incomplete;

    const quint32 payloadSize = qFromBigEndian<quint32>(
        reinterpret_cast<const uchar *>(buffer.constData()));
    if (payloadSize > kMaxFrameBytes) {
        if (error)
            *error = QStringLiteral("IPC frame exceeds the 8 MiB limit");
        buffer.clear();
        return DecodeResult::Error;
    }

    const qsizetype frameSize = 4 + static_cast<qsizetype>(payloadSize);
    if (buffer.size() < frameSize)
        return DecodeResult::Incomplete;

    const QByteArray payload = buffer.mid(4, payloadSize);
    buffer.remove(0, frameSize);

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (error) {
            *error = parseError.error == QJsonParseError::NoError
                ? QStringLiteral("IPC frame root is not a JSON object")
                : QStringLiteral("Invalid IPC JSON: %1").arg(parseError.errorString());
        }
        return DecodeResult::Error;
    }

    message = document.object().toVariantMap();
    return DecodeResult::Frame;
}

} // namespace LfQuick::IpcFrameCodec
