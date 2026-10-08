#include <QComboBox>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScopedPointer>
#include <QTimer>
#include <QToolButton>
#include <QTranslator>
#include <QtTest>

#include "idoscasetreemodel.h"
#include "idosdatatreemenuprovider.h"
#include "idosdatatreemodel.h"
#include "idosdatatreeview.h"
#include "idosmainwindow.h"
#include "idosnewprojectdialog.h"
#include "idosproject.h"
#include "idoswell.h"

class ProjectDialogTest : public QObject
{
    Q_OBJECT
  private slots:
    void onNewWellFlow()
    {
        IDOSMainWindow window;
        window.onNewWell();
        IDOSProject* project = new IDOSProject(&window);
        window.setProject(project);
        IDOSDataTreeView* view = window.dataTreeView();
        IDOSDataTreeModel* model = window.dataTreeModel();
        IDOSDataTreeMenuProvider* provider = view->findChild<IDOSDataTreeMenuProvider*>();
        QVERIFY(provider != nullptr);
        QVERIFY(provider->createContextMenu() == nullptr);
        view->setCurrentIndex(model->index(0, 0, QModelIndex()));
        QVERIFY(provider->createContextMenu() == nullptr);
        view->setCurrentIndex(model->index(4, 0, model->index(0, 0, QModelIndex())));
        QScopedPointer<QMenu> menu(provider->createContextMenu());
        QVERIFY(menu != nullptr);
        QCOMPARE(menu->actions().size(), 2);
        QCOMPARE(menu->actions().first()->objectName(), QStringLiteral("newWellAction"));
        m_wellNames = QStringList{QStringLiteral("   "), QStringLiteral("  A10  ")};
        m_cancelAfterInput = false;
        QTimer::singleShot(0, this, &ProjectDialogTest::onSubmitWellName);
        menu->actions().first()->trigger();
        QCOMPARE(project->objects().size(), 1);
        IDOSWell* well = qobject_cast<IDOSWell*>(project->objects().first());
        QVERIFY(well != nullptr);
        QCOMPARE(well->name(), QStringLiteral("A10"));
        QVERIFY(!well->objectId().isEmpty());
        const QModelIndex wells = model->index(4, 0, model->index(0, 0, QModelIndex()));
        QCOMPARE(model->rowCount(wells), 1);
        QCOMPARE(model->objectFromIndex(model->index(0, 0, wells)), well);
        QCOMPARE(model->rowCount(model->index(0, 0, wells)), 4);
        view->setCurrentIndex(model->index(0, 0, wells));
        QVERIFY(provider->createContextMenu() == nullptr);
        m_wellNames = QStringList{QStringLiteral("a10")};
        m_cancelAfterInput = true;
        QTimer::singleShot(0, this, &ProjectDialogTest::onSubmitWellName);
        window.onNewWell();
        QCOMPARE(project->objects().size(), 1);
        QTimer::singleShot(0, this, &ProjectDialogTest::onCancelWellDialog);
        window.onNewWell();
        QCOMPARE(project->objects().size(), 1);
    }

    void onChineseTranslations()
    {
        QTranslator translator;
        QVERIFY(translator.load(QCoreApplication::applicationDirPath() + "/../../i18n/idos_zh_CN.qm"));
        QVERIFY(QCoreApplication::installTranslator(&translator));
        QVERIFY(QCoreApplication::translate("IDOSMainWindow", "Data") != QStringLiteral("Data"));
        QVERIFY(QCoreApplication::translate("IDOSMainWindow", "Case") != QStringLiteral("Case"));
        QVERIFY(QCoreApplication::translate("IDOSMainWindow", "Discard") != QStringLiteral("Discard"));
        QVERIFY(QObject::tr("All Supported Files (%1)") != QStringLiteral("All Supported Files (%1)"));
        QVERIFY(QObject::tr("All Supported Files (%1)").arg("*.las").contains("*.las"));
        QVERIFY(QObject::tr("Cannot open file: %1") != QStringLiteral("Cannot open file: %1"));
        QVERIFY(QObject::tr("Cannot open file: %1").arg("sample.las").contains("sample.las"));
        QCoreApplication::removeTranslator(&translator);
    }

    void onValidationAndMetadata()
    {
        IDOSNewProjectDialog dialog;
        QLineEdit* name = dialog.findChild<QLineEdit*>("projectName");
        QPushButton* create = dialog.findChild<QPushButton*>("createProject");
        QVERIFY(name && create);
        QVERIFY(!create->isEnabled());
        name->setText("   ");
        dialog.onAccepted();
        QCOMPARE(dialog.result(), int(QDialog::Rejected));
        name->setText(QStringLiteral("  Test Block Study  "));
        QVERIFY(create->isEnabled());
        dialog.findChild<QLineEdit*>("fieldBlock")->setText(QStringLiteral("East Block"));
        dialog.findChild<QPlainTextEdit*>("projectDescription")
            ->setPlainText(QStringLiteral("Integrated well and geological data study"));
        QComboBox* coordinates = dialog.findChild<QComboBox*>("coordinateType");
        coordinates->setCurrentIndex(2);
        QVERIFY(!create->isEnabled());
        dialog.findChild<QLineEdit*>("coordinateReference")->setText("EPSG:32650");
        dialog.findChild<QComboBox*>("unitSystem")->setCurrentIndex(1);
        dialog.findChild<QLineEdit*>("verticalDatum")->setText(QStringLiteral("Mean Sea Level"));
        dialog.findChild<QToolButton*>()->setChecked(true);
        dialog.show();
        QTest::qWait(50);
        const QString screenshotRoot = qEnvironmentVariable("IDOS_TEST_SCREENSHOT_DIR");
        if (!screenshotRoot.isEmpty())
        {
            QVERIFY(dialog.grab().save(screenshotRoot + "/new-project.png"));
        }
        dialog.onAccepted();
        QCOMPARE(dialog.result(), int(QDialog::Accepted));
        IDOSProject project;
        QVERIFY(project.setMetadata(dialog.projectMetadata()));
        QCOMPARE(project.metadata().name(), QStringLiteral("Test Block Study"));
        QCOMPARE(project.metadata().fieldBlock(), QStringLiteral("East Block"));
        QCOMPARE(project.metadata().coordinateReference(), QStringLiteral("EPSG:32650"));
        QCOMPARE(project.metadata().verticalDatum(), QStringLiteral("Mean Sea Level"));
        QVERIFY(project.metadata().unitSystem() == IDOSProjectMetadata::UnitSystem::Field);
        QVERIFY(project.objects().isEmpty());
        IDOSProjectMetadata invalid = project.metadata();
        invalid.setName(" ");
        QVERIFY(!project.setMetadata(invalid));
        QCOMPARE(project.metadata().name(), QStringLiteral("Test Block Study"));
        coordinates->setCurrentIndex(0);
        QVERIFY(dialog.projectMetadata().coordinateReference().isEmpty());
    }

    void onCreateCancelAndReplace()
    {
        IDOSMainWindow window;
        QVERIFY(!window.dataTreeModel()->project());
        QTimer::singleShot(0, this, &ProjectDialogTest::onRejectDialog);
        window.onNewProject();
        QCOMPARE(window.windowTitle(), QStringLiteral("IDOS"));
        QTimer::singleShot(0, this, &ProjectDialogTest::onAcceptFirstDialog);
        window.onNewProject();
        IDOSProject* original = window.dataTreeModel()->project();
        QVERIFY(original);
        QCOMPARE(window.caseTreeModel()->project(), original);
        QCOMPARE(window.windowTitle(), QStringLiteral("IDOS"));
        QCOMPARE(original->metadata().name(), QStringLiteral("Project One"));
        QVERIFY(original->objects().isEmpty());
        IDOSDataTreeModel* model = window.dataTreeModel();
        QCOMPARE(model->rowCount(QModelIndex()), 14);
        const QModelIndex wellGroup = model->index(0, 0, QModelIndex());
        QCOMPARE(model->data(wellGroup, Qt::DisplayRole).toString(), QStringLiteral("Well Group"));
        QCOMPARE(model->rowCount(wellGroup), 5);
        QCOMPARE(model->data(model->index(4, 0, wellGroup), Qt::DisplayRole).toString(), QStringLiteral("Wells"));
        model->setProject(original);
        QCOMPARE(model->rowCount(QModelIndex()), 14);
        IDOSWell* well = new IDOSWell();
        well->setName(QStringLiteral("A10"));
        original->addObject(well);
        const QModelIndex wells = model->index(4, 0, model->index(0, 0, QModelIndex()));
        QCOMPARE(model->rowCount(wells), 1);
        QCOMPARE(model->data(model->index(0, 0, wells), Qt::DisplayRole).toString(), QStringLiteral("A10"));
        IDOSProjectMetadata renamed = original->metadata();
        renamed.setName(QStringLiteral("Renamed Project"));
        QVERIFY(original->setMetadata(renamed));
        QCOMPARE(window.windowTitle(), QStringLiteral("IDOS"));
        QTimer::singleShot(0, this, &ProjectDialogTest::onAcceptReplacementDialog);
        window.onNewProject();
        QCOMPARE(window.dataTreeModel()->project(), original);
        original->removeObject(well->objectId());
        QCOMPARE(model->rowCount(QModelIndex()), 14);
        QCOMPARE(model->rowCount(model->index(4, 0, model->index(0, 0, QModelIndex()))), 0);
        delete original;
        QCOMPARE(model->rowCount(QModelIndex()), 0);
        QVERIFY(!window.dataTreeModel()->project());
        QVERIFY(!window.caseTreeModel()->project());
        QCOMPARE(window.windowTitle(), QStringLiteral("IDOS"));
    }

  private:
    QStringList m_wellNames;
    bool m_cancelAfterInput = false;

    void onSubmitWellName()
    {
        QInputDialog* dialog = qobject_cast<QInputDialog*>(QApplication::activeModalWidget());
        QVERIFY(dialog != nullptr);
        QVERIFY(!m_wellNames.isEmpty());
        dialog->setTextValue(m_wellNames.takeFirst());
        dialog->accept();
        if (!m_wellNames.isEmpty())
        {
            QTimer::singleShot(0, this, &ProjectDialogTest::onSubmitWellName);
        }
        else if (m_cancelAfterInput)
        {
            QTimer::singleShot(0, this, &ProjectDialogTest::onCancelWellDialog);
        }
    }

    void onCancelWellDialog()
    {
        QInputDialog* dialog = qobject_cast<QInputDialog*>(QApplication::activeModalWidget());
        QVERIFY(dialog != nullptr);
        dialog->reject();
    }

    void onRejectDialog()
    {
        IDOSNewProjectDialog* dialog = qobject_cast<IDOSNewProjectDialog*>(QApplication::activeModalWidget());
        QVERIFY(dialog);
        dialog->reject();
    }

    void onAcceptFirstDialog()
    {
        IDOSNewProjectDialog* dialog = qobject_cast<IDOSNewProjectDialog*>(QApplication::activeModalWidget());
        QVERIFY(dialog);
        dialog->findChild<QLineEdit*>("projectName")->setText(QStringLiteral("Project One"));
        dialog->onAccepted();
    }

    void onAcceptReplacementDialog()
    {
        IDOSNewProjectDialog* dialog = qobject_cast<IDOSNewProjectDialog*>(QApplication::activeModalWidget());
        QVERIFY(dialog);
        dialog->findChild<QLineEdit*>("projectName")->setText(QStringLiteral("Project Two"));
        dialog->onAccepted();
        QTimer::singleShot(0, this, &ProjectDialogTest::onCancelReplacement);
    }

    void onCancelReplacement()
    {
        QMessageBox* warning = qobject_cast<QMessageBox*>(QApplication::activeModalWidget());
        QVERIFY(warning);
        warning->done(QMessageBox::Cancel);
    }
};

QTEST_MAIN(ProjectDialogTest)
#include "project_dialog_test.moc"
