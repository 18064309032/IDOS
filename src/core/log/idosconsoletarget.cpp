#include "idosconsoletarget.h"

#include <iostream>

#include <QByteArray>
#include <QString>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "idosloglevel.h"
#include "idoslogrecord.h"

#if defined(ERROR)
#undef ERROR
#endif

IDOSConsoleTarget::IDOSConsoleTarget(bool enableColor)
    : m_enableColor(enableColor)
{
}

IDOSConsoleTarget::~IDOSConsoleTarget()
{
}

void IDOSConsoleTarget::write(const IDOSLogRecord& record)
{
    QString formatted =
        QStringLiteral("[%1] [%2] %3")
            .arg(record.time().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz")),
                 idosLogLevelToString(record.level()),
                 record.message());

    if (m_enableColor)
    {
#ifdef Q_OS_WIN
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE)
        {
            SetConsoleOutputCP(CP_UTF8);

            CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
            GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
            WORD originalAttributes = consoleInfo.wAttributes;

            WORD color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

            switch (record.level())
            {
                case IDOSLogLevel::Trace:
                    color = FOREGROUND_INTENSITY;
                    break;
                case IDOSLogLevel::Debug:
                    color = FOREGROUND_BLUE | FOREGROUND_GREEN;
                    break;
                case IDOSLogLevel::Info:
                    color = FOREGROUND_GREEN;
                    break;
                case IDOSLogLevel::Warn:
                    color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
                    break;
                case IDOSLogLevel::Error:
                    color = FOREGROUND_RED | FOREGROUND_INTENSITY;
                    break;
                case IDOSLogLevel::Fatal:
                    color = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
                    break;
                default:
                    break;
            }

            SetConsoleTextAttribute(hConsole, color);
            const QByteArray outputBytes = formatted.toUtf8();
            std::cout << outputBytes.constData() << '\n';
            SetConsoleTextAttribute(hConsole, originalAttributes);
        }
#else
        const char* colorCode = "";
        const char* resetCode = "\033[0m";

        switch (record.level())
        {
            case IDOSLogLevel::Trace:
                colorCode = "\033[37m";
                break;
            case IDOSLogLevel::Debug:
                colorCode = "\033[36m";
                break;
            case IDOSLogLevel::Info:
                colorCode = "\033[32m";
                break;
            case IDOSLogLevel::Warn:
                colorCode = "\033[33m";
                break;
            case IDOSLogLevel::Error:
                colorCode = "\033[31m";
                break;
            case IDOSLogLevel::Fatal:
                colorCode = "\033[35m";
                break;
            default:
                break;
        }

        std::cout << colorCode << formatted.toLocal8Bit().constData() << resetCode << std::endl;
#endif
    }
    else
    {
#ifdef Q_OS_WIN
        std::cout << formatted.toUtf8().constData() << std::endl;
#else
        std::cout << formatted.toLocal8Bit().constData() << std::endl;
#endif
    }
}

void IDOSConsoleTarget::flush()
{
    std::cout.flush();
}
