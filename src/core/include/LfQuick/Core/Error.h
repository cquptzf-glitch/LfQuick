
#pragma once

#include <LfQuick/Core/LfQuickCoreGlobal.h>

#include <QString>
#include <QVariantMap>

namespace LfQuick {

class LFQUICKCORE_EXPORT Error
{
public:
    Error() = default;
    Error(QString domain, int code, QString message, QVariantMap details = {});

    bool isValid() const noexcept { return m_code != 0 || !m_message.isEmpty(); }
    const QString &domain() const noexcept { return m_domain; }
    int code() const noexcept { return m_code; }
    const QString &message() const noexcept { return m_message; }
    const QVariantMap &details() const noexcept { return m_details; }

private:
    QString m_domain;
    int m_code = 0;
    QString m_message;
    QVariantMap m_details;
};

} // namespace LfQuick
