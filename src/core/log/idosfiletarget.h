#ifndef IDOS_FILETARGET_H
#define IDOS_FILETARGET_H

#include <QFile>
#include <QString>
#include <QTextStream>

#include "idos_core.h"
#include "idoslogtarget.h"

class CORE_EXPORT IDOSFileTarget : public IDOSLogTarget
{
  public:
    IDOSFileTarget(const QString& filePath,
                   qint64 maxSize = 10 * 1024 * 1024,
                   int maxBackups = 5);
    ~IDOSFileTarget() override;

    void write(const IDOSLogRecord& record) override;
    void flush() override;
    void rotateFile();

  private:
    QFile m_file;
    QString m_filePath;
    qint64 m_maxSize;
    int m_maxBackups;
    QTextStream m_stream;
};

#endif // IDOS_FILETARGET_H
