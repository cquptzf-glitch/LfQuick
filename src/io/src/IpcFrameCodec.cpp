#include "IpcFrameCodec_p.h"

#include <QIODevice>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QtEndian>

#include <algorithm>

namespace LfQuick::IpcFrameCodec {
namespace {
constexpr qsizetype kMaxFrameBytes = 8 * 1024 * 1024;
constexpr qsizetype kFrameHeaderBytes = 4;
constexpr qsizetype kReadChunkBytes = 64 * 1024;

bool nonEmptyString(const QVariantMap &message, const QString &key)
{
    return !message.value(key).toString().trimmed().isEmpty();
}

} // namespace

qsizetype maximumPayloadBytes() noexcept
{
    return kMaxFrameBytes;
}

qsizetype maximumBufferedBytes() noexcept
{
    return kFrameHeaderBytes + kMaxFrameBytes;
}

qsizetype recommendedReadChunkBytes() noexcept
{
    return kReadChunkBytes;
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

    QByteArray frame(kFrameHeaderBytes, Qt::Uninitialized);
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

    if (buffer.size() < kFrameHeaderBytes)
        return DecodeResult::Incomplete;

    const quint32 payloadSize = qFromBigEndian<quint32>(
        reinterpret_cast<const uchar *>(buffer.constData()));
    if (payloadSize > static_cast<quint32>(kMaxFrameBytes)) {
        if (error)
            *error = QStringLiteral("IPC frame exceeds the 8 MiB limit");
        buffer.clear();
        return DecodeResult::Error;
    }

    const qsizetype frameSize = kFrameHeaderBytes + static_cast<qsizetype>(payloadSize);
    if (buffer.size() < frameSize)
        return DecodeResult::Incomplete;

    const QByteArray payload = buffer.mid(kFrameHeaderBytes, payloadSize);
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

bool validateTypedEnvelope(const QVariantMap &message, QString *error)
{
    if (error)
        error->clear();

    const QString type = message.value(QStringLiteral("type")).toString();
    if (type.isEmpty())
        return true; // Generic untyped message.

    const auto fail = [error](const QString &messageText) {
        if (error)
            *error = messageText;
        return false;
    };

    if (type == QStringLiteral("request")) {
        if (!nonEmptyString(message, QStringLiteral("id")))
            return fail(QStringLiteral("IPC request is missing a non-empty id"));
        if (!nonEmptyString(message, QStringLiteral("method")))
            return fail(QStringLiteral("IPC request is missing a non-empty method"));
        if (message.contains(QStringLiteral("params"))
            && !message.value(QStringLiteral("params")).canConvert<QVariantMap>()) {
            return fail(QStringLiteral("IPC request params must be an object"));
        }
        return true;
    }

    if (type == QStringLiteral("notification")) {
        if (!nonEmptyString(message, QStringLiteral("method")))
            return fail(QStringLiteral("IPC notification is missing a non-empty method"));
        if (message.contains(QStringLiteral("params"))
            && !message.value(QStringLiteral("params")).canConvert<QVariantMap>()) {
            return fail(QStringLiteral("IPC notification params must be an object"));
        }
        return true;
    }

    if (type == QStringLiteral("response")) {
        if (!nonEmptyString(message, QStringLiteral("id")))
            return fail(QStringLiteral("IPC response is missing a non-empty id"));

        const bool hasResult = message.contains(QStringLiteral("result"));
        const bool hasError = message.contains(QStringLiteral("error"));
        if (hasResult == hasError) {
            return fail(QStringLiteral(
                "IPC response must contain exactly one of result or error"));
        }
        if (hasError) {
            const QVariant errorValue = message.value(QStringLiteral("error"));
            if (!errorValue.canConvert<QVariantMap>())
                return fail(QStringLiteral("IPC response error must be an object"));
            const QVariantMap errorMap = errorValue.toMap();
            if (errorMap.value(QStringLiteral("message")).toString().trimmed().isEmpty()) {
                return fail(QStringLiteral(
                    "IPC response error is missing a non-empty message"));
            }
        }
        return true;
    }

    // Custom typed messages remain valid for sendMessage()/messageReceived().
    return true;
}

bool writeAll(QIODevice *device,
              const QByteArray &data,
              QString *error)
{
    if (error)
        error->clear();
    if (!device || !device->isWritable()) {
        if (error)
            *error = QStringLiteral("IPC device is not writable");
        return false;
    }

    qsizetype offset = 0;
    while (offset < data.size()) {
        const qint64 written = device->write(
            data.constData() + offset,
            static_cast<qint64>(data.size() - offset));
        if (written <= 0) {
            if (error) {
                const QString deviceError = device->errorString().trimmed();
                *error = deviceError.isEmpty()
                    ? QStringLiteral("IPC device accepted a partial frame and stopped writing")
                    : deviceError;
            }
            return false;
        }
        offset += static_cast<qsizetype>(written);
    }
    return true;
}

} // namespace LfQuick::IpcFrameCodec
