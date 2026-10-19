#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTemporaryDir>
#include <QVBoxLayout>
#include <QVariantMap>

#include "case/idossimulationcaseeclipseprovider.h"
#include "data/case/idossimulationcaseobject.h"
#include "idosproject.h"
#include "logger/idoslogger.h"

#include "idossimulationdatagenerationwidget.h"

IDOSSimulationDataGenerationWidget::IDOSSimulationDataGenerationWidget(IDOSSimulationCaseObject* simulationCase,
                                                                       QWidget* parent)
    :
    QWidget(parent)
    , m_caseName(simulationCase != nullptr ? simulationCase->name() : QString())
    , m_sourceFile(simulationCase != nullptr ? simulationCase->sourceFile() : QString())
    , m_simulationCase(simulationCase)
    , m_parentDirectoryEdit(nullptr)
    , m_dataEditor(nullptr)
    , m_outputEdit(nullptr)
{
    setObjectName(QStringLiteral("simulationDataGenerationPage"));
    setStyleSheet(QStringLiteral(
        "QPlainTextEdit { border: 1px solid #d8e0e9; border-radius: 3px; background: #fbfcfe; }"
        "QLineEdit { min-height: 26px; }"
        "QPushButton { min-height: 28px; padding: 2px 10px; }"));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(7);
    QLabel* descriptionLabel = new QLabel(
        tr("Generate a standalone DATA file from the case's current in-memory grid and static properties. "
           "Other simulator settings are retained from the source DATA file."),
        this);
    descriptionLabel->setWordWrap(true);
    layout->addWidget(descriptionLabel);

    m_dataEditor = new QPlainTextEdit(this);
    m_dataEditor->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_dataEditor->setReadOnly(true);
    m_dataEditor->setPlaceholderText(tr("Generate DATA to preview the current in-memory grid and properties."));
    layout->addWidget(m_dataEditor, 1);

    QHBoxLayout* directoryLayout = new QHBoxLayout();
    directoryLayout->addWidget(new QLabel(tr("Output parent directory"), this));
    m_parentDirectoryEdit = new QLineEdit(QFileInfo(m_sourceFile).absolutePath(), this);
    directoryLayout->addWidget(m_parentDirectoryEdit, 1);
    QPushButton* browseButton = new QPushButton(tr("Browse…"), this);
    directoryLayout->addWidget(browseButton);
    layout->addLayout(directoryLayout);
    connect(browseButton, &QPushButton::clicked, this,
            &IDOSSimulationDataGenerationWidget::onBrowseParentDirectoryTriggered);

    m_outputEdit = new QPlainTextEdit(this);
    m_outputEdit->setReadOnly(true);
    m_outputEdit->setMaximumHeight(90);
    layout->addWidget(m_outputEdit);

    QPushButton* generateButton = new QPushButton(tr("Generate DATA File"), this);
    layout->addWidget(generateButton);
    connect(generateButton, &QPushButton::clicked, this,
            &IDOSSimulationDataGenerationWidget::onGeneratePackageTriggered);
}

void IDOSSimulationDataGenerationWidget::onBrowseParentDirectoryTriggered()
{
    const QString selectedDirectory =
        QFileDialog::getExistingDirectory(this, tr("Select output parent directory"), m_parentDirectoryEdit->text());
    if (!selectedDirectory.isEmpty())
    {
        m_parentDirectoryEdit->setText(selectedDirectory);
    }
}

void IDOSSimulationDataGenerationWidget::onGeneratePackageTriggered()
{
    IDOS_MESSAGE(tr("Generating DATA file from case memory."), IDOSLogLevel::Info);
    QString packagePath;
    QString errorMessage;
    if (!writePackage(m_parentDirectoryEdit->text(), &packagePath, &errorMessage))
    {
        showOutput(errorMessage);
        return;
    }

    const QString dataFilePath = QDir(packagePath).absoluteFilePath(generatedMainFileName());
    QFile generatedFile(dataFilePath);
    if (generatedFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        m_dataEditor->setPlainText(QString::fromUtf8(generatedFile.readAll()));
    }
    showOutput(tr("DATA file created and validated: %1").arg(dataFilePath), IDOSLogLevel::Info);
    if (m_simulationCase != nullptr)
    {
        QVariantMap runInfo = m_simulationCase->runInfo();
        runInfo.insert(QStringLiteral("generatedDataFile"), dataFilePath);
        runInfo.insert(QStringLiteral("generatedAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
        m_simulationCase->setRunInfo(runInfo);
    }
}

QString IDOSSimulationDataGenerationWidget::generatedMainFileName() const
{
    const QFileInfo sourceInfo(m_sourceFile);
    QString extension = sourceInfo.suffix();
    if (extension.isEmpty())
    {
        extension = QStringLiteral("DATA");
    }
    return QStringLiteral("%1_generated.%2").arg(sourceInfo.completeBaseName(), extension);
}

bool IDOSSimulationDataGenerationWidget::writePackage(const QString& parentDirectory, QString* packagePath,
                                                      QString* errorMessage) const
{
    const QFileInfo sourceInfo(m_sourceFile);
    const QFileInfo parentInfo(parentDirectory);
    if (m_simulationCase == nullptr)
    {
        *errorMessage = tr("The simulation case is no longer available.");
        return false;
    }
    if (!sourceInfo.isFile() || !sourceInfo.isReadable())
    {
        *errorMessage = tr("The source DATA file is missing or unreadable.");
        return false;
    }
    if (!parentInfo.isDir() || !parentInfo.isWritable())
    {
        *errorMessage = tr("The output parent directory is missing or not writable.");
        return false;
    }

    QString safeCaseName = m_caseName;
    const QString invalidCharacters = QStringLiteral("<>:\"/\\|?*");
    for (const QChar character : invalidCharacters)
    {
        safeCaseName.replace(character, QLatin1Char('_'));
    }
    if (safeCaseName.trimmed().isEmpty())
    {
        safeCaseName = tr("Simulation Case");
    }
    const QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"));
    const QString directoryName = QStringLiteral("%1_DATA_%2").arg(safeCaseName, stamp);
    const QString destinationDirectory = QDir(parentDirectory).absoluteFilePath(directoryName);
    if (QFileInfo::exists(destinationDirectory))
    {
        *errorMessage = tr("The output directory already exists. Try again in a moment.");
        return false;
    }

    QTemporaryDir temporaryDirectory(QDir(parentDirectory).absoluteFilePath(QStringLiteral(".idos_data_XXXXXX")));
    if (!temporaryDirectory.isValid())
    {
        *errorMessage = tr("Unable to create a temporary DATA output directory.");
        return false;
    }

    IDOSProject* project = qobject_cast<IDOSProject*>(m_simulationCase->parent());
    IDOSSimulationCaseEclipseProvider provider;
    if (!provider.writeMemoryData(m_simulationCase, project, temporaryDirectory.path(), generatedMainFileName(),
                                  errorMessage))
    {
        return false;
    }

    temporaryDirectory.setAutoRemove(false);
    if (!QDir().rename(temporaryDirectory.path(), destinationDirectory))
    {
        QDir(temporaryDirectory.path()).removeRecursively();
        *errorMessage = tr("Unable to finalize the DATA output directory.");
        return false;
    }
    *packagePath = destinationDirectory;
    return true;
}

void IDOSSimulationDataGenerationWidget::showOutput(const QString& message, IDOSLogLevel level)
{
    m_outputEdit->setPlainText(message);
    const QStringList lines = message.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString& line : lines)
    {
        IDOSLogger::instance().message(line, level);
    }
}
