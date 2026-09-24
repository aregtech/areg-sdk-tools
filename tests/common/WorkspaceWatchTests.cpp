/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        tests/common/WorkspaceWatchTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       What the workspace navigation and the open documents do when files and
 *               folders change on disk behind Lusan's back:
 *
 *                 1. A created, deleted or renamed folder, and a created, deleted or renamed
 *                    file that passes the filter, shows up in the tree. A filtered out file
 *                    does not.
 *                 2. The tree keeps its expanded folders and its selection.
 *                 3. A removed or renamed selected entry, or its folder, clears the selection.
 *                 4. A folder that holds an open document can be renamed.
 *                 5. An open document whose file is gone asks once whether to keep or close
 *                    it. A kept document is modified, and saving it asks for a file.
 *                 6. A modified document carries the unsaved marker on its tab icon.
 *
 ************************************************************************/

#include "lusan/app/LusanApplication.hpp"
#include "lusan/app/NEAppThemes.hpp"
#include "lusan/data/common/WorkspaceWatcher.hpp"
#include "lusan/model/common/FileSystemModel.hpp"
#include "lusan/view/common/MdiChild.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/common/NaviFileSystem.hpp"

#include "tests/common/UiTestEnv.hpp"

#include <QAbstractButton>
#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QImage>
#include <QItemSelectionModel>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QPointer>
#include <QStandardPaths>
#include <QTabBar>
#include <QTemporaryDir>
#include <QThread>
#include <QTimer>
#include <QTreeView>

#include <cstdio>
#include <functional>

namespace
{
    int gChecks = 0;
    int gFailures = 0;

    void check(bool condition, const char* what)
    {
        ++gChecks;
        if (condition == false)
        {
            ++gFailures;
            std::printf("  [FAIL] %s\n", what);
        }
    }

    //!< The time a change on disk has to reach the window.
    constexpr int WaitMs{ 4000 };

    bool waitFor(const std::function<bool()>& done, int ms)
    {
        QElapsedTimer timer;
        timer.start();
        while (done() == false)
        {
            if (timer.elapsed() > ms)
                return false;

            QApplication::processEvents(QEventLoop::AllEvents, 20);
            QThread::msleep(10);
        }

        return true;
    }

    void idle(int ms)
    {
        waitFor([]() { return false; }, ms);
    }

    //!< Answers the prompts the checks provoke, and counts them.
    struct sResponder
    {
        QString answer;             //!< The button to click in a message box.
        int     prompts{ 0 };       //!< The message boxes seen.
        int     fileDialogs{ 0 };   //!< The file dialogs seen, each one cancelled.
        QString lastText;           //!< The text of the last message box.
    };

    sResponder gResponder;

    void answerPrompts()
    {
        const QWidgetList widgets{ QApplication::topLevelWidgets() };
        for (QWidget* widget : widgets)
        {
            if (widget->isVisible() == false)
                continue;

            if (QMessageBox* box = qobject_cast<QMessageBox*>(widget))
            {
                ++gResponder.prompts;
                gResponder.lastText = box->text();
                for (QAbstractButton* button : box->buttons())
                {
                    if (button->text() == gResponder.answer)
                    {
                        button->click();
                        return;
                    }
                }

                box->reject();
                return;
            }

            if (QFileDialog* dialog = qobject_cast<QFileDialog*>(widget))
            {
                ++gResponder.fileDialogs;
                dialog->reject();
                return;
            }
        }
    }

    QString pathOf(FileSystemModel& model, const QModelIndex& index)
    {
        return WorkspaceWatcher::normalizePath(model.filePath(index));
    }

    //!< Finds the row of the path among the loaded rows.
    QModelIndex findPath(FileSystemModel& model, const QModelIndex& parent, const QString& path)
    {
        for (int row = 0; row < model.rowCount(parent); ++row)
        {
            const QModelIndex index{ model.index(row, 0, parent) };
            const QString entry{ pathOf(model, index) };
            if (entry.compare(path, Qt::CaseSensitivity::CaseInsensitive) == 0)
                return index;

            if (model.isDir(index) && WorkspaceWatcher::isSameOrUnder(path, entry))
            {
                const QModelIndex found{ findPath(model, index, path) };
                if (found.isValid())
                    return found;
            }
        }

        return QModelIndex();
    }

    //!< Loads and expands every folder from the root down to the path.
    QModelIndex expandTo(QTreeView& tree, FileSystemModel& model, const QString& path)
    {
        QModelIndex parent{ tree.rootIndex() };
        for (int depth = 0; depth < 32; ++depth)
        {
            if (model.canFetchMore(parent))
            {
                model.fetchMore(parent);
            }

            QModelIndex next;
            for (int row = 0; row < model.rowCount(parent); ++row)
            {
                const QModelIndex index{ model.index(row, 0, parent) };
                if (WorkspaceWatcher::isSameOrUnder(path, pathOf(model, index)))
                {
                    next = index;
                    break;
                }
            }

            if (next.isValid() == false)
                return QModelIndex();

            if (pathOf(model, next).compare(path, Qt::CaseSensitivity::CaseInsensitive) == 0)
                return next;

            if (model.canFetchMore(next))
            {
                model.fetchMore(next);
            }

            tree.expand(next);
            parent = next;
        }

        return QModelIndex();
    }

    bool writeFile(const QString& path, const QByteArray& data)
    {
        QFile file(path);
        return file.open(QIODevice::WriteOnly) && (file.write(data) == data.size());
    }

    //!< Returns true if the icon carries the unsaved marker in its bottom right quarter.
    bool hasUnsavedMark(const QIcon& icon)
    {
        const QImage image{ icon.pixmap(QSize(32, 32)).toImage() };
        if (image.isNull())
            return false;

        const QColor mark{ NEAppThemes::unsavedMarkColor() };
        const QColor pixel{ image.pixelColor(image.width() * 3 / 4, image.height() * 3 / 4) };
        return (qAbs(pixel.red() - mark.red()) < 40) && (qAbs(pixel.green() - mark.green()) < 40) && (qAbs(pixel.blue() - mark.blue()) < 40);
    }

    MdiChild* activeChild(QMdiArea& area)
    {
        QMdiSubWindow* sub{ area.activeSubWindow() };
        return (sub != nullptr ? qobject_cast<MdiChild*>(sub->widget()) : nullptr);
    }
}

int main(int argc, char* argv[])
{
    LusanTest::prepareUiEnvironment();
    QStandardPaths::setTestModeEnabled(true);
    // The prompt answerer sees only the dialogs Qt draws itself.
    QCoreApplication::setAttribute(Qt::ApplicationAttribute::AA_DontUseNativeDialogs, true);
    LusanApplication app(argc, argv);

    QTemporaryDir temp;
    check(temp.isValid(), "a temporary workspace can be created");
    const QString root{ WorkspaceWatcher::normalizePath(temp.path()) };

    QFile source(QStringLiteral(LUSAN_TEST_DATA_DIR "/TrafficLight.fsml"));
    check(source.open(QIODevice::ReadOnly), "the sample state machine can be read");
    const QByteArray document{ source.readAll() };

    QDir(root).mkpath(QStringLiteral("A/B"));
    QDir(root).mkpath(QStringLiteral("P"));
    check(writeFile(root + "/A/B/f.fsml", document), "the first document is written");
    check(writeFile(root + "/P/q.fsml", document), "the second document is written");
    check(writeFile(root + "/h.fsml", document), "the third document is written");

    LusanApplication::getOptions().addWorkspace(root, QStringLiteral("watch-test"), QString());

    QTimer responder;
    responder.setInterval(30);
    QObject::connect(&responder, &QTimer::timeout, &answerPrompts);
    responder.start();

    MdiMainWindow window;
    window.resize(1400, 900);
    window.show();
    QApplication::processEvents();

    NaviFileSystem& navi{ window.getNaviFileSystem() };
    QTreeView* tree{ navi.findChild<QTreeView*>() };
    FileSystemModel* model{ tree != nullptr ? qobject_cast<FileSystemModel*>(tree->model()) : nullptr };
    QMdiArea* area{ window.findChild<QMdiArea*>() };
    check((tree != nullptr) && (model != nullptr) && (area != nullptr), "the window carries the workspace tree and the editor area");
    if ((tree == nullptr) || (model == nullptr) || (area == nullptr))
    {
        std::printf("%d checks, %d failures\n", gChecks, gFailures);
        return 1;
    }

    const QStringList& roots{ window.getWorkspaceWatcher().getRoots() };
    check(roots.contains(root, Qt::CaseSensitivity::CaseInsensitive), "the workspace root is watched");

    std::printf("[tree] changes on disk reach the tree, which keeps its state\n");
    const QString dirA{ root + "/A" };
    const QString dirB{ root + "/A/B" };
    const QString fileF{ dirB + "/f.fsml" };
    {
        const QModelIndex indexF{ expandTo(*tree, *model, fileF) };
        check(indexF.isValid(), "the first document is in the tree");
        tree->setCurrentIndex(indexF);
        QApplication::processEvents();

        auto currentIs = [&](const QString& path) { return pathOf(*model, tree->currentIndex()).compare(path, Qt::CaseSensitivity::CaseInsensitive) == 0; };
        auto shows = [&](const QString& path) { return findPath(*model, tree->rootIndex(), path).isValid(); };
        auto expanded = [&](const QString& path) { return tree->isExpanded(findPath(*model, tree->rootIndex(), path)); };

        check(writeFile(dirB + "/g.siml", QByteArray()), "a file is created on disk");
        check(waitFor([&]() { return shows(dirB + "/g.siml"); }, WaitMs), "a created file that passes the filter appears");
        check(expanded(dirA) && expanded(dirB), "the folders stay expanded");
        check(currentIs(fileF), "the selected file stays selected");

        check(writeFile(dirB + "/n.zzz", QByteArray()), "a file of an unknown kind is created on disk");
        idle(WorkspaceWatcher::BatchIntervalMs * 4);
        check(shows(dirB + "/n.zzz") == false, "a filtered out file does not appear");

        check(QDir(dirA).mkdir(QStringLiteral("D")), "a folder is created on disk");
        check(waitFor([&]() { return shows(dirA + "/D"); }, WaitMs), "a created folder appears");

        check(QDir(dirA).rename(QStringLiteral("D"), QStringLiteral("E")), "a folder is renamed on disk");
        check(waitFor([&]() { return shows(dirA + "/E") && (shows(dirA + "/D") == false); }, WaitMs), "a renamed folder appears under its new name only");

        check(QFile::rename(dirB + "/g.siml", dirB + "/k.siml"), "a file is renamed on disk");
        check(waitFor([&]() { return shows(dirB + "/k.siml") && (shows(dirB + "/g.siml") == false); }, WaitMs), "a renamed file appears under its new name only");

        check(QFile::remove(dirB + "/k.siml"), "a file is deleted on disk");
        check(waitFor([&]() { return shows(dirB + "/k.siml") == false; }, WaitMs), "a deleted file disappears");
        check(expanded(dirA) && expanded(dirB), "the folders are still expanded");
        check(currentIs(fileF), "the selected file is still selected");

        check(QDir(dirA).rename(QStringLiteral("B"), QStringLiteral("B2")), "the folder of the selected file is renamed on disk");
        check(waitFor([&]() { return shows(dirA + "/B2") && (shows(dirB) == false); }, WaitMs), "the renamed folder appears under its new name");
        check(tree->currentIndex().isValid() == false, "the selection is cleared when the folder of the selected file is renamed");
        check(tree->selectionModel()->selectedIndexes().isEmpty(), "nothing stays selected");

        const QModelIndex indexE{ findPath(*model, tree->rootIndex(), dirA + "/E") };
        tree->setCurrentIndex(indexE);
        QApplication::processEvents();
        check(currentIs(dirA + "/E"), "a folder is selected");
        check(QDir(dirA + "/E").removeRecursively(), "the selected folder is deleted on disk");
        check(waitFor([&]() { return shows(dirA + "/E") == false; }, WaitMs), "the deleted folder disappears");
        check(tree->currentIndex().isValid() == false, "the selection is cleared when the selected folder is deleted");
    }

    std::printf("[documents] an open document whose file is gone\n");
    {
        gResponder = sResponder();
        gResponder.answer = QStringLiteral("Keep");
        check(window.openFile(root + "/P/q.fsml"), "a document is opened");
        QPointer<MdiChild> child{ activeChild(*area) };
        check((child != nullptr) && (child->isModified() == false), "the opened document is not modified");

        QMdiSubWindow* sub{ child != nullptr ? child->getMdiSubwindow() : nullptr };
        QTabBar* tabs{ area->findChild<QTabBar*>(QString(), Qt::FindDirectChildrenOnly) };
        auto tabIcon = [&]() { return ((tabs != nullptr) && (sub != nullptr)) ? tabs->tabIcon(area->subWindowList().indexOf(sub)) : QIcon(); };
        check((tabs != nullptr) && (hasUnsavedMark(tabIcon()) == false), "the tab of a saved document carries no marker");
        if (child != nullptr)
        {
            child->setModified(true);
            QApplication::processEvents();
            check(hasUnsavedMark(tabIcon()), "the tab of a modified document carries the unsaved marker");
            if ((argc > 1) && (tabs != nullptr))
            {
                // A picture of the marked tab in every theme, for a look by eye.
                QDir().mkpath(QString::fromLocal8Bit(argv[1]));
                for (OptionsManager::eAppTheme theme : NEAppThemes::allThemes())
                {
                    NEAppThemes::applyTheme(theme);
                    QApplication::processEvents();
                    const QImage shot{ tabs->grab().toImage() };
                    shot.scaled(shot.size() * 3, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::FastTransformation)
                        .save(QString::fromLocal8Bit(argv[1]) + QStringLiteral("/tab-%1.png").arg(static_cast<int>(theme)));
                    check(hasUnsavedMark(tabIcon()), "the marker is painted in the color of every theme");
                }
            }

            child->setModified(false);
            QApplication::processEvents();
            check(hasUnsavedMark(tabIcon()) == false, "the marker goes when the document is no longer modified");
        }

        check(QDir(root).rename(QStringLiteral("P"), QStringLiteral("P2")), "the folder of an open document can be renamed");
        check(waitFor([&]() { return gResponder.prompts > 0; }, WaitMs), "the user is told the file is gone");
        check(gResponder.lastText.contains(QDir::toNativeSeparators(root + "/P/q.fsml"), Qt::CaseSensitivity::CaseInsensitive), "the message names the file with its path");
        idle(1000);
        check(gResponder.prompts == 1, "the user is told once");
        check((child != nullptr) && child->isFileMissing() && child->isModified(), "a kept document is marked modified");
        check(hasUnsavedMark(tabIcon()), "a kept document carries the unsaved marker");

        if (child != nullptr)
        {
            const bool saved{ child->save() };
            check((saved == false) && (gResponder.fileDialogs == 1), "saving a kept document asks for the file to save into");
            check(QFileInfo::exists(root + "/P/q.fsml") == false, "the missing folder is not created by the save");
            child->setModified(false);
        }

        gResponder = sResponder();
        gResponder.answer = QStringLiteral("Close");
        check(window.openFile(root + "/h.fsml"), "another document is opened");
        QPointer<MdiChild> closing{ activeChild(*area) };
        check(QFile::rename(root + "/h.fsml", root + "/h2.fsml"), "the file of the open document is renamed");
        check(waitFor([&]() { return closing.isNull(); }, WaitMs), "a document the user chose to close is closed");
        check(gResponder.prompts == 1, "the user was asked once before it closed");
    }

    responder.stop();
    for (QMdiSubWindow* sub : area->subWindowList())
    {
        MdiChild* child{ qobject_cast<MdiChild*>(sub->widget()) };
        if (child != nullptr)
        {
            child->setModified(false);
        }
    }

    area->closeAllSubWindows();
    QApplication::processEvents();

    std::printf("%d checks, %d failures\n", gChecks, gFailures);
    return (gFailures == 0 ? 0 : 1);
}
