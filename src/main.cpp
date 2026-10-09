#include <QCoreApplication>
#include <QLocale>
#include <QObject>
#include <QSharedPointer>
#include <QTranslator>

#include "log/idoslogger.h"
#include "log/idosfiletarget.h"
#include "idosapplication.h"
#include "idosmainwindow.h"

int main(int argc, char* argv[])
{
    IDOSApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    IDOSApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    IDOSApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    IDOSApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    IDOSApplication application(argc, argv);

    IDOSLogger::instance().addTarget(QSharedPointer<IDOSLogTarget>(new IDOSFileTarget("idos_log.txt")));

    QTranslator* translator = new QTranslator(&application);
    const QString exeDir = QCoreApplication::applicationDirPath();
    const QString translationDirectory = exeDir + QStringLiteral("/i18n");
    if (translator->load(QLocale(), QStringLiteral("idos"), QStringLiteral("_"),
                         translationDirectory) ||
        translator->load(QLocale(), QStringLiteral("idos"), QStringLiteral("_"), exeDir) ||
        translator->load(QLocale(), QStringLiteral("idos"), QStringLiteral("_"),
                         exeDir + QStringLiteral("/../../i18n")))
    {
        application.installTranslator(translator);
        IDOS_INFO(QObject::tr("Application translation loaded."));
    }
    else
    {
        IDOS_WARN(QObject::tr("Application translation could not be loaded."));
    }
    IDOS_MESSAGE(QObject::tr("Application startup is beginning."), IDOSLogLevel::Info);

    IDOSMainWindow window;
    window.showMaximized();

    const int exitCode = application.exec();
    IDOS_INFO(QObject::tr("Application event loop ended with exit code %1.").arg(exitCode));
    return exitCode;
}
