#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include "idosmainwindow.h"
#include "idosproject.h"
#include "case/idoscaseitemref.h"
#include "case/idossimulationcaseobject.h"
#include "data/grid/idosgrid.h"
#include "data/grid/idosgridproperty.h"
#include "data/well/idoswell.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 加载翻译：优先 exe 同目录（发布布局），其次构建目录 i18n/（开发布局）
    auto* translator = new QTranslator(&app);
    const QString exeDir = QCoreApplication::applicationDirPath();
    if (translator->load(QLocale(), QStringLiteral("idos"), QStringLiteral("_"), exeDir)
        || translator->load(QLocale(), QStringLiteral("idos"), QStringLiteral("_"),
                            exeDir + QStringLiteral("/../../i18n"))) {
        app.installTranslator(translator);
    }

    // 演示工程：网格（含属性）进模型树，井（带射孔/轨迹）进输入树"井组"
    IDOSProject project;

    auto* grid = new IDOSGrid(&project);
    grid->setName(QStringLiteral("Demo Grid"));
    grid->setDimensions(10, 10, 5);
    project.addObject(grid);

    auto* permx = new IDOSGridProperty(&project);
    permx->setName(QStringLiteral("Permeability X"));
    permx->setKeyword(QStringLiteral("PERMX"));
    permx->setGridId(grid->objectId());          // containerId → 挂网格子树
    permx->setDimensions(10, 10, 5);
    QVector<double> permValues(10 * 10 * 5, 100.0);
    permx->setValues(permValues);
    project.addObject(permx);                    // 模型树：网格 → 属性 → 静态属性 → PERMX

    auto* well = new IDOSWell(&project);
    well->setName(QStringLiteral("Well-1"));
    well->addCompletion(IDOSWellCompletion());
    well->addSegment(IDOSWellSegment());
    project.addObject(well);

    // 模拟工况：按 objectId 引用网格与井（引用 = 配料清单，数据本体不搬家）
    auto* simCase = new IDOSSimulationCaseObject(&project);
    simCase->setName(QStringLiteral("Demo Case"));
    simCase->addItemRef(IDOSCaseItemRef(QStringLiteral("case.grid"), grid->objectId()));
    simCase->addItemRef(IDOSCaseItemRef(QStringLiteral("case.well"), well->objectId()));
    project.addObject(simCase);

    IDOSMainWindow window;
    window.setProject(&project);
    window.show();

    return app.exec();
}
