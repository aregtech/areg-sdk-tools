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
 *               The Constants, Data Types, Includes, Attributes and Methods pages are one class
 *               each, used by the service interface editor and by the state machine editor alike.
 *               What is proved here, for all ten combinations:
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
#include "lusan/view/sm/SMAttribute.hpp"
#include "lusan/view/sm/SMConstant.hpp"
#include "lusan/view/sm/SMInclude.hpp"
#include "lusan/view/sm/SMMethod.hpp"

#include <QApplication>
#include <QDir>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPixmap>
#include <QToolButton>
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
                                                               , QStringLiteral("Data Type:")
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

    std::printf("=== %d checks, %d failure(s) ===\n", gChecks, gFailures);
    return (gFailures == 0) ? 0 : 1;
}
