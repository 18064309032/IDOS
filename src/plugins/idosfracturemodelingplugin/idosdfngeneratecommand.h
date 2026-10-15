#ifndef IDOS_DFN_GENERATE_COMMAND_H
#define IDOS_DFN_GENERATE_COMMAND_H

#include <QJsonObject>
#include <QPointer>

#include "idoscommand.h"
#include "idoscommandmetadata.h"

class IDOSNaturalFractureSet;

class IDOSDfnGenerateCommand : public IDOSCommand
{
  public:
    IDOSDfnGenerateCommand(IDOSProject* project, const QJsonObject& parameters);
    ~IDOSDfnGenerateCommand() override;
    static int liveCount();

  protected:
    bool validateCommand(QString& error) const override;
    QJsonObject buildPreview() const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

  private:
    static int s_liveCount;
    QJsonObject m_parameters;
    QPointer<IDOSNaturalFractureSet> m_data;
};

class IDOSDfnGenerateCommandMetadata : public IDOSCommandMetadata
{
  public:
    IDOSDfnGenerateCommandMetadata();
    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments, IDOSProject* project) const override;
};

#endif // IDOS_DFN_GENERATE_COMMAND_H
