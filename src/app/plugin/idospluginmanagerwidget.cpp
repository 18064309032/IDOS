#include <QAbstractItemView>
#include <QFormLayout>
#include <QFont>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QStyle>
#include <QTreeView>
#include <QVBoxLayout>

#include "idospluginlistmodel.h"
#include "idospluginregistry.h"

#include "idospluginmanagerwidget.h"

IDOSPluginManagerWidget::IDOSPluginManagerWidget(IDOSPluginRegistry* registry,
                                                 QWidget* parent)
    : QWidget(parent)
    , m_pluginModel(new IDOSPluginListModel(registry, this))
    , m_filterProxy(new QSortFilterProxyModel(this))
    , m_pageTitle(new QLabel(tr("Installed Plugins"), this))
    , m_pageSubtitle(new QLabel(
          tr("Manage and inspect plugins installed in this application."), this))
    , m_pluginSummary(new QLabel(this))
    , m_searchEdit(new QLineEdit(this))
    , m_pluginPane(new QWidget(this))
    , m_pluginList(new QTreeView(this))
    , m_detailsPane(new QWidget(this))
    , m_detailsTitle(new QLabel(tr("Plugin Information"), this))
    , m_nameLabel(new QLabel(this))
    , m_pathEdit(new QLineEdit(this))
    , m_categoryLabel(new QLabel(this))
    , m_versionLabel(new QLabel(this))
    , m_statusLabel(new QLabel(this))
    , m_descriptionLabel(new QLabel(this))
    , m_errorLabel(new QLabel(this))
{
    QFont pageTitleFont = m_pageTitle->font();
    pageTitleFont.setPointSize(pageTitleFont.pointSize() + 2);
    pageTitleFont.setBold(true);
    m_pageTitle->setFont(pageTitleFont);

    QFont detailsTitleFont = m_detailsTitle->font();
    detailsTitleFont.setBold(true);
    m_detailsTitle->setFont(detailsTitleFont);

    m_pageTitle->setObjectName(QStringLiteral("pluginPageTitle"));
    m_pageSubtitle->setObjectName(QStringLiteral("pluginPageSubtitle"));
    m_pluginSummary->setObjectName(QStringLiteral("pluginSummary"));
    m_pluginSummary->setAlignment(Qt::AlignCenter);
    m_detailsTitle->setObjectName(QStringLiteral("pluginSectionTitle"));

    m_searchEdit->setObjectName(QStringLiteral("pluginSearch"));
    m_searchEdit->setPlaceholderText(tr("Search by name, category, or description"));
    m_searchEdit->setClearButtonEnabled(true);

    m_filterProxy->setSourceModel(m_pluginModel);
    m_filterProxy->setFilterKeyColumn(0);
    m_filterProxy->setFilterRole(IDOSPluginListModel::SearchTextRole);
    m_filterProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    m_pluginList->setModel(m_filterProxy);
    m_pluginList->setRootIsDecorated(false);
    m_pluginList->setItemsExpandable(false);
    m_pluginList->setAlternatingRowColors(false);
    m_pluginList->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pluginList->setUniformRowHeights(true);
    m_pluginList->setIndentation(0);
    m_pluginList->header()->setStretchLastSection(false);
    m_pluginList->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_pluginList->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    m_nameLabel->setWordWrap(true);
    m_pathEdit->setReadOnly(true);
    m_pathEdit->setFrame(false);
    m_categoryLabel->setWordWrap(true);
    m_versionLabel->setWordWrap(true);
    m_statusLabel->setWordWrap(true);
    m_descriptionLabel->setWordWrap(true);
    m_descriptionLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_errorLabel->setWordWrap(true);
    m_errorLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_errorLabel->hide();

    QPushButton* refreshButton = new QPushButton(tr("Refresh"), this);
    refreshButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    refreshButton->setMinimumHeight(36);
    refreshButton->setObjectName(QStringLiteral("pluginRefreshButton"));

    QHBoxLayout* headingLayout = new QHBoxLayout();
    headingLayout->addWidget(m_pageTitle);
    headingLayout->addWidget(m_pluginSummary);
    headingLayout->addStretch();

    QHBoxLayout* toolbarLayout = new QHBoxLayout();
    toolbarLayout->setSpacing(8);
    toolbarLayout->addWidget(m_searchEdit, 1);
    toolbarLayout->addWidget(refreshButton);

    QLabel* listHint = new QLabel(
        tr("Use the checkbox to enable or disable a plugin."), m_pluginPane);
    listHint->setWordWrap(true);

    QVBoxLayout* listLayout = new QVBoxLayout(m_pluginPane);
    listLayout->setContentsMargins(0, 0, 12, 0);
    listLayout->setSpacing(6);
    listLayout->addWidget(m_pluginList, 1);
    listLayout->addWidget(listHint);

    QFormLayout* metadataLayout = new QFormLayout();
    metadataLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
    metadataLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    metadataLayout->setHorizontalSpacing(16);
    metadataLayout->setVerticalSpacing(10);
    metadataLayout->addRow(tr("Category"), m_categoryLabel);
    metadataLayout->addRow(tr("Version"), m_versionLabel);
    metadataLayout->addRow(tr("Location"), m_pathEdit);

    QLabel* descriptionTitle = new QLabel(tr("Description"), m_detailsPane);
    QVBoxLayout* detailsLayout = new QVBoxLayout(m_detailsPane);
    detailsLayout->setContentsMargins(16, 0, 0, 0);
    detailsLayout->setSpacing(12);
    detailsLayout->addWidget(m_detailsTitle);

    QHBoxLayout* pluginHeadingLayout = new QHBoxLayout();
    pluginHeadingLayout->addWidget(m_nameLabel, 1);
    pluginHeadingLayout->addWidget(m_statusLabel, 0, Qt::AlignRight | Qt::AlignVCenter);
    detailsLayout->addLayout(pluginHeadingLayout);
    detailsLayout->addLayout(metadataLayout);
    detailsLayout->addWidget(descriptionTitle);
    detailsLayout->addWidget(m_descriptionLabel, 1);
    detailsLayout->addWidget(m_errorLabel);

    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(m_pluginPane);
    splitter->addWidget(m_detailsPane);
    splitter->setStretchFactor(0, 4);
    splitter->setStretchFactor(1, 6);
    splitter->setChildrenCollapsible(false);
    splitter->setSizes(QList<int>() << 420 << 560);

    QVBoxLayout* pageLayout = new QVBoxLayout();
    pageLayout->setSpacing(4);
    pageLayout->addLayout(headingLayout);
    pageLayout->addWidget(m_pageSubtitle);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 14, 16, 16);
    layout->setSpacing(12);
    layout->addLayout(pageLayout);
    layout->addLayout(toolbarLayout);
    layout->addWidget(splitter, 1);

    setMinimumSize(900, 580);
    connect(m_pluginList->selectionModel(),
            &QItemSelectionModel::currentChanged,
            this,
            &IDOSPluginManagerWidget::onCurrentChanged);
    connect(m_pluginModel,
            &QAbstractItemModel::dataChanged,
            this,
            &IDOSPluginManagerWidget::onPluginDataChanged);
    connect(m_searchEdit,
            &QLineEdit::textChanged,
            this,
            &IDOSPluginManagerWidget::onSearchTextChanged);
    connect(refreshButton,
            &QPushButton::clicked,
            this,
            &IDOSPluginManagerWidget::onRefreshClicked);
}

void IDOSPluginManagerWidget::refresh()
{
    refreshPluginList();
}

void IDOSPluginManagerWidget::onCurrentChanged(const QModelIndex& current,
                                               const QModelIndex& previous)
{
    Q_UNUSED(previous);
    updatePluginDetails(current);
}

void IDOSPluginManagerWidget::onPluginDataChanged(const QModelIndex& topLeft,
                                                  const QModelIndex& bottomRight,
                                                  const QVector<int>& roles)
{
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);
    Q_UNUSED(roles);
    updatePluginDetails(m_pluginList->currentIndex());
    updatePluginSummary();
}

void IDOSPluginManagerWidget::onSearchTextChanged(const QString& text)
{
    m_filterProxy->setFilterFixedString(text.trimmed());
}

void IDOSPluginManagerWidget::onRefreshClicked()
{
    QModelIndex currentIndex = m_pluginList->currentIndex();
    QString selectedKey;
    if (currentIndex.isValid())
    {
        const QModelIndex sourceIndex = m_filterProxy->mapToSource(currentIndex);
        selectedKey = m_pluginModel->data(sourceIndex, IDOSPluginListModel::PluginKeyRole)
                          .toString();
    }
    refreshPluginList(selectedKey);
}

void IDOSPluginManagerWidget::refreshPluginList(const QString& selectedKey)
{
    m_pluginModel->refresh();
    updatePluginSummary();

    int selectedRow = m_pluginModel->rowForKey(selectedKey);
    if (selectedRow < 0 && m_pluginModel->rowCount() > 0)
    {
        selectedRow = 0;
    }

    if (selectedRow < 0)
    {
        m_pluginList->setCurrentIndex(QModelIndex());
        updatePluginDetails(QModelIndex());
        return;
    }

    const QModelIndex sourceIndex = m_pluginModel->index(selectedRow, 0);
    const QModelIndex proxyIndex = m_filterProxy->mapFromSource(sourceIndex);
    m_pluginList->setCurrentIndex(proxyIndex);
    updatePluginDetails(proxyIndex);
}

void IDOSPluginManagerWidget::updatePluginDetails(const QModelIndex& proxyIndex)
{
    if (!proxyIndex.isValid())
    {
        const bool hasPlugins = m_pluginModel->rowCount() > 0;
        m_nameLabel->setText(hasPlugins ? tr("Select a plugin") : tr("Plugin Manager"));
        m_statusLabel->clear();
        m_pathEdit->clear();
        m_categoryLabel->clear();
        m_versionLabel->clear();
        if (!hasPlugins)
        {
            m_descriptionLabel->setText(tr("No plugins found in the plugin directory."));
        }
        else if (m_filterProxy->rowCount() == 0)
        {
            m_descriptionLabel->setText(tr("No plugins match your search."));
        }
        else
        {
            m_descriptionLabel->clear();
        }
        m_errorLabel->clear();
        return;
    }

    const QModelIndex sourceIndex = m_filterProxy->mapToSource(proxyIndex);
    const QString pluginName =
        m_pluginModel->data(sourceIndex, Qt::DisplayRole).toString();
    const QString category =
        m_pluginModel->data(sourceIndex, IDOSPluginListModel::PluginCategoryRole).toString();
    const QString version =
        m_pluginModel->data(sourceIndex, IDOSPluginListModel::PluginVersionRole).toString();
    const QString status =
        m_pluginModel->data(m_pluginModel->index(sourceIndex.row(), 1), Qt::DisplayRole)
            .toString();
    const QString description =
        m_pluginModel->data(sourceIndex, IDOSPluginListModel::PluginDescriptionRole).toString();
    const QString errorMessage =
        m_pluginModel->data(sourceIndex, IDOSPluginListModel::PluginErrorRole).toString();
    const QString pluginPath =
        m_pluginModel->data(sourceIndex, IDOSPluginListModel::PluginPathRole).toString();

    m_nameLabel->setText(pluginName);
    m_pathEdit->setText(pluginPath);
    m_pathEdit->setToolTip(pluginPath);
    m_pathEdit->setCursorPosition(pluginPath.length());
    m_categoryLabel->setText(category.isEmpty() ? tr("Not specified") : category);
    m_versionLabel->setText(version.isEmpty() ? tr("Not specified") : version);
    m_statusLabel->setText(status);
    QFont statusFont = m_statusLabel->font();
    statusFont.setBold(true);
    m_statusLabel->setFont(statusFont);
    m_descriptionLabel->setText(description.isEmpty() ? tr("No description is available.")
                                                      : description);
    m_errorLabel->setText(errorMessage.isEmpty()
                              ? QString()
                              : tr("Plugin failed to load. See the application log for details."));
    m_errorLabel->setToolTip(errorMessage);
    m_errorLabel->setVisible(!errorMessage.isEmpty());
}

void IDOSPluginManagerWidget::updatePluginSummary()
{
    int loadedCount = 0;
    for (int row = 0; row < m_pluginModel->rowCount(); ++row)
    {
        if (m_pluginModel->data(m_pluginModel->index(row, 0),
                                IDOSPluginListModel::PluginLoadedRole)
                .toBool())
        {
            ++loadedCount;
        }
    }
    m_pluginSummary->setText(tr("%1 plugins · %2 loaded")
                                 .arg(m_pluginModel->rowCount())
                                 .arg(loadedCount));
}
