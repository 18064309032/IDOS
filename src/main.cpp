#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include "idosmainwindow.h"
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 加载翻译：优先 exe 同目录（发布布局），其次构建目录 i18n/（开发布局）
    QTranslator* translator = new QTranslator(&app);
    const QString exeDir = QCoreApplication::applicationDirPath();
    if (translator->load(QLocale(), QStringLiteral("idos"), QStringLiteral("_"), exeDir) ||
        translator->load(QLocale(), QStringLiteral("idos"), QStringLiteral("_"),
                         exeDir + QStringLiteral("/../../i18n")))
    {
        app.installTranslator(translator);
    }

    IDOSMainWindow window;
    window.show();

    return app.exec();
}
