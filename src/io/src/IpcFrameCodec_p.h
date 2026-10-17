#pragma once

#include <QByteArray>
#include <QString>
#include <QVariantMap>

namespace LfQuick::IpcFrameCodec {

enum class DecodeResult {
    Incomplete,
    Frame,
    Error
};

QByteArray encode(const QVariantMap &message, QString *error = nullptr);
DecodeResult takeFrame(QByteArray &buffer,
                       QVariantMap &message,
                       QString *error = nullptr);

} // namespace LfQuick::IpcFrameCodec
