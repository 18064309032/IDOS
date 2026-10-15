#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QPoint>
#include <QPushButton>
#include <QSizePolicy>
#include <QStyle>
#include <QVBoxLayout>

#include "idosprojectstartwidget.h"

IDOSProjectStartWidget::IDOSProjectStartWidget(QWidget* parent)
    :
    QWidget(parent)
    , m_recentProjectsList(new QListWidget(this))
    , m_emptyRecentProjectsLabel(new QLabel(tr("No Recent Projects"), this))
{
    setObjectName(QStringLiteral("projectStartPage"));
    setStyleSheet(QStringLiteral(
        "QWidget#projectStartPage { background-color: #F3F6FA; }"
        "QFrame#startHeroCard, QFrame#startRecentCard { background-color: #FFFFFF; "
        "border: 1px solid #DCE4EE; border-radius: 12px; }"
        "QFrame#startHeroAccent { background-color: #2E83C5; border: 0; "
        "border-radius: 2px; }"
        "QLabel#startEyebrow { color: #3574C4; font-size: 9pt; font-weight: 700; "
        "letter-spacing: 1px; }"
        "QLabel#startWelcome { color: #172B4D; font-size: 25pt; font-weight: 700; }"
        "QLabel#startDescription, QLabel#startCardDescription, "
        "QLabel#startRecentHint { color: #64748B; }"
        "QLabel#startSectionTitle { color: #172B4D; font-size: 13pt; font-weight: 700; }"
        "QLabel#startRecentIcon { color: #3574C4; font-size: 15pt; }"
        "QPushButton#startPrimaryButton { background-color: #1769AA; color: #FFFFFF; "
        "border: 1px solid #1769AA; border-radius: 6px; padding: 9px 16px; "
        "font-weight: 600; text-align: left; }"
        "QPushButton#startPrimaryButton:hover { background-color: #0F568F; }"
        "QPushButton#startSecondaryButton { background-color: #FFFFFF; color: #245D91; "
        "border: 1px solid #B8C9DB; border-radius: 6px; padding: 9px 16px; "
        "font-weight: 600; text-align: left; }"
        "QPushButton#startSecondaryButton:hover { background-color: #EFF6FC; }"
        "QListWidget#startPageRecentProjectsList { background-color: #FFFFFF; border: 0; "
        "outline: 0; }"
        "QListWidget#startPageRecentProjectsList::item { color: #1F344D; "
        "border-bottom: 1px solid #E8EDF3; padding: 10px 8px; }"
        "QListWidget#startPageRecentProjectsList::item:hover { background-color: #F3F8FD; }"
        "QListWidget#startPageRecentProjectsList::item:selected { "
        "background-color: #E7F1FA; color: #174F7D; }"
        "QLabel#startEmptyRecentProjects { color: #7B8796; padding: 12px; }"));

    QVBoxLayout* pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(40, 32, 40, 32);
    pageLayout->setSpacing(18);

    QFrame* heroCard = new QFrame(this);
    heroCard->setObjectName(QStringLiteral("startHeroCard"));
    QVBoxLayout* heroLayout = new QVBoxLayout(heroCard);
    heroLayout->setContentsMargins(28, 24, 28, 24);
    heroLayout->setSpacing(8);

    QFrame* heroAccent = new QFrame(heroCard);
    heroAccent->setObjectName(QStringLiteral("startHeroAccent"));
    heroAccent->setFixedSize(42, 4);
    heroLayout->addWidget(heroAccent);

    QLabel* eyebrowLabel = new QLabel(tr("PROJECT WORKSPACE"), heroCard);
    eyebrowLabel->setObjectName(QStringLiteral("startEyebrow"));
    QLabel* welcomeLabel = new QLabel(tr("Welcome to IDOS"), heroCard);
    welcomeLabel->setObjectName(QStringLiteral("startWelcome"));
    QLabel* descriptionLabel = new QLabel(
        tr("Create a project, open an existing project, or continue from the recent list."), heroCard);
    descriptionLabel->setObjectName(QStringLiteral("startDescription"));
    descriptionLabel->setWordWrap(true);
    heroLayout->addWidget(eyebrowLabel);
    heroLayout->addWidget(welcomeLabel);
    heroLayout->addWidget(descriptionLabel);

    QHBoxLayout* actionsLayout = new QHBoxLayout();
    actionsLayout->setSpacing(12);
    QPushButton* newProjectButton = new QPushButton(
        QIcon(QStringLiteral(":/images/app-project-new.svg")), tr("New Project"), heroCard);
    newProjectButton->setObjectName(QStringLiteral("startPrimaryButton"));
    newProjectButton->setMinimumSize(190, 46);
    QLabel* newProjectDescription = new QLabel(tr("Start a project with new settings."), heroCard);
    newProjectDescription->setObjectName(QStringLiteral("startCardDescription"));
    newProjectDescription->setWordWrap(true);
    QVBoxLayout* newProjectLayout = new QVBoxLayout();
    newProjectLayout->setSpacing(5);
    newProjectLayout->addWidget(newProjectButton);
    newProjectLayout->addWidget(newProjectDescription);
    connect(newProjectButton, &QPushButton::clicked,
            this, &IDOSProjectStartWidget::onNewProjectClicked);

    QPushButton* openProjectButton = new QPushButton(
        QIcon(QStringLiteral(":/images/app-project-open.svg")), tr("Open Project"), heroCard);
    openProjectButton->setObjectName(QStringLiteral("startSecondaryButton"));
    openProjectButton->setMinimumSize(190, 46);
    QLabel* openProjectDescription = new QLabel(tr("Browse for an existing IDOS project."), heroCard);
    openProjectDescription->setObjectName(QStringLiteral("startCardDescription"));
    openProjectDescription->setWordWrap(true);
    QVBoxLayout* openProjectLayout = new QVBoxLayout();
    openProjectLayout->setSpacing(5);
    openProjectLayout->addWidget(openProjectButton);
    openProjectLayout->addWidget(openProjectDescription);
    connect(openProjectButton, &QPushButton::clicked,
            this, &IDOSProjectStartWidget::onOpenProjectClicked);
    actionsLayout->addLayout(newProjectLayout);
    actionsLayout->addLayout(openProjectLayout);
    actionsLayout->addStretch(1);
    heroLayout->addSpacing(8);
    heroLayout->addLayout(actionsLayout);
    pageLayout->addWidget(heroCard);

    QFrame* recentCard = new QFrame(this);
    recentCard->setObjectName(QStringLiteral("startRecentCard"));
    QVBoxLayout* recentLayout = new QVBoxLayout(recentCard);
    recentLayout->setContentsMargins(22, 18, 22, 16);
    recentLayout->setSpacing(8);

    QHBoxLayout* recentHeaderLayout = new QHBoxLayout();
    QLabel* recentProjectsLabel = new QLabel(tr("Recent Projects"), recentCard);
    recentProjectsLabel->setObjectName(QStringLiteral("startSectionTitle"));
    QLabel* recentHintLabel = new QLabel(tr("Select a project to continue."), recentCard);
    recentHintLabel->setObjectName(QStringLiteral("startRecentHint"));
    QLabel* recentIconLabel = new QLabel(recentCard);
    recentIconLabel->setObjectName(QStringLiteral("startRecentIcon"));
    recentIconLabel->setPixmap(style()->standardIcon(QStyle::SP_DirOpenIcon).pixmap(18, 18));
    recentHeaderLayout->addWidget(recentIconLabel);
    recentHeaderLayout->addWidget(recentProjectsLabel);
    recentHeaderLayout->addStretch(1);
    recentHeaderLayout->addWidget(recentHintLabel);
    recentLayout->addLayout(recentHeaderLayout);

    m_recentProjectsList->setObjectName(QStringLiteral("startPageRecentProjectsList"));
    m_recentProjectsList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_recentProjectsList->setFrameShape(QFrame::NoFrame);
    m_recentProjectsList->setSpacing(1);
    m_recentProjectsList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_recentProjectsList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_recentProjectsList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_recentProjectsList, &QListWidget::itemActivated,
            this, &IDOSProjectStartWidget::onRecentProjectItemActivated);
    connect(m_recentProjectsList, &QListWidget::customContextMenuRequested,
            this, &IDOSProjectStartWidget::onRecentProjectContextMenuRequested);
    recentLayout->addWidget(m_recentProjectsList, 1);

    m_emptyRecentProjectsLabel->setObjectName(QStringLiteral("startEmptyRecentProjects"));
    m_emptyRecentProjectsLabel->setAlignment(Qt::AlignCenter);
    recentLayout->addWidget(m_emptyRecentProjectsLabel, 1);
    pageLayout->addWidget(recentCard, 1);
    setRecentProjects(QStringList());
}

void IDOSProjectStartWidget::setRecentProjects(const QStringList& projectPaths)
{
    m_recentProjectsList->clear();
    const bool hasRecentProjects = !projectPaths.isEmpty();
    m_recentProjectsList->setVisible(hasRecentProjects);
    m_emptyRecentProjectsLabel->setVisible(!hasRecentProjects);

    for (const QString& projectPath : projectPaths)
    {
        QFileInfo projectInfo(projectPath);
        const bool projectExists = projectInfo.exists();
        const QString status = projectExists ? tr("Available") : tr("File Not Found");
        QListWidgetItem* item = new QListWidgetItem(
            QStringLiteral("%1\n%2\n%3").arg(projectInfo.completeBaseName(),
                                               projectInfo.absoluteFilePath(),
                                               status),
            m_recentProjectsList);
        item->setData(Qt::UserRole, projectPath);
        if (!projectExists)
        {
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        }
    }
}

void IDOSProjectStartWidget::onNewProjectClicked()
{
    emit newProjectRequested();
}

void IDOSProjectStartWidget::onOpenProjectClicked()
{
    emit openProjectRequested();
}

void IDOSProjectStartWidget::onRecentProjectItemActivated(QListWidgetItem* item)
{
    if (item != nullptr)
    {
        const QString path = item->data(Qt::UserRole).toString();
        if (!path.isEmpty())
        {
            emit recentProjectOpenRequested(path);
        }
    }
}

void IDOSProjectStartWidget::onRecentProjectContextMenuRequested(const QPoint& position)
{
    QListWidgetItem* item = m_recentProjectsList->itemAt(position);
    if (item == nullptr)
    {
        return;
    }

    const QString path = item->data(Qt::UserRole).toString();
    if (path.isEmpty())
    {
        return;
    }

    QMenu contextMenu(this);
    QAction* removeAction = contextMenu.addAction(tr("Remove from Recent Projects"));
    QAction* selectedAction = contextMenu.exec(m_recentProjectsList->viewport()->mapToGlobal(position));
    if (selectedAction == removeAction)
    {
        emit recentProjectRemoveRequested(path);
    }
}

