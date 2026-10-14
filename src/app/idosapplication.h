#ifndef IDOS_APPLICATION_H
#define IDOS_APPLICATION_H

#include <QApplication>

#include "idos_app.h"

class IDOSDataRegistry;

class APP_EXPORT IDOSApplication : public QApplication
{
    Q_OBJECT

  public:
    IDOSApplication(int& argc, char** argv);
    ~IDOSApplication() override;

    IDOSDataRegistry* dataRegistry() const;
    static void initProviders();
    static void initDataRegistry(IDOSDataRegistry* registry);

  private:
    IDOSDataRegistry* m_dataRegistry;

    static void handleQtMessage(QtMsgType type,
                                const QMessageLogContext& context,
                                const QString& message);
};

#ifdef qApp
#undef qApp
#endif
#define qApp (static_cast<IDOSApplication*>(QApplication::instance()))

#endif // IDOS_APPLICATION_H
