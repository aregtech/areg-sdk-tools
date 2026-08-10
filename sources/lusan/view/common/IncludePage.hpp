#ifndef LUSAN_VIEW_COMMON_INCLUDEPAGE_HPP
#define LUSAN_VIEW_COMMON_INCLUDEPAGE_HPP
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
 *  \file        lusan/view/common/IncludePage.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Includes page shared by every document editor.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/common/DocIssue.hpp"
#include "lusan/view/common/IEditCommit.hpp"
#include "lusan/view/common/IncludeListView.hpp"
#include "lusan/view/common/TableCell.hpp"

#include <QScrollArea>
#include <QStringList>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class IncludeDetailsView;
class IncludeEntry;
class IncludeModel;
class QEvent;
class QStackedWidget;
class QTreeWidgetItem;

/**
 * \class   IncludePage
 * \brief   The Includes page: one list of everything the document pulls in from outside -- C++
 *          headers, data type documents, and documents of the host's own kind where the host has
 *          them -- under its headings, with the selected entry's editor beside it.
 *
 *          One list rather than several, because a reader should not have to know which registry
 *          a file belongs to before they can look for it or add it.
 *
 *          Every edit is committed through the model's undo commands; the page mutates no data
 *          itself and rebuilds the list from the live model on every notifier signal, so a row
 *          can never drift from what was stored -- after an undo, a redo or a reload alike.
 *
 *          A document with extra meaning for a row subclasses this: see \ref selectedInclude,
 *          \ref confirmRemove, \ref acceptLocation and \ref commitBrowsedLocation.
 **/
class IncludePage : public    QScrollArea
                  , public    IEditCommit
                  , protected IETableHelper
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Builds the page.
     * \param   model       The includes model of the document being edited.
     * \param   config      What the host document may include besides headers and data types.
     * \param   headline    The title shown above the two panels.
     * \param   parent      The parent widget.
     **/
    explicit IncludePage(IncludeModel& model, const IncludeTypeConfig& config, const QString& headline, QWidget* parent = nullptr);

    virtual ~IncludePage(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Selects the include with the given ID, expanding its group first, and puts the
     *          accent on the field at fault when the check names one.
     * \param   id      The document ID of the include to reveal.
     * \param   field   The field to accent once the row is selected.
     * \return  False when no row carries that ID.
     **/
    bool revealElement(uint32_t id, eIssueField field = eIssueField::None);

    /**
     * \brief   Hands over the description text the page is still holding. The box applies its
     *          text when it loses the focus, which a save from the keyboard never causes.
     **/
    virtual void commitPendingEdits(void) override;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Populates the details panel for the given include and brings the matching editor
     *          forward. A document with an editor of its own for some row kind overrides this.
     **/
    virtual void selectedInclude(const IncludeEntry* entry);

    /**
     * \brief   Clears the details panel and disables the row-only tool buttons.
     **/
    virtual void showClean(void);

    /**
     * \brief   Asks whether the include may be deleted. The base page always agrees; a document
     *          that can tell where the file is used warns about that first.
     **/
    virtual bool confirmRemove(uint32_t id);

    /**
     * \brief   Asks whether a location the user typed or picked may be committed. The base page
     *          always agrees; a document that resolves the file checks it first.
     **/
    virtual bool acceptLocation(const QString& location);

    /**
     * \brief   Stores the file the user picked in the browse dialog. The base page stores the
     *          path relative to the workspace.
     * \param   id              The include being edited.
     * \param   absolutePath    The picked file.
     * \param   relativePath    The same file, relative to the workspace.
     **/
    virtual void commitBrowsedLocation(uint32_t id, const QString& absolutePath, const QString& relativePath);

    /**
     * \brief   The browse dialog filters: C++ sources, `.dtml` data types, and the host's own
     *          document kind where it has one.
     **/
    virtual QStringList getSupportedExtensions(void) const;

    virtual bool eventFilter(QObject* watched, QEvent* event) override;

    virtual int getColumnCount(void) const override;

    virtual QString getCellText(const QModelIndex& cell) const override;

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
protected slots:
    void onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void onAddClicked(void);
    void onInsertClicked(void);
    void onRemoveClicked(void);
    void onMoveUpClicked(void);
    void onMoveDownClicked(void);

    void onLocationCommitted(void);

    //!< Mirrors the typed path into the selected row while it is typed; the model commit still
    //!< happens once on editing-finished.
    void onLocationTextChanged(const QString& text);

    //!< Commits an inline edit of the Location column into the model (undo command); the derived
    //!< Type/Name columns then refresh from the notifier. Escape is handled by the delegate.
    void onInlineLocationEdited(const QModelIndex& index, const QString& newValue);

    void onBrowseClicked(void);
    void onDeprecatedToggled(bool checked);
    void onDeprecateHintCommitted(void);

    //!< Re-derives the Type/Name/Version columns from the current include locations.
    void onUpdateClicked(void);

    //!< Rebuilds the whole list on any include-kind notifier signal.
    void onNotifierChanged(void);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
protected:
    //!< Rebuilds the whole list from the live model and restores the selection by ID.
    void refreshAll(void);

    //!< Selects the include by ID; returns false if not found (and selects nothing).
    bool selectInclude(uint32_t id);

    //!< The include ID stored on the currently selected row, or 0 if none.
    uint32_t currentIncludeId(void) const;

    void updateMoveButtons(int row, int rowCount);

    //!< Puts the caret in the selected include's Location field with its text selected. An
    //!< include has no name of its own, the location is what identifies it.
    void focusLocationField(void);

    inline IncludeModel& getModel(void) const;
    inline IncludeListView* getList(void) const;
    inline IncludeDetailsView* getDetails(void) const;
    inline QStackedWidget* getDetailsStack(void) const;

private:
    void buildUi(const QString& headline);
    void setupSignals(void);

    //!< Fills one entry row and moves it into the group its location belongs to.
    void setNodeText(QTreeWidgetItem* node, const IncludeEntry& entry) const;

    //!< A placeholder location for a new entry, unique in the registry.
    QString genName(void);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    IncludeModel&       mModel;         //!< The includes of the document being edited.
    IncludeListView*    mList;          //!< The list panel.
    QStackedWidget*     mDetailsStack;  //!< Swaps the editor by the selected row's kind.
    IncludeDetailsView* mDetails;       //!< Source and data type editor.
    TableCell*          mTableCell;     //!< Inline editor delegate for the Location column.
    uint32_t            mNameCounter;   //!< Seeds the placeholder location of a new entry.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    IncludePage(const IncludePage& /*src*/) = delete;
    IncludePage& operator = (const IncludePage& /*src*/) = delete;
};

//////////////////////////////////////////////////////////////////////////
// IncludePage inline methods
//////////////////////////////////////////////////////////////////////////

inline IncludeModel& IncludePage::getModel(void) const
{
    return mModel;
}

inline IncludeListView* IncludePage::getList(void) const
{
    return mList;
}

inline IncludeDetailsView* IncludePage::getDetails(void) const
{
    return mDetails;
}

inline QStackedWidget* IncludePage::getDetailsStack(void) const
{
    return mDetailsStack;
}

#endif  // LUSAN_VIEW_COMMON_INCLUDEPAGE_HPP
