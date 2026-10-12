#include <QByteArray>
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
#include <QSaveFile>
#include <QSet>
#include <QTemporaryDir>
#include <QVariantList>
#include <QVariantMap>
#include <QVBoxLayout>

#include "idossimulationcaseobject.h"
#include "log/idoslogger.h"

#include "idossimulationdatagenerationwidget.h"

IDOSSimulationDataGenerationWidget::IDOSSimulationDataGenerationWidget(IDOSSimulationCaseObject* simulationCase,
                                                                       QWidget* parent)
    :
    QWidget(parent, Qt::Window)
    , m_caseName(simulationCase != nullptr ? simulationCase->name() : QString())
    , m_sourceFile(simulationCase != nullptr ? simulationCase->sourceFile() : QString())
    , m_parentDirectoryEdit(nullptr)
    , m_dataEditor(nullptr)
    , m_outputEdit(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName(QStringLiteral("simulationDataGenerationWindow"));
    setWindowTitle(tr("Generate DATA — %1").arg(m_caseName));

    if (simulationCase != nullptr)
    {
        const QVariantList keywordIndex =
            simulationCase->parsedSourceData().value(QStringLiteral("keywordIndex")).toList();
        for (const QVariant& keywordValue : keywordIndex)
        {
            const QString filePath =
                QFileInfo(keywordValue.toMap().value(QStringLiteral("file")).toString()).absoluteFilePath();
            if (!filePath.isEmpty() && !m_sourceFiles.contains(filePath, Qt::CaseInsensitive))
            {
                m_sourceFiles.append(filePath);
            }
        }
    }
    if (!m_sourceFile.isEmpty() &&
        !m_sourceFiles.contains(QFileInfo(m_sourceFile).absoluteFilePath(), Qt::CaseInsensitive))
    {
        m_sourceFiles.prepend(QFileInfo(m_sourceFile).absoluteFilePath());
    }

    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* descriptionLabel = new QLabel(tr("Edit the original DATA text and export it with its indexed input files. "
                                             "The source files will not be changed."),
                                          this);
    descriptionLabel->setWordWrap(true);
    layout->addWidget(descriptionLabel);

    m_dataEditor = new QPlainTextEdit(this);
    m_dataEditor->setLineWrapMode(QPlainTextEdit::NoWrap);
    layout->addWidget(m_dataEditor, 1);

    QFile sourceFile(m_sourceFile);
    if (sourceFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        m_dataEditor->setPlainText(QString::fromUtf8(sourceFile.readAll()));
    }
    else
    {
        m_outputEdit = new QPlainTextEdit(this);
        m_outputEdit->setReadOnly(true);
        m_outputEdit->setMaximumHeight(90);
        showOutput(tr("Unable to read the source DATA file: %1").arg(m_sourceFile));
        layout->addWidget(m_outputEdit);
    }

    QHBoxLayout* directoryLayout = new QHBoxLayout();
    directoryLayout->addWidget(new QLabel(tr("Package parent directory"), this));
    m_parentDirectoryEdit = new QLineEdit(QFileInfo(m_sourceFile).absolutePath(), this);
    directoryLayout->addWidget(m_parentDirectoryEdit, 1);
    QPushButton* browseButton = new QPushButton(tr("Browse…"), this);
    directoryLayout->addWidget(browseButton);
    layout->addLayout(directoryLayout);
    connect(browseButton, &QPushButton::clicked, this,
            &IDOSSimulationDataGenerationWidget::onBrowseParentDirectoryTriggered);

    if (m_outputEdit == nullptr)
    {
        m_outputEdit = new QPlainTextEdit(this);
        m_outputEdit->setReadOnly(true);
        m_outputEdit->setMaximumHeight(90);
        layout->addWidget(m_outputEdit);
    }

    QPushButton* generateButton = new QPushButton(tr("Generate DATA Package"), this);
    layout->addWidget(generateButton);
    connect(generateButton, &QPushButton::clicked, this,
            &IDOSSimulationDataGenerationWidget::onGeneratePackageTriggered);
    resize(1000, 760);
}

void IDOSSimulationDataGenerationWidget::onBrowseParentDirectoryTriggered()
{
    const QString selectedDirectory =
        QFileDialog::getExistingDirectory(this, tr("Select package parent directory"), m_parentDirectoryEdit->text());
    if (!selectedDirectory.isEmpty())
    {
        m_parentDirectoryEdit->setText(selectedDirectory);
    }
}

void IDOSSimulationDataGenerationWidget::onGeneratePackageTriggered()
{
    IDOS_MESSAGE(tr("Generating DATA package."), IDOSLogLevel::Info);
    QString packagePath;
    QString errorMessage;
    if (!writePackage(m_parentDirectoryEdit->text(), &packagePath, &errorMessage))
    {
        showOutput(errorMessage);
        return;
    }
    showOutput(tr("DATA package created: %1").arg(packagePath), IDOSLogLevel::Info);
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
    if (m_sourceFile.isEmpty() || !sourceInfo.isFile() || !sourceInfo.isReadable())
    {
        *errorMessage = tr("The source DATA file is missing or unreadable.");
        return false;
    }
    if (m_dataEditor->toPlainText().trimmed().isEmpty())
    {
        *errorMessage = tr("The generated DATA text is empty.");
        return false;
    }

    const QFileInfo parentInfo(parentDirectory);
    if (!parentInfo.isDir() || !parentInfo.isWritable())
    {
        *errorMessage = tr("The package parent directory is missing or not writable.");
        return false;
    }

    const QString sourceRoot = sourceInfo.absolutePath();
    const QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"));
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
    const QString directoryName = QStringLiteral("%1_DATA_package_%2").arg(safeCaseName, stamp);
    const QString destinationDirectory = QDir(parentDirectory).absoluteFilePath(directoryName);
    if (QFileInfo::exists(destinationDirectory))
    {
        *errorMessage = tr("The package directory already exists. Try again in a moment.");
        return false;
    }

    QTemporaryDir temporaryDirectory(
        QDir(parentDirectory).absoluteFilePath(QStringLiteral(".idos_data_package_XXXXXX")));
    if (!temporaryDirectory.isValid())
    {
        *errorMessage = tr("Unable to create a temporary package directory.");
        return false;
    }

    const QString generatedFileName = generatedMainFileName();
    QSet<QString> copiedRelativePaths;
    const QString mainSourcePath = sourceInfo.absoluteFilePath();
    for (const QString& filePath : m_sourceFiles)
    {
        const QFileInfo inputInfo(filePath);
        if (!inputInfo.isFile() || !inputInfo.isReadable())
        {
            *errorMessage = tr("An indexed input file is missing or unreadable: %1").arg(filePath);
            return false;
        }
        if (inputInfo.isSymLink())
        {
            *errorMessage = tr("Symbolic links cannot be included in a generated DATA package: %1").arg(filePath);
            return false;
        }
        if (inputInfo.absoluteFilePath().compare(mainSourcePath, Qt::CaseInsensitive) == 0)
        {
            continue;
        }

        const QString relativePath =
            QDir::fromNativeSeparators(QDir(sourceRoot).relativeFilePath(inputInfo.absoluteFilePath()));
        if (relativePath == QStringLiteral("..") || relativePath.startsWith(QStringLiteral("../")) ||
            QDir::isAbsolutePath(relativePath))
        {
            *errorMessage =
                tr("An indexed input file is outside the DATA directory tree and cannot be relocated safely: %1")
                    .arg(filePath);
            return false;
        }
        if (relativePath.compare(generatedFileName, Qt::CaseInsensitive) == 0)
        {
            *errorMessage =
                tr("An indexed input file conflicts with the generated DATA filename: %1").arg(relativePath);
            return false;
        }
        const QString relativePathKey = relativePath.toCaseFolded();
        if (copiedRelativePaths.contains(relativePathKey))
        {
            continue;
        }
        copiedRelativePaths.insert(relativePathKey);

        const QString destinationPath = QDir(temporaryDirectory.path()).absoluteFilePath(relativePath);
        const QFileInfo destinationInfo(destinationPath);
        if (!QDir().mkpath(destinationInfo.absolutePath()) ||
            !QFile::copy(inputInfo.absoluteFilePath(), destinationPath))
        {
            *errorMessage = tr("Unable to copy indexed input file into package: %1").arg(relativePath);
            return false;
        }
    }

    const QString generatedFilePath = QDir(temporaryDirectory.path()).absoluteFilePath(generatedFileName);
    QSaveFile generatedFile(generatedFilePath);
    const QByteArray generatedContents = m_dataEditor->toPlainText().toUtf8();
    if (!generatedFile.open(QIODevice::WriteOnly) ||
        generatedFile.write(generatedContents) != generatedContents.size() || !generatedFile.commit())
    {
        *errorMessage = tr("Unable to write the generated DATA file.");
        return false;
    }

    temporaryDirectory.setAutoRemove(false);
    if (!QDir().rename(temporaryDirectory.path(), destinationDirectory))
    {
        QDir(temporaryDirectory.path()).removeRecursively();
        *errorMessage = tr("Unable to finalize the DATA package directory.");
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
