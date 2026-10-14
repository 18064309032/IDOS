#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPlainTextEdit>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QVariant>
#include <QVariantMap>
#include <QVBoxLayout>

#include "data/case/idossimulationcaseobject.h"

#include "idossimulationdeckbrowserwidget.h"

IDOSSimulationDeckBrowserWidget::IDOSSimulationDeckBrowserWidget(IDOSSimulationCaseObject* simulationCase,
                                                                 QWidget* parent)
    :
    QWidget(parent, Qt::Window)
    , m_filter(nullptr)
    , m_keywordList(nullptr)
    , m_keywordText(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName(QStringLiteral("simulationDeckBrowserWindow"));
    setWindowTitle(tr("Deck Browser — %1").arg(simulationCase->name()));

    const QString sourceFile = simulationCase->sourceFile();
    const QVariantList keywordIndex =
        simulationCase->parsedSourceData().value(QStringLiteral("keywordIndex")).toList();
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* sourceLabel = new QLabel(sourceFile, this);
    sourceLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(sourceLabel);

    m_filter = new QLineEdit(this);
    m_filter->setPlaceholderText(tr("Filter by keyword name"));
    layout->addWidget(m_filter);

    QHBoxLayout* contentLayout = new QHBoxLayout();
    m_keywordList = new QListWidget(this);
    m_keywordText = new QPlainTextEdit(this);
    m_keywordText->setReadOnly(true);
    contentLayout->addWidget(m_keywordList, 1);
    contentLayout->addWidget(m_keywordText, 3);
    layout->addLayout(contentLayout);

    for (const QVariant& value : keywordIndex)
    {
        const QVariantMap keyword = value.toMap();
        const QString name = keyword.value(QStringLiteral("name")).toString();
        const QString file = keyword.value(QStringLiteral("file")).toString();
        const qulonglong line = keyword.value(QStringLiteral("line")).toULongLong();
        const QString display = QStringLiteral("%1  —  %2:%3").arg(name, file).arg(line);
        QListWidgetItem* item = new QListWidgetItem(display, m_keywordList);
        item->setData(Qt::UserRole, file);
        item->setData(Qt::UserRole + 1, name);
        item->setData(Qt::UserRole + 2, line);
        item->setData(Qt::UserRole + 3, keyword.value(QStringLiteral("endLine")));
    }

    connect(m_filter,
            &QLineEdit::textChanged,
            this,
            &IDOSSimulationDeckBrowserWidget::onFilterTextChanged);
    connect(m_keywordList,
            &QListWidget::currentItemChanged,
            this,
            &IDOSSimulationDeckBrowserWidget::onKeywordSelectionChanged);

    resize(1000, 680);
    m_keywordList->setCurrentRow(0);
}

IDOSSimulationDeckBrowserWidget::~IDOSSimulationDeckBrowserWidget()
{
}

void IDOSSimulationDeckBrowserWidget::onFilterTextChanged(const QString& text)
{
    QListWidgetItem* firstVisibleItem = nullptr;
    for (int index = 0; index < m_keywordList->count(); ++index)
    {
        QListWidgetItem* item = m_keywordList->item(index);
        const bool visible = item->data(Qt::UserRole + 1).toString().contains(text, Qt::CaseInsensitive);
        item->setHidden(!visible);
        if (visible && firstVisibleItem == nullptr)
        {
            firstVisibleItem = item;
        }
    }
    if (m_keywordList->currentItem() == nullptr || m_keywordList->currentItem()->isHidden())
    {
        m_keywordList->setCurrentItem(firstVisibleItem);
    }
}

void IDOSSimulationDeckBrowserWidget::onKeywordSelectionChanged()
{
    QListWidgetItem* item = m_keywordList->currentItem();
    if (item == nullptr)
    {
        m_keywordText->clear();
        return;
    }

    const QString filePath = item->data(Qt::UserRole).toString();
    const qulonglong startLine = item->data(Qt::UserRole + 2).toULongLong();
    const qulonglong endLine = item->data(Qt::UserRole + 3).toULongLong();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        m_keywordText->setPlainText(tr("Unable to open keyword source file: %1").arg(file.errorString()));
        return;
    }

    QTextStream stream(&file);
    qulonglong currentLine = 1;
    while (!stream.atEnd() && currentLine < startLine)
    {
        stream.readLine();
        ++currentLine;
    }
    if (currentLine != startLine)
    {
        m_keywordText->setPlainText(tr("Unable to read keyword at %1:%2").arg(filePath).arg(startLine));
        return;
    }

    QStringList lines;
    while (!stream.atEnd() && (endLine == 0 || currentLine <= endLine))
    {
        lines.append(stream.readLine());
        ++currentLine;
    }
    m_keywordText->setPlainText(lines.join(QLatin1Char('\n')));
}
