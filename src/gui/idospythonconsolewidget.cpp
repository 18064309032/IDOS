#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

#include "idospythonconsolewidget.h"

IDOSPythonConsoleWidget::IDOSPythonConsoleWidget(QWidget* parent)
    :
    QWidget(parent)
    , m_output(new QPlainTextEdit(this))
    , m_input(new QPlainTextEdit(this))
{
    m_output->setObjectName(QStringLiteral("pythonConsoleOutput"));
    m_output->setReadOnly(true);
    m_output->setPlaceholderText(tr("Python output will appear here."));
    m_output->setMaximumBlockCount(5000);

    m_input->setObjectName(QStringLiteral("pythonConsoleInput"));
    m_input->setPlaceholderText(tr("Enter Python code here."));
    m_input->setMinimumHeight(64);

    QPushButton* runButton = new QPushButton(tr("Run"), this);
    runButton->setObjectName(QStringLiteral("pythonConsoleRunButton"));
    runButton->setEnabled(false);
    runButton->setToolTip(tr("Python runtime is not connected."));
    QPushButton* clearOutputButton = new QPushButton(tr("Clear Output"), this);

    QSplitter* splitter = new QSplitter(Qt::Vertical, this);
    splitter->addWidget(m_input);
    splitter->addWidget(m_output);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes(QList<int>() << 330 << 110);

    QHBoxLayout* controlsLayout = new QHBoxLayout();
    controlsLayout->addWidget(new QLabel(tr("Python runtime is not connected."), this));
    controlsLayout->addStretch();
    controlsLayout->addWidget(clearOutputButton);
    controlsLayout->addWidget(runButton);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(6);
    layout->addWidget(splitter, 1);
    layout->addLayout(controlsLayout);

    setFocusProxy(m_input);
    connect(clearOutputButton,
            &QPushButton::clicked,
            this,
            &IDOSPythonConsoleWidget::onClearOutputClicked);
}

IDOSPythonConsoleWidget::~IDOSPythonConsoleWidget()
{
}

void IDOSPythonConsoleWidget::onClearOutputClicked()
{
    m_output->clear();
}
