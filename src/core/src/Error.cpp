
#include <LfQuick/Core/Error.h>

#include <utility>

namespace LfQuick {

Error::Error(QString domain, int code, QString message, QVariantMap details)
    : m_domain(std::move(domain))
    , m_code(code)
    , m_message(std::move(message))
    , m_details(std::move(details))
{
}

} // namespace LfQuick
