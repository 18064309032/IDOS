#ifndef IDOS_COMMAND_H
#define IDOS_COMMAND_H

#include <QJsonObject>
#include <QString>
#include <QUndoCommand>

#include "idos_core.h"

class IDOSProject;

/**
 * @brief Reversible project operation.
 */
class CORE_EXPORT IDOSCommand : public QUndoCommand
{
public:
    enum class Type
    {
        Action,
        Query
    };

    IDOSCommand(QString name,
                Type type,
                IDOSProject& project,
                QString text = QString(),
                QUndoCommand* parent = nullptr);
    ~IDOSCommand() override;

    const QString& name() const;
    Type type() const;
    IDOSProject* project() const;

    bool validate(QString& error) const;
    QJsonObject preview() const;
    bool isSuccessful() const;
    const QString& errorCode() const;
    const QString& errorString() const;
    const QJsonObject& result() const;

    void undo() final;
    void redo() final;

protected:
    virtual bool validateCommand(QString& error) const;
    virtual QJsonObject buildPreview() const;
    virtual bool apply(QString& error) = 0;
    virtual bool revert(QString& error) = 0;

    void setResult(QJsonObject result);

private:
    void setError(QString errorCode, QString errorString);

    QString m_name;
    Type m_type;
    IDOSProject* m_project;
    QString m_errorCode;
    QString m_errorString;
    QJsonObject m_result;
};

#endif // IDOS_COMMAND_H
