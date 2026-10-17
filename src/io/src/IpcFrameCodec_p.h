#pragma once

#include <QByteArray>
#include <QString>
#include <QVariantMap>

class QIODevice;

namespace LfQuick::IpcFrameCodec {

enum class DecodeResult {
    Incomplete,
    Frame,
    Error
};

qsizetype maximumPayloadBytes() noexcept;
qsizetype maximumBufferedBytes() noexcept;
qsizetype recommendedReadChunkBytes() noexcept;

QByteArray encode(const QVariantMap &message, QString *error = nullptr);
DecodeResult takeFrame(QByteArray &buffer,
                       QVariantMap &message,
                       QString *error = nullptr);

// Generic messages are allowed. Known RPC envelope types are validated so a
// malformed request/response never silently enters the typed signal path.
bool validateTypedEnvelope(const QVariantMap &message,
                           QString *error = nullptr);

// QIODevice::write() may legally accept fewer bytes than requested. Queue the
// full already-encoded frame or fail explicitly instead of dropping a suffix.
bool writeAll(QIODevice *device,
              const QByteArray &data,
              QString *error = nullptr);

} // namespace LfQuick::IpcFrameCodec
