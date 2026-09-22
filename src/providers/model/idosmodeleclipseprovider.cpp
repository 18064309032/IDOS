#include "idosmodeleclipseprovider.h"
#include "data/model/idosmodel.h"

#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>

#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <vector>

QList<IDOSDataObject*> IDOSModelEclipseProvider::read(const QString& filePath)
{
    QList<IDOSDataObject*> result;

    std::string dataFile = filePath.toStdString();

    try
    {
        Opm::Parser parser;
        Opm::Deck deck = parser.parseFile(dataFile);

        // ① 工况名：TITLE 关键字首条记录的 item 0；缺失回退到文件名
        QString caseName;
        std::vector<const Opm::DeckKeyword*> titles = deck.getKeywordList("TITLE");
        if (!titles.empty())
        {
            const Opm::DeckKeyword* titleKw = titles.front();
            if (titleKw->size() > 0)
            {
                const Opm::DeckRecord& rec = titleKw->getRecord(0);
                if (rec.size() > 0)
                {
                    caseName = QString::fromStdString(rec.getItem(0).getTrimmedString(0));
                }
            }
        }
        if (caseName.isEmpty())
        {
            caseName = QFileInfo(filePath).completeBaseName();
        }

        // ② 井名清单：遍历所有 WELSPECS 关键字（SCHEDULE 段可多次出现）
        QStringList wellNames;
        std::vector<const Opm::DeckKeyword*> welspecs = deck.getKeywordList("WELSPECS");
        for (const Opm::DeckKeyword* kw : welspecs)
        {
            if (kw == nullptr) continue;
            for (std::size_t r = 0; r < kw->size(); ++r)
            {
                const Opm::DeckRecord& rec = kw->getRecord(r);
                if (rec.size() == 0) continue;
                std::string name = rec.getItem(0).getTrimmedString(0);
                if (!name.empty())
                {
                    wellNames.append(QString::fromStdString(name));
                }
            }
        }
        wellNames.removeDuplicates();

        IDOSModel* model = new IDOSModel();
        model->setName(caseName);
        model->setPendingWellNames(wellNames);
        // TODO: gridId 解析——.DATA 的 GRID 段常 INCLUDE 一个 .EGRID 路径，
        //   需走 INCLUDE 关键字找相对路径再 link 到 Project 里已导入的网格。
        //   复杂度高，留后续做；当前 gridId 留空，由 GUI/手动 link 解决。

        result.append(model);
    }
    catch (const std::exception& e)
    {
        setLastError(QStringLiteral("OPM parse .DATA failed: %1")
                         .arg(QString::fromStdString(e.what())));
    }

    return result;
}
