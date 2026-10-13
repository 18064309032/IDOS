#include "idosdataprovider.h"

QString IDOSDataProvider::lastError() const
{
    return m_lastError;
}

QStringList IDOSDataProvider::splitTextFields(const QString& line, bool& valid) const
{
    QStringList fields;
    QString token;
    bool quoted = false;
    bool started = false;
    for (int index = 0; index < line.size(); ++index)
    {
        const QChar character = line.at(index);
        if (character == QLatin1Char('"'))
        {
            if (quoted && index + 1 < line.size() && line.at(index + 1) == QLatin1Char('"'))
            {
                token.append(character);
                ++index;
            }
            else
            {
                quoted = !quoted;
            }
            started = true;
        }
        else if (!quoted && character.isSpace())
        {
            if (started)
            {
                fields.append(token);
                token.clear();
                started = false;
            }
        }
        else
        {
            token.append(character);
            started = true;
        }
    }
    if (started)
    {
        fields.append(token);
    }
    valid = !quoted;
    return fields;
}

void IDOSDataProvider::setLastError(const QString& msg)
{
    m_lastError = msg;
}

IDOSDataProvider::~IDOSDataProvider()
{
}
