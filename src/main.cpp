#include <QLocale>
#include <QTranslator>

#include "idosapplication.h"
#include "idosmainwindow.h"

int main(int argc, char* argv[])
{
    IDOSApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    IDOSApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    IDOSApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    IDOSApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    IDOSApplication application(argc, argv);

    QTranslator* translator = new QTranslator(&application);
    const QString exeDir = QCoreApplication::applicationDirPath();
    if (translator->load(QLocale(), QStringLiteral("idos"), QStringLiteral("_"), exeDir) ||
        translator->load(QLocale(), QStringLiteral("idos"), QStringLiteral("_"),
                         exeDir + QStringLiteral("/../../i18n")))
    {
        application.installTranslator(translator);
    }

    IDOSMainWindow window;
    window.showMaximized();

    return application.exec();
}
