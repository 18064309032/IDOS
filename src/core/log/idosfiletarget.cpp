#include <QDir>
#include <QFileInfo>

#include "idosloglevel.h"
#include "idoslogrecord.h"

#include "idosfiletarget.h"

IDOSFileTarget::IDOSFileTarget(const QString& filePath, qint64 maxSize, int maxBackups)
    : m_filePath(filePath)
    , m_maxSize(maxSize)
    , m_maxBackups(maxBackups)
{
    QDir dir = QFileInfo(filePath).absoluteDir();
    if (!dir.exists())
    {
        dir.mkpath(QStringLiteral("."));
    }

    m_file.setFileName(filePath);
    if (m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        m_stream.setDevice(&m_file);
    }
}

IDOSFileTarget::~IDOSFileTarget()
{
    if (m_file.isOpen())
    {
        m_file.close();
    }
}

void IDOSFileTarget::write(const IDOSLogRecord& record)
{
    if (!m_file.isOpen())
    {
        return;
    }

    if (m_file.size() > m_maxSize)
    {
        rotateFile();
    }

    QFileInfo fileInfo(record.file());
    QString filePart = fileInfo.fileName();

    QString formatted =
        QStringLiteral("[%1]\t[%2]\t%3:%4\t%5\t%6")
            .arg(record.time().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz")),
                 idosLogLevelToString(record.level()),
                 filePart)
            .arg(record.line())
            .arg(record.function(), record.message());

    m_stream << formatted << "\n";
    m_stream.flush();
}

void IDOSFileTarget::flush()
{
    m_stream.flush();
}

void IDOSFileTarget::rotateFile()
{
    m_stream.flush();
    m_file.close();

    QString backupFile = QStringLiteral("%1.%2").arg(m_filePath).arg(m_maxBackups);
    if (QFile::exists(backupFile))
    {
        QFile::remove(backupFile);
    }

    int index = m_maxBackups - 1;
    while (index >= 1)
    {
        QString oldName = QStringLiteral("%1.%2").arg(m_filePath).arg(index);
        QString newName = QStringLiteral("%1.%2").arg(m_filePath).arg(index + 1);
        if (QFile::exists(oldName))
        {
            QFile::rename(oldName, newName);
        }
        --index;
    }

    QString firstBackup = QStringLiteral("%1.1").arg(m_filePath);
    QFile::rename(m_filePath, firstBackup);

    if (m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        m_stream.setDevice(&m_file);
    }
}
