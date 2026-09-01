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
 *  \file        tests/common/DesignPageTests.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The shared editor pages, built over both document kinds and driven offscreen.
 *
 *               The Overview, Constants, Data Types, Includes, Attributes and Methods pages are
 *               one class each, used by the service interface editor and by the state machine
 *               editor alike. The Overview page edits the document itself and is driven by its
 *               own sweep, below; what is proved for the five list pages, over both documents:
 *
 *                 1. The page builds over the document's model and shows what the document holds.
 *                 2. Its Add button reaches the document through a command, and the row appears.
 *                 3. The new row comes up selected, with the caret in the field that names it.
 *                 4. Typing a name and committing it reaches the document, and the row follows.
 *                 5. Undo takes the name back and redo puts it again, with the row following
 *                    both ways -- the page rebuilds from the model rather than patching rows.
 *                 6. Undoing the add as well leaves the page exactly as it started.
 *
 *               Each page is also grabbed to a PNG, so the four merges that landed without a
 *               running window can be looked at.
 *
 *               Run as: lusan_design_pages <TrafficLight.fsml> [output directory]
 *
 ************************************************************************/

#include "lusan/data/common/AttributeDataSection.hpp"
#include "lusan/data/common/ConstantDataSection.hpp"
#include "lusan/data/common/DataTypeCustom.hpp"
#include "lusan/data/common/DataTypeDataSection.hpp"
#include "lusan/data/common/IncludeDataSection.hpp"
#include "lusan/data/common/MethodDataSection.hpp"
#include "lusan/model/dt/DataTypeDocumentModel.hpp"
#include "lusan/model/si/ServiceInterfaceModel.hpp"
#include "lusan/model/sm/SMIncludeModel.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/common/AttributeDetailsView.hpp"
#include "lusan/view/common/AttributeListView.hpp"
#include "lusan/view/common/AttributePage.hpp"
#include "lusan/view/common/ConstantDetailsView.hpp"
#include "lusan/view/common/ConstantListView.hpp"
#include "lusan/view/common/ConstantPage.hpp"
#include "lusan/view/common/DataTypeDetailsView.hpp"
#include "lusan/view/common/DataTypeListView.hpp"
#include "lusan/view/common/DataTypePage.hpp"
#include "lusan/view/common/IncludeDetailsView.hpp"
#include "lusan/view/common/IncludeListView.hpp"
#include "lusan/view/common/IncludePage.hpp"
#include "lusan/view/common/MethodDetailsView.hpp"
#include "lusan/view/common/MethodListView.hpp"
#include "lusan/view/common/MethodPage.hpp"
#include "lusan/view/common/MethodParamDetailsView.hpp"
#include "lusan/view/common/OverviewPage.hpp"
#include "lusan/view/si/SIOverview.hpp"
#include "lusan/view/sm/SMAttribute.hpp"
#include "lusan/view/sm/SMConstant.hpp"
#include "lusan/view/sm/SMInclude.hpp"
#include "lusan/view/sm/SMMethod.hpp"
#include "lusan/view/sm/SMOverview.hpp"

#include <QAbstractButton>
#include <QApplication>
#include <QCheckBox>
#include <QGroupBox>
#include <QMenu>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QToolButton>
#include <QTemporaryDir>
#include <QTextStream>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>

#include <cstdio>
#include <functional>

//////////////////////////////////////////////////////////////////////////
// Minimal assertion harness
//////////////////////////////////////////////////////////////////////////

namespace
{
    int gChecks  { 0 };
    int gFailures{ 0 };

    void check(bool condition, const char* what)
    {
        ++gChecks;
        if (condition == false)
        {
            ++gFailures;
            std::printf("  [FAIL] %s\n", what);
        }
    }
}

#define CHECK(cond)  check((cond), #cond)

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////

namespace
{
    void typeText(QWidget* field, const QString& text)
    {
        for (const QChar ch : text)
        {
            QKeyEvent press(QEvent::KeyPress, Qt::Key_unknown, Qt::NoModifier, QString(ch));
            QApplication::sendEvent(field, &press);
        }

        QApplication::processEvents();
    }

    void pressReturn(QWidget* field)
    {
        QKeyEvent press(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
        QApplication::sendEvent(field, &press);
        QApplication::processEvents();
    }

    /**
     * \brief   Opens the inline editor of a cell, picks the entry reading \p choice out of its
     *          drop-down and activates it, the way a click on the list does.
     * \return  False when the cell offered no drop-down, or offered no such entry.
     **/
    bool pickInCell(QTreeWidget* table, QTreeWidgetItem* row, int column, const QString& choice)
    {
        // A closed editor lives on until its deferred deletion runs, and would be picked up
        // instead of the one this call opens.
        QApplication::processEvents();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        const QList<QComboBox*> stale = table->viewport()->findChildren<QComboBox*>();

        const QModelIndex index = table->indexFromItem(row, column);
        table->setCurrentItem(row, column);
        table->edit(index);
        QApplication::processEvents();

        QComboBox* combo = nullptr;
        for (QComboBox* opened : table->viewport()->findChildren<QComboBox*>())
        {
            if (stale.contains(opened) == false)
            {
                combo = opened;
                break;
            }
        }

        if (combo == nullptr)
            return false;

        // The editor stays inside its cell: anything wider covers the first characters of the
        // column beside it while the edit is open.
        CHECK(combo->width() <= table->visualRect(index).width());

        const int at = combo->findText(choice);
        if (at < 0)
            return false;

        combo->setCurrentIndex(at);
        QMetaObject::invokeMethod(combo, "activated", Q_ARG(int, at));
        QApplication::processEvents();
        return true;
    }

    int countRows(const QTreeWidgetItem* item)
    {
        int count = 1;
        for (int i = 0; i < item->childCount(); ++i)
        {
            count += countRows(item->child(i));
        }

        return count;
    }

    int countRows(const QTreeWidget* tree)
    {
        int count = 0;
        for (int i = 0; i < tree->topLevelItemCount(); ++i)
        {
            count += countRows(tree->topLevelItem(i));
        }

        return count;
    }

    bool rowShows(const QTreeWidgetItem* item, const QString& text)
    {
        for (int col = 0; col < item->columnCount(); ++col)
        {
            if (item->text(col) == text)
                return true;
        }

        for (int i = 0; i < item->childCount(); ++i)
        {
            if (rowShows(item->child(i), text))
                return true;
        }

        return false;
    }

    bool treeShows(const QTreeWidget* tree, const QString& text)
    {
        for (int i = 0; i < tree->topLevelItemCount(); ++i)
        {
            if (rowShows(tree->topLevelItem(i), text))
                return true;
        }

        return false;
    }

    void savePicture(QWidget* page, const QString& outDir, const QString& name)
    {
        const QPixmap shot = page->grab();
        CHECK(shot.isNull() == false);
        if (shot.isNull() == false)
        {
            shot.save(outDir + QDir::separator() + name + QStringLiteral(".png"));
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// One page, driven through the widgets a user would touch
//////////////////////////////////////////////////////////////////////////

namespace
{
    /**
     * \brief   Everything the sweep needs from a page, whatever kind of element it edits.
     **/
    struct PageProbe
    {
        QWidget*                    page;       //!< The page under test.
        QTreeWidget*                tree;       //!< Its list.
        QToolButton*                addButton;  //!< The Add button of its list.
        QLineEdit*                  nameField;  //!< The details field that names an element.
        std::function<int()>        count;      //!< How many elements the document's section holds.
        std::function<QString(int)> nameAt;     //!< The name of the element at the given index.
    };

    void exercisePage( const char* label, const PageProbe& probe, DocUndoStack& stack
                     , const QString& typedName, const QString& outDir, const QString& picture)
    {
        std::printf("--- %s\n", label);
        CHECK(probe.page != nullptr);
        CHECK(probe.tree != nullptr);
        CHECK(probe.addButton != nullptr);
        CHECK(probe.nameField != nullptr);
        if ((probe.page == nullptr) || (probe.tree == nullptr) || (probe.addButton == nullptr) || (probe.nameField == nullptr))
            return;

        const int elemsBefore = probe.count();
        const int rowsBefore  = countRows(probe.tree);
        CHECK(rowsBefore >= elemsBefore);        // every stored element has a row of its own

        // 1. Add: the page asks the model, the model pushes a command, the notifier brings the row.
        probe.addButton->click();
        QApplication::processEvents();
        CHECK(probe.count() == elemsBefore + 1);
        CHECK(countRows(probe.tree) > rowsBefore);
        CHECK(stack.canUndo());
        CHECK(QApplication::focusWidget() == probe.nameField);

        const QString generated = probe.nameAt(elemsBefore);
        CHECK(generated.isEmpty() == false);
        CHECK(treeShows(probe.tree, generated));

        // 2. The field comes up with its text selected, so typing replaces it.
        typeText(probe.nameField, typedName);
        pressReturn(probe.nameField);
        CHECK(probe.nameAt(elemsBefore) == typedName);
        CHECK(treeShows(probe.tree, typedName));

        savePicture(probe.page, outDir, picture);

        // 3. Undo and redo both reach the row, because the row is rebuilt from the model.
        stack.undo();
        QApplication::processEvents();
        CHECK(probe.nameAt(elemsBefore) == generated);
        CHECK(treeShows(probe.tree, generated));
        CHECK(treeShows(probe.tree, typedName) == false);

        stack.redo();
        QApplication::processEvents();
        CHECK(probe.nameAt(elemsBefore) == typedName);
        CHECK(treeShows(probe.tree, typedName));

        // 4. All the way back: the page stands exactly where it started.
        stack.undo();
        stack.undo();
        QApplication::processEvents();
        CHECK(probe.count() == elemsBefore);
        CHECK(countRows(probe.tree) == rowsBefore);
        CHECK(treeShows(probe.tree, typedName) == false);
    }

    /**
     * \brief   The two cells of the Methods list that edit through a drop-down: the method kind,
     *          and the method that answers it. Proves the column names as well, since both are
     *          the same in every editor now.
     * \param   otherKind   A kind of this document other than the one Add creates.
     * \param   replyKind   The kind that may be named as an answer, empty where the document has
     *                      none and the Reply column must therefore not exist.
     **/
    void exerciseMethodCells( const char* label, MethodPage* page, MethodDataSection& methods
                            , DocUndoStack& stack, const QString& otherKind, const QString& replyKind)
    {
        std::printf("--- %s\n", label);
        QTreeWidget* table = page->getList()->ctrlTableList();
        const bool hasReply = (replyKind.isEmpty() == false);

        CHECK(table->headerItem()->text(MethodListView::ColType) == QStringLiteral("Method Type:"));
        CHECK((table->columnCount() > static_cast<int>(MethodListView::ColReply)) == hasReply);
        if (hasReply)
        {
            CHECK(table->headerItem()->text(MethodListView::ColReply) == QStringLiteral("Reply:"));
        }

        const int before = static_cast<int>(methods.getElements().size());
        page->getList()->ctrlButtonAdd()->click();
        page->getList()->ctrlButtonAdd()->click();
        QApplication::processEvents();
        CHECK(static_cast<int>(methods.getElements().size()) == before + 2);

        MethodEntry* first  = methods.getElements().at(before);
        MethodEntry* second = methods.getElements().at(before + 1);
        const int addedKind = second->getKind();

        // The kind cell offers every kind the document has, and picking one is the edit the
        // details radio makes.
        CHECK(pickInCell(table, table->topLevelItem(before + 1), MethodListView::ColType, otherKind));
        QApplication::processEvents();
        CHECK(second->getKind() != addedKind);
        CHECK(second->kind().label == otherKind);

        int steps = 3;
        if (hasReply)
        {
            // A request may name its answer; the cell offers exactly the methods that can be one.
            CHECK(second->kind().label == replyKind);
            CHECK(pickInCell(table, table->topLevelItem(before), MethodListView::ColReply, second->getName()));
            QApplication::processEvents();
            CHECK(first->getReply() == second->getName());
            ++steps;
        }

        // Back to where the page started, so the next check runs on the document it expects.
        for (int i = 0; i < steps; ++i)
        {
            stack.undo();
        }

        QApplication::processEvents();
        CHECK(static_cast<int>(methods.getElements().size()) == before);
    }

    /**
     * \brief   The Overview page has no list and no rows: the document itself is what it edits.
     *          What is proved is the same, in the terms this page has -- a committed edit reaches
     *          the document as one undo step, the fields follow undo and redo, and the text a
     *          field still holds is handed over when the document is saved.
     * \param   nameEditable    False for a document whose name row is shown but not edited.
     **/
    void exerciseOverview( const char* label, OverviewPage* page, DocUndoStack& stack, bool nameEditable
                         , std::function<QString()> name, std::function<QString()> description
                         , std::function<uint32_t()> major, const QString& outDir, const QString& picture)
    {
        std::printf("--- %s\n", label);
        QLineEdit* nameField = page->findChild<QLineEdit*>(QStringLiteral("overviewName"));
        QLineEdit* majorField = page->findChild<QLineEdit*>(QStringLiteral("overviewMajor"));
        QPlainTextEdit* descriptionBox = page->findChild<QPlainTextEdit*>(QStringLiteral("overviewDescription"));
        CHECK(nameField != nullptr);
        CHECK(majorField != nullptr);
        CHECK(descriptionBox != nullptr);
        if ((nameField == nullptr) || (majorField == nullptr) || (descriptionBox == nullptr))
            return;

        // 1. The page shows what the document holds.
        CHECK(nameField->text() == name());
        CHECK(descriptionBox->toPlainText() == description());
        CHECK(nameField->isReadOnly() == (nameEditable == false));

        // 2. A version typed in reaches the document when the field is done with, not before:
        //    one undo step for the whole number rather than one per digit.
        const uint32_t majorBefore = major();
        majorField->setFocus();
        majorField->selectAll();
        typeText(majorField, QStringLiteral("7"));
        CHECK(major() == majorBefore);
        pressReturn(majorField);
        CHECK(major() == 7u);
        CHECK(stack.canUndo());

        // 3. A name typed in behaves the same, where the document's name is its own.
        const QString nameBefore = name();
        if (nameEditable)
        {
            nameField->setFocus();
            nameField->selectAll();
            typeText(nameField, QStringLiteral("ProbedMachine"));
            CHECK(name() == nameBefore);
            pressReturn(nameField);
            CHECK(name() == QStringLiteral("ProbedMachine"));
        }

        // 4. The description box gives its text to the document when the document is saved, with
        //    the caret still in it.
        const QString descriptionBefore = description();
        descriptionBox->setFocus();
        descriptionBox->clear();
        typeText(descriptionBox, QStringLiteral("Probed overview."));
        page->commitPendingEdits();
        CHECK(description() == QStringLiteral("Probed overview."));

        savePicture(page, outDir, picture);

        // 5. Undo and redo both reach the fields, because the page refills from the model.
        stack.undo();
        QApplication::processEvents();
        CHECK(description() == descriptionBefore);
        CHECK(descriptionBox->toPlainText() == descriptionBefore);
        stack.redo();
        QApplication::processEvents();
        CHECK(description() == QStringLiteral("Probed overview."));
        CHECK(descriptionBox->toPlainText() == QStringLiteral("Probed overview."));

        // 6. All the way back: the document stands exactly where it started.
        while (stack.canUndo())
        {
            stack.undo();
        }

        QApplication::processEvents();
        CHECK(major() == majorBefore);
        CHECK(name() == nameBefore);
        CHECK(majorField->text() == QString::number(majorBefore));
        CHECK(nameField->text() == nameBefore);
    }

    //!< Puts a page into a shown, active window, so the fields can take the focus.
    QWidget* showPage(QWidget* page)
    {
        QWidget* window = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(window);
        layout->setContentsMargins(0, 0, 0, 0);
        page->setParent(window);
        layout->addWidget(page);
        window->resize(1100, 720);
        window->show();
        window->activateWindow();
        QApplication::processEvents();
        return window;
    }
}

//////////////////////////////////////////////////////////////////////////
// The two documents
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testServiceInterfacePages(const QString& outDir)
    {
        std::printf("=== the shared pages over a service interface ===\n");
        ServiceInterfaceModel model;
        DocUndoStack& stack = model.getUndoStack();

        {
            // The interface is named by its file, so its name row is shown and not edited.
            SIOverview* page = new SIOverview(model.getOverviewModel());
            QWidget* window = showPage(page);
            exerciseOverview( "service interface / Overview", page, stack, true
                            , [&model]() { return model.getData().getOverviewData().getName(); }
                            , [&model]() { return model.getData().getOverviewData().getDescription(); }
                            , [&model]() { return model.getData().getOverviewData().getVersion().getMajor(); }
                            , outDir, QStringLiteral("si-overview"));
            delete window;
        }

        {
            ConstantPage* page = new ConstantPage(model.getConstantsModel(), QStringLiteral("Constants"));
            QWidget* window = showPage(page);
            ConstantDetailsView* details = page->findChild<ConstantDetailsView*>();
            PageProbe probe
            {
                  page
                , page->getList()->ctrlTableList()
                , page->getList()->ctrlButtonAdd()
                , (details != nullptr ? details->ctrlName() : nullptr)
                , [&model]() { return static_cast<int>(model.getConstantSection().getElements().size()); }
                , [&model](int at) { return model.getConstantSection().getElements().at(at).getName(); }
            };

            exercisePage("service interface / Constants", probe, stack, QStringLiteral("ProbedConstant"), outDir, QStringLiteral("si-constants"));
            delete window;
        }

        {
            DataTypePage* page = new DataTypePage(model.getDataTypeModel(), QStringLiteral("Data Types"));
            QWidget* window = showPage(page);
            DataTypeDetailsView* details = page->findChild<DataTypeDetailsView*>();
            PageProbe probe
            {
                  page
                , page->getList()->ctrlTableList()
                , page->getList()->ctrlButtonAdd()
                , (details != nullptr ? details->ctrlName() : nullptr)
                , [&model]() { return static_cast<int>(model.getDataTypeSection().getElements().size()); }
                , [&model](int at) { return model.getDataTypeSection().getElements().at(at)->getName(); }
            };

            exercisePage("service interface / Data Types", probe, stack, QStringLiteral("ProbedType"), outDir, QStringLiteral("si-datatypes"));
            delete window;
        }

        {
            AttributePage* page = new AttributePage(model.getAttributeModel(), QStringLiteral("Data Attributes"));
            QWidget* window = showPage(page);
            AttributeDetailsView* details = page->findChild<AttributeDetailsView*>();
            PageProbe probe
            {
                  page
                , page->getList()->ctrlTableList()
                , page->getList()->ctrlButtonAdd()
                , (details != nullptr ? details->ctrlName() : nullptr)
                , [&model]() { return static_cast<int>(model.getAttributeSection().getElements().size()); }
                , [&model](int at) { return model.getAttributeSection().getElements().at(at).getName(); }
            };

            exercisePage("service interface / Attributes", probe, stack, QStringLiteral("ProbedAttribute"), outDir, QStringLiteral("si-attributes"));
            delete window;
        }

        {
            // An interface includes no interface of its own kind: no document group, headers and
            // data types only. This is the configuration the editor itself passes.
            IncludePage* page = new IncludePage(model.getIncludesModel(), IncludeTypeConfig{}, QStringLiteral("Includes"));
            QWidget* window = showPage(page);
            IncludeListView* list = page->findChild<IncludeListView*>();
            IncludeDetailsView* details = page->findChild<IncludeDetailsView*>();
            PageProbe probe
            {
                  page
                , (list != nullptr ? list->ctrlTableList() : nullptr)
                , (list != nullptr ? list->ctrlButtonAdd() : nullptr)
                , (details != nullptr ? details->ctrlInclude() : nullptr)
                , [&model]() { return static_cast<int>(model.getIncludeSection().getElements().size()); }
                , [&model](int at) { return model.getIncludeSection().getElements().at(at).getName(); }
            };

            exercisePage("service interface / Includes", probe, stack, QStringLiteral("probe/Verified.hpp"), outDir, QStringLiteral("si-includes"));
            delete window;
        }

        {
            // An interface declares requests, the responses that answer them, and broadcasts.
            MethodPage* page = new MethodPage(model.getMethodsModel()
                                             , MethodPageConfig{ QStringLiteral("Methods")
                                                               , QStringLiteral("Service Methods List:")
                                                               , false });
            QWidget* window = showPage(page);
            MethodDetailsView* details = page->findChild<MethodDetailsView*>();
            PageProbe probe
            {
                  page
                , page->getList()->ctrlTableList()
                , page->getList()->ctrlButtonAdd()
                , (details != nullptr ? details->ctrlName() : nullptr)
                , [&model]() { return static_cast<int>(model.getMethodSection().getElements().size()); }
                , [&model](int at) { return model.getMethodSection().getElements().at(at)->getName(); }
            };

            exercisePage("service interface / Methods", probe, stack, QStringLiteral("probedRequest"), outDir, QStringLiteral("si-methods"));
            exerciseMethodCells( "service interface / Methods cells", page, model.getMethodSection(), stack
                               , QStringLiteral("Response"), QStringLiteral("Response"));
            delete window;
        }
    }

    void testStateMachinePages(const QString& documentPath, const QString& outDir)
    {
        std::printf("=== the shared pages over a state machine ===\n");
        StateMachineModel model;
        if (model.loadFromFile(documentPath) == false)
        {
            std::printf("  [FAIL] cannot read %s\n", qPrintable(documentPath));
            ++gFailures;
            return;
        }

        DocUndoStack& stack = model.getUndoStack();

        {
            SMOverview* page = new SMOverview(model.getOverviewModel());
            QWidget* window = showPage(page);
            exerciseOverview( "state machine / Overview", page, stack, true
                            , [&model]() { return model.getData().getOverview().getName(); }
                            , [&model]() { return model.getData().getOverview().getDescription(); }
                            , [&model]() { return model.getData().getOverview().getVersion().getMajor(); }
                            , outDir, QStringLiteral("sm-overview"));
            delete window;
        }

        {
            SMConstant* page = new SMConstant(model.getConstantModel(), model);
            QWidget* window = showPage(page);
            ConstantDetailsView* details = page->findChild<ConstantDetailsView*>();
            PageProbe probe
            {
                  page
                , page->getList()->ctrlTableList()
                , page->getList()->ctrlButtonAdd()
                , (details != nullptr ? details->ctrlName() : nullptr)
                , [&model]() { return static_cast<int>(model.getConstantSection().getElements().size()); }
                , [&model](int at) { return model.getConstantSection().getElements().at(at).getName(); }
            };

            exercisePage("state machine / Constants", probe, stack, QStringLiteral("ProbedConstant"), outDir, QStringLiteral("sm-constants"));
            delete window;
        }

        {
            DataTypePage* page = new DataTypePage(model.getDataTypeModel(), QStringLiteral("Data Types"));
            QWidget* window = showPage(page);
            DataTypeDetailsView* details = page->findChild<DataTypeDetailsView*>();
            PageProbe probe
            {
                  page
                , page->getList()->ctrlTableList()
                , page->getList()->ctrlButtonAdd()
                , (details != nullptr ? details->ctrlName() : nullptr)
                , [&model]() { return static_cast<int>(model.getDataTypeSection().getElements().size()); }
                , [&model](int at) { return model.getDataTypeSection().getElements().at(at)->getName(); }
            };

            exercisePage("state machine / Data Types", probe, stack, QStringLiteral("ProbedType"), outDir, QStringLiteral("sm-datatypes"));
            delete window;
        }

        {
            SMAttribute* page = new SMAttribute(model.getAttributeModel(), model);
            QWidget* window = showPage(page);
            AttributeDetailsView* details = page->findChild<AttributeDetailsView*>();
            PageProbe probe
            {
                  page
                , page->getList()->ctrlTableList()
                , page->getList()->ctrlButtonAdd()
                , (details != nullptr ? details->ctrlName() : nullptr)
                , [&model]() { return static_cast<int>(model.getAttributeSection().getElements().size()); }
                , [&model](int at) { return model.getAttributeSection().getElements().at(at).getName(); }
            };

            exercisePage("state machine / Attributes", probe, stack, QStringLiteral("ProbedAttribute"), outDir, QStringLiteral("sm-attributes"));
            delete window;
        }

        {
            SMInclude* page = new SMInclude(model.getIncludeModel());
            QWidget* window = showPage(page);
            IncludeListView* list = page->findChild<IncludeListView*>();
            IncludeDetailsView* details = page->findChild<IncludeDetailsView*>();
            PageProbe probe
            {
                  page
                , (list != nullptr ? list->ctrlTableList() : nullptr)
                , (list != nullptr ? list->ctrlButtonAdd() : nullptr)
                , (details != nullptr ? details->ctrlInclude() : nullptr)
                , [&model]() { return static_cast<int>(model.getIncludeSection().getElements().size()); }
                , [&model](int at) { return model.getIncludeSection().getElements().at(at).getName(); }
            };

            exercisePage("state machine / Includes", probe, stack, QStringLiteral("probe/Verified.hpp"), outDir, QStringLiteral("sm-includes"));
            delete window;
        }

        {
            SMMethod* page = new SMMethod(model.getMethodModel(), model);
            QWidget* window = showPage(page);
            MethodDetailsView* details = page->findChild<MethodDetailsView*>();
            PageProbe probe
            {
                  page
                , page->getList()->ctrlTableList()
                , page->getList()->ctrlButtonAdd()
                , (details != nullptr ? details->ctrlName() : nullptr)
                , [&model]() { return static_cast<int>(model.getMethodSection().getElements().size()); }
                , [&model](int at) { return model.getMethodSection().getElements().at(at)->getName(); }
            };

            exercisePage("state machine / Methods", probe, stack, QStringLiteral("probedTrigger"), outDir, QStringLiteral("sm-methods"));
            exerciseMethodCells( "state machine / Methods cells", page, model.getMethodSection(), stack
                               , QStringLiteral("Condition"), QString());
            delete window;
        }
    }
}


//////////////////////////////////////////////////////////////////////////
// The data type document
//////////////////////////////////////////////////////////////////////////

namespace
{
    void testDataTypeDocumentPages(const QString& outDir)
    {
        std::printf("=== the shared pages over a data type document ===\n");
        DataTypeDocumentModel model;
        DocUndoStack& stack = model.getUndoStack();

        {
            // A data type document owns its name, the way a service interface does, so the name
            // row is edited here. It has neither a category nor a threading row.
            OverviewPageConfig config;
            config.headline        = QStringLiteral("Data Type Document Overview");
            config.versionTitle    = QStringLiteral("Document Version:");
            config.descriptionHint = QStringLiteral("Describe the data types collected here");
            config.nameEditable    = true;
            config.links           =
            {
                  { 1, QStringLiteral("linkDataTypes"), QStringLiteral("Data Types ...")
                  , QStringLiteral("Click to open the Data Types page"), QStringLiteral("Open Data Types Page ...") }
                , { 2, QStringLiteral("linkIncludes") , QStringLiteral("Includes ...")
                  , QStringLiteral("Click to open the Includes page"), QStringLiteral("Open Includes Page ...") }
            };

            OverviewPage* page = new OverviewPage(model.getOverviewModel(), config);
            page->refreshAll();
            QWidget* window = showPage(page);

            // Exactly two quick links, and neither of the rows the other two documents add.
            CHECK(page->findChild<QAbstractButton*>(QStringLiteral("linkDataTypes")) != nullptr);
            CHECK(page->findChild<QAbstractButton*>(QStringLiteral("linkIncludes")) != nullptr);
            CHECK(page->findChild<QAbstractButton*>(QStringLiteral("overviewCategoryPublic")) == nullptr);
            CHECK(page->findChild<QAbstractButton*>(QStringLiteral("overviewThreadingShared")) == nullptr);

            exerciseOverview( "data type document / Overview", page, stack, true
                            , [&model]() { return model.getData().getOverviewData().getName(); }
                            , [&model]() { return model.getData().getOverviewData().getDescription(); }
                            , [&model]() { return model.getData().getOverviewData().getVersion().getMajor(); }
                            , outDir, QStringLiteral("dt-overview"));
            delete window;
        }

        {
            DataTypePage* page = new DataTypePage(model.getDataTypeModel(), QStringLiteral("Data Types"));
            QWidget* window = showPage(page);
            DataTypeDetailsView* details = page->findChild<DataTypeDetailsView*>();
            PageProbe probe
            {
                  page
                , page->getList()->ctrlTableList()
                , page->getList()->ctrlButtonAdd()
                , (details != nullptr ? details->ctrlName() : nullptr)
                , [&model]() { return static_cast<int>(model.getDataTypeSection().getElements().size()); }
                , [&model](int at) { return model.getDataTypeSection().getElements().at(at)->getName(); }
            };

            exercisePage("data type document / Data Types", probe, stack, QStringLiteral("ProbedType"), outDir, QStringLiteral("dt-datatypes"));
            delete window;
        }

        {
            // A data type document is a leaf: C++ headers only, so its Includes page shows one
            // group -- neither a document group nor the Data Types group the other two have.
            IncludeTypeConfig config{};
            config.takesDataTypes = false;
            IncludePage* page = new IncludePage(model.getIncludesModel(), config, QStringLiteral("Includes"));
            QWidget* window = showPage(page);
            IncludeListView* list = page->findChild<IncludeListView*>();
            IncludeDetailsView* details = page->findChild<IncludeDetailsView*>();
            CHECK(list != nullptr);
            if (list != nullptr)
            {
                CHECK(list->ctrlTableList()->topLevelItemCount() == 1);
                CHECK(treeShows(list->ctrlTableList(), QStringLiteral("Data Types (0)")) == false);
            }

            PageProbe probe
            {
                  page
                , (list != nullptr ? list->ctrlTableList() : nullptr)
                , (list != nullptr ? list->ctrlButtonAdd() : nullptr)
                , (details != nullptr ? details->ctrlInclude() : nullptr)
                , [&model]() { return static_cast<int>(model.getIncludeSection().getElements().size()); }
                , [&model](int at) { return model.getIncludeSection().getElements().at(at).getName(); }
            };

            exercisePage("data type document / Includes", probe, stack, QStringLiteral("probe/Verified.hpp"), outDir, QStringLiteral("dt-includes"));
            delete window;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// The Data Types page of a document that reads types out of an included one
//////////////////////////////////////////////////////////////////////////

namespace
{
    //!< Writes \p content to \p path, reporting the failure rather than going on quietly.
    bool writeFixture(const QString& path, const QString& content)
    {
        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text) == false)
        {
            return false;
        }

        QTextStream stream(&file);
        stream << content;
        file.close();
        return true;
    }

    void testImportedTypesPage(const QString& outDir)
    {
        std::printf("=== the Data Types page over an included data type document ===\n");

        QTemporaryDir dir;
        CHECK(dir.isValid());
        if (dir.isValid() == false)
        {
            return;
        }

        const bool wroteTypes = writeFixture(dir.filePath(QStringLiteral("Shared.dtml")),
            QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                           "<DataTypeDocument FormatVersion=\"1.0.0\">\n"
                           "    <Overview ID=\"50\" Name=\"Shared\" Version=\"1.0.0\"/>\n"
                           "    <DataTypeList>\n"
                           "        <DataType ID=\"51\" Name=\"Unit\" Type=\"Enumeration\" Values=\"uint16\">\n"
                           "            <FieldList>\n"
                           "                <EnumEntry ID=\"52\" Name=\"Celsius\" Value=\"0\"/>\n"
                           "            </FieldList>\n"
                           "        </DataType>\n"
                           "        <DataType ID=\"53\" Name=\"Reading\" Type=\"Structure\">\n"
                           "            <FieldList>\n"
                           "                <Field ID=\"54\" Name=\"value\" DataType=\"uint32\"/>\n"
                           "            </FieldList>\n"
                           "        </DataType>\n"
                           "    </DataTypeList>\n"
                           "</DataTypeDocument>\n"));
        CHECK(wroteTypes);

        const QString hostPath = dir.filePath(QStringLiteral("Sensor.siml"));
        const bool wroteHost = writeFixture(hostPath,
            QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                           "<ServiceInterface FormatVersion=\"1.1.0\">\n"
                           "    <Overview ID=\"1\" Name=\"Sensor\" Version=\"1.0.0\" Category=\"Public\"/>\n"
                           "    <DataTypeList>\n"
                           "        <DataType ID=\"5\" Name=\"Local\" Type=\"Structure\">\n"
                           "            <FieldList>\n"
                           "                <Field ID=\"6\" Name=\"unit\" DataType=\"Shared::Unit\"/>\n"
                           "            </FieldList>\n"
                           "        </DataType>\n"
                           "    </DataTypeList>\n"
                           "    <IncludeList>\n"
                           "        <Location ID=\"20\" Name=\"./Shared.dtml\"/>\n"
                           "    </IncludeList>\n"
                           "</ServiceInterface>\n"));
        CHECK(wroteHost);

        ServiceInterfaceModel model(hostPath);
        CHECK(model.openSucceeded());

        DataTypePage* page = new DataTypePage(model.getDataTypeModel(), QStringLiteral("Data Types"));
        QWidget* window = showPage(page);
        QTreeWidget* tree = page->getList()->ctrlTableList();
        DataTypeDetailsView* details = page->findChild<DataTypeDetailsView*>();
        CHECK(details != nullptr);

        // The document's own type first, then one heading per included document.
        CHECK(tree->topLevelItemCount() == 2);
        CHECK(treeShows(tree, QStringLiteral("Local")));
        CHECK(treeShows(tree, QStringLiteral("Shared")));

        QTreeWidgetItem* group = (tree->topLevelItemCount() == 2 ? tree->topLevelItem(1) : nullptr);
        CHECK((group != nullptr) && (group->childCount() == 2));

        // The heading names a document, not a declaration, so nothing is selected through it.
        if (group != nullptr)
        {
            tree->setCurrentItem(group);
            QApplication::processEvents();
            CHECK(page->getList()->ctrlButtonRemove()->isEnabled() == false);
        }

        // An imported row shows what it declares and lets none of it be touched.
        QTreeWidgetItem* importedRow = ((group != nullptr) && (group->childCount() == 2)) ? group->child(0) : nullptr;
        CHECK(importedRow != nullptr);
        if (importedRow != nullptr)
        {
            group->setExpanded(true);
            tree->setCurrentItem(importedRow);
            QApplication::processEvents();

            // The value column carries the spelling the author has to write.
            CHECK(importedRow->text(2) == QStringLiteral("Shared::Unit"));
            CHECK((details != nullptr) && (details->isEnabled() == false));
            CHECK(page->getList()->ctrlButtonRemove()->isEnabled() == false);
            CHECK(page->getList()->ctrlButtonAddChild()->isEnabled() == false);

            // Neither the document's own types nor the borrowed ones moved.
            CHECK(model.getDataTypeSection().getElements().size() == 1);
            CHECK(model.getDataTypeSection().getImportedTypes().size() == 2);
        }

        // Back on the document's own type, everything is editable again.
        tree->setCurrentItem(tree->topLevelItem(0));
        QApplication::processEvents();
        CHECK((details != nullptr) && details->isEnabled());
        CHECK(page->getList()->ctrlButtonRemove()->isEnabled());

        savePicture(page, outDir, QStringLiteral("si-imported-datatypes"));
        delete window;
    }
}

//////////////////////////////////////////////////////////////////////////
// The editing gestures every list page shares
//////////////////////////////////////////////////////////////////////////

namespace
{
    //!< The name shown on the current row of a tree, or an empty string when nothing is current.
    QString currentRowName(const QTreeWidget* tree)
    {
        const QTreeWidgetItem* item = tree->currentItem();
        return (item != nullptr) ? item->text(0) : QString();
    }

    //!< The rectangle a group box occupies inside the page it sits on.
    QRect groupRect(const QWidget* page, const QGroupBox* group)
    {
        return QRect(group->mapTo(const_cast<QWidget*>(page), QPoint(0, 0)), group->size());
    }

    /**
     * \brief   The two panels of a page take the same width and leave the same margin on either
     *          side of the page. Every page splits its width the same way.
     **/
    void checkTwoColumnGeometry(const char* label, QWidget* page)
    {
        std::printf("--- %s\n", label);
        QRect left;
        QRect right;
        for (QGroupBox* box : page->findChildren<QGroupBox*>())
        {
            // Only the outermost panels: a group nested inside one of them has a group above it.
            bool nested = false;
            for (QWidget* up = box->parentWidget(); up != nullptr; up = up->parentWidget())
            {
                if (qobject_cast<QGroupBox*>(up) != nullptr)
                {
                    nested = true;
                    break;
                }
            }

            if (nested)
                continue;

            const QRect rect = groupRect(page, box);
            QRect& side = (rect.center().x() < (page->width() / 2)) ? left : right;
            if (side.isNull() || (rect.top() < side.top()))
            {
                side = rect;
            }
        }

        std::printf("    left  = x %d..%d (w %d)  y %d..%d (h %d)\n", left.left(), left.right(), left.width(), left.top(), left.bottom(), left.height());
        std::printf("    right = x %d..%d (w %d)  y %d..%d (h %d)\n", right.left(), right.right(), right.width(), right.top(), right.bottom(), right.height());
        CHECK(left.isNull() == false);
        CHECK(right.isNull() == false);
        // One pixel of slack: an odd width cannot be split in two equal halves.
        CHECK(qAbs(left.width() - right.width()) <= 1);
        CHECK(qAbs(left.left() - (page->width() - 1 - right.right())) <= 1);
        CHECK(left.top() == right.top());
        CHECK(qAbs(left.height() - right.height()) <= 1);
    }

    /**
     * \brief   Moving a row up or down carries the selection with it. A section keeps its IDs in
     *          list order, so the moved element ends up carrying the ID of the one it passed; a
     *          page that reselects by the ID it read before the move lands on the wrong row.
     **/
    void checkMoveKeepsSelection(const char* label, QTreeWidget* tree, QToolButton* up, QToolButton* down, QTreeWidgetItem* second)
    {
        std::printf("--- %s\n", label);
        tree->setCurrentItem(second);
        QApplication::processEvents();
        const QString moving = currentRowName(tree);
        CHECK(moving.isEmpty() == false);

        CHECK(up->isEnabled());
        up->click();
        QApplication::processEvents();
        CHECK(currentRowName(tree) == moving);

        CHECK(down->isEnabled());
        down->click();
        QApplication::processEvents();
        CHECK(currentRowName(tree) == moving);
    }

    /**
     * \brief   The row context menu carries the toolbar commands with the state they have now,
     *          and the entries a keyboard user reaches by shortcut.
     **/
    void checkContextMenu(const char* label, ElementListView* list, int expectedEntries)
    {
        std::printf("--- %s\n", label);
        QMenu menu;
        list->buildContextMenu(menu);

        QStringList entries;
        int enabled = 0;
        for (const QAction* action : menu.actions())
        {
            if (action->isSeparator())
                continue;

            entries.append(action->text());
            if (action->isEnabled())
            {
                ++enabled;
            }
        }

        std::printf("    %s\n", entries.join(QStringLiteral(" | ")).toUtf8().constData());
        CHECK(entries.isEmpty() == false);
        CHECK(enabled > 0);
        // Every toolbar command is offered, plus rename, copy and the tree commands.
        CHECK(entries.size() == expectedEntries);
    }

    /**
     * \brief   The Methods page gestures both editors share: a parameter is added, moved and
     *          deleted from the toolbar, and the default values stay at the end of the parameter
     *          list, the way C++ needs them to.
     **/
    void testMethodParameterGestures(const char* label, MethodPage* page, MethodDataSection& methods, DocUndoStack& stack)
    {
        std::printf("=== %s ===\n", label);
        MethodListView* list = page->getList();
        QTreeWidget* tree = list->ctrlTableList();
        MethodParamDetailsView* details = page->findChild<MethodParamDetailsView*>();
        CHECK(details != nullptr);
        if (details == nullptr)
            return;

        const int methodsBefore = static_cast<int>(methods.getElements().size());
        list->ctrlButtonAdd()->click();
        QApplication::processEvents();
        CHECK(static_cast<int>(methods.getElements().size()) == (methodsBefore + 1));

        MethodEntry* method = methods.getElements().at(methodsBefore);
        CHECK(method != nullptr);
        if (method == nullptr)
            return;

        // 1. Two parameters, added from the toolbar.
        list->ctrlButtonAddChild()->click();
        QApplication::processEvents();
        list->ctrlButtonAddChild()->click();
        QApplication::processEvents();
        CHECK(method->getElementCount() == 2);

        // The page rebuilds its tree from the model on every change, so a row pointer only
        // lives until the next one. Ask for the row again before every use.
        auto methodRow = [tree, methodsBefore]() { return tree->topLevelItem(methodsBefore); };
        CHECK((methodRow() != nullptr) && (methodRow()->childCount() == 2));
        if ((methodRow() == nullptr) || (methodRow()->childCount() != 2))
            return;

        // 2. Moving the second parameter up keeps the selection on it, and moving it back down
        //    puts it where it started.
        checkMoveKeepsSelection("parameter move keeps the selection", tree, list->ctrlButtonMoveUp(), list->ctrlButtonMoveDown(), methodRow()->child(1));

        // 3. The parameter trio of the toolbar is live only on a parameter row.
        tree->setCurrentItem(methodRow()->child(1));
        QApplication::processEvents();
        CHECK(list->ctrlButtonAddChild()->isEnabled());
        CHECK(list->ctrlButtonInsertChild()->isEnabled());
        CHECK(list->ctrlButtonRemoveChild()->isEnabled());
        tree->setCurrentItem(methodRow());
        QApplication::processEvents();
        CHECK(list->ctrlButtonInsertChild()->isEnabled() == false);
        CHECK(list->ctrlButtonRemoveChild()->isEnabled() == false);

        // 4. A default value on the last parameter, and the parameter added after it comes up
        //    carrying one too, with its checkbox refusing to give it away.
        tree->setCurrentItem(methodRow()->child(1));
        QApplication::processEvents();
        CHECK(details->ctrlHasDefault()->isEnabled());
        details->ctrlHasDefault()->setChecked(true);
        QApplication::processEvents();
        CHECK(method->getElements().at(1).hasDefault());

        list->ctrlButtonAddChild()->click();
        QApplication::processEvents();
        CHECK(method->getElementCount() == 3);
        CHECK(method->getElements().at(2).hasDefault());
        CHECK(details->ctrlHasDefault()->isChecked());
        CHECK(details->ctrlHasDefault()->isEnabled() == false);

        // The first of the two carrying a default may still give it away.
        tree->setCurrentItem(methodRow()->child(1));
        QApplication::processEvents();
        CHECK(details->ctrlHasDefault()->isEnabled());

        // 5. A parameter carrying a default value cannot be moved in front of one without.
        CHECK(list->ctrlButtonMoveUp()->isEnabled() == false);
        CHECK(list->ctrlButtonMoveDown()->isEnabled());

        // 6. And the plain one before them cannot be moved past them.
        tree->setCurrentItem(methodRow()->child(0));
        QApplication::processEvents();
        CHECK(list->ctrlButtonMoveDown()->isEnabled() == false);

        // 7. The row context menu carries the same commands as the toolbar.
        checkContextMenu("Methods context menu", list, 12);

        // 8. Delete removes the selected parameter, not the method that declares it.
        tree->setCurrentItem(methodRow()->child(2));
        QApplication::processEvents();
        list->ctrlButtonRemoveChild()->click();
        QApplication::processEvents();
        CHECK(method->getElementCount() == 2);
        CHECK(static_cast<int>(methods.getElements().size()) == (methodsBefore + 1));

        // All the way back.
        while (stack.canUndo())
        {
            stack.undo();
        }

        QApplication::processEvents();
        CHECK(static_cast<int>(methods.getElements().size()) == methodsBefore);
    }

    //!< The Data Types page gesture: reordering a field carries the selection with it.
    void testDataTypeFieldGestures(const char* label, DataTypePage* page, DataTypeDataSection& types, DocUndoStack& stack)
    {
        std::printf("=== %s ===\n", label);
        DataTypeListView* list = page->getList();
        QTreeWidget* tree = list->ctrlTableList();

        const int typesBefore = static_cast<int>(types.getElements().size());
        list->ctrlButtonAdd()->click();
        QApplication::processEvents();
        CHECK(static_cast<int>(types.getElements().size()) == (typesBefore + 1));

        list->ctrlButtonAddChild()->click();
        QApplication::processEvents();
        list->ctrlButtonAddChild()->click();
        QApplication::processEvents();

        QTreeWidgetItem* typeRow = tree->topLevelItem(typesBefore);
        CHECK((typeRow != nullptr) && (typeRow->childCount() == 2));
        if ((typeRow == nullptr) || (typeRow->childCount() != 2))
            return;

        checkMoveKeepsSelection("field move keeps the selection", tree, list->ctrlButtonMoveUp(), list->ctrlButtonMoveDown(), typeRow->child(1));
        CHECK(tree->topLevelItem(typesBefore)->childCount() == 2);

        while (stack.canUndo())
        {
            stack.undo();
        }

        QApplication::processEvents();
        CHECK(static_cast<int>(types.getElements().size()) == typesBefore);
    }

    void testEditingGestures(const QString& outDir)
    {
        std::printf("=== the editing gestures over a service interface ===\n");
        ServiceInterfaceModel model;
        DocUndoStack& stack = model.getUndoStack();

        {
            SIOverview* page = new SIOverview(model.getOverviewModel());
            QWidget* window = showPage(page);
            checkTwoColumnGeometry("service interface / Overview column widths", page);
            savePicture(page, outDir, QStringLiteral("si-overview-columns"));
            delete window;
        }

        {
            MethodPage* page = new MethodPage(model.getMethodsModel(), MethodPageConfig{ QStringLiteral("Methods"), QStringLiteral("Methods List:"), false });
            QWidget* window = showPage(page);
            checkTwoColumnGeometry("service interface / Methods column widths", page);
            testMethodParameterGestures("service interface / Methods parameters", page, model.getMethodSection(), stack);
            savePicture(page, outDir, QStringLiteral("si-methods-params"));
            delete window;
        }

        {
            DataTypePage* page = new DataTypePage(model.getDataTypeModel(), QStringLiteral("Data Types"));
            QWidget* window = showPage(page);
            testDataTypeFieldGestures("service interface / Data Types fields", page, model.getDataTypeSection(), stack);
            delete window;
        }

        {
            AttributePage* page = new AttributePage(model.getAttributeModel(), QStringLiteral("Attributes"));
            QWidget* window = showPage(page);
            AttributeListView* list = page->findChild<AttributeListView*>();
            CHECK(list != nullptr);
            if (list != nullptr)
            {
                QTreeWidget* tree = list->ctrlTableList();
                const int before = tree->topLevelItemCount();
                list->ctrlButtonAdd()->click();
                QApplication::processEvents();
                list->ctrlButtonAdd()->click();
                QApplication::processEvents();
                CHECK(tree->topLevelItemCount() == (before + 2));
                checkMoveKeepsSelection("attribute move keeps the selection", tree, list->ctrlButtonMoveUp(), list->ctrlButtonMoveDown(), tree->topLevelItem(before + 1));
                checkContextMenu("Attributes context menu", list, 7);
                CHECK(tree->contextMenuPolicy() == Qt::CustomContextMenu);
            }

            while (stack.canUndo())
            {
                stack.undo();
            }

            QApplication::processEvents();
            delete window;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Entry point
//////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);
    if (argc < 2)
    {
        std::printf("Usage: %s <TrafficLight.fsml> [output directory]\n", argv[0]);
        return 2;
    }

    const QString documentPath = QString::fromLocal8Bit(argv[1]);
    const QString outDir = (argc >= 3) ? QString::fromLocal8Bit(argv[2]) : QDir::tempPath();
    QDir().mkpath(outDir);

    testServiceInterfacePages(outDir);
    testStateMachinePages(documentPath, outDir);
    testDataTypeDocumentPages(outDir);
    testImportedTypesPage(outDir);
    testEditingGestures(outDir);

    std::printf("=== %d checks, %d failure(s) ===\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
