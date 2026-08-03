#ifndef LUSAN_VIEW_SM_SMINCLUDE_HPP
#define LUSAN_VIEW_SM_SMINCLUDE_HPP
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
 *  \file        lusan/view/sm/SMInclude.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Includes page.
 *
 ************************************************************************/

#include <QScrollArea>
#include "lusan/view/common/IEditCommit.hpp"
#include "lusan/view/common/TableCell.hpp"

#include "lusan/data/common/IncludeEntry.hpp"

#include <QStringList>
#include <cstdint>

class IncludeDetailsView;
class IncludeListView;
class QEvent;
class QLabel;
class QLineEdit;
class QModelIndex;
class QPlainTextEdit;
class QPushButton;
class QStackedWidget;
class QTreeWidgetItem;
class SMIncludeModel;

/**
 * \brief   The FSM Includes page: one list of everything the document pulls in from outside --
 *          C++ headers, data type documents and imported state machines -- under three
 *          headings, with the selected entry's editor beside it.
 *
 *          One list rather than two, because a reader should not have to know which registry a
 *          file belongs to before they can look for it or add it. The detail editor swaps by the
 *          selected row's kind: an imported machine has an alias, a pinned version and a
 *          resolution status; a header has a deprecation flag; a data type has neither.
 *
 *          Every edit is committed through the page model's undo commands; the page mutates no
 *          model state directly and rebuilds the list from the live model on every notifier
 *          signal, self-triggered or from undo/redo alike.
 **/
class SMInclude : public    QScrollArea
                , public    IEditCommit
                , protected IETableHelper
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMInclude(SMIncludeModel& model, QWidget* parent = nullptr);
    virtual ~SMInclude() = default;

    /**
     * \brief   Selects the include with the given ID, expanding its group first
     *          (go-to-declaration landing).
     **/
    bool revealElement(uint32_t id);

    /**
     * \brief   Hands over the description text the page is still holding, from whichever of the
     *          two detail forms the selected row uses. The box applies its text when it loses the
     *          focus, which a save from the keyboard never causes.
     **/
    void commitPendingEdits(void) override;

//////////////////////////////////////////////////////////////////////////
// Shared entry points
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Opens the machine browse dialog and runs the add-time safety checks on the pick.
     * \return  The picked absolute file path, or an empty string when nothing was picked or the
     *          candidate was refused.
     **/
    static QString browseForMachine(SMIncludeModel& model, QWidget* parent);

    /**
     * \brief   Runs the add-time safety checks on a candidate machine file and reports the
     *          outcome. A cycle, a self-import, too deep a chain and an unsaved host are hard
     *          refusals; an unreadable file is reported and still accepted, because a broken
     *          import must never cost the user their own document.
     * \return  False when the candidate must not be registered.
     **/
    static bool acceptMachine(SMIncludeModel& model, const QString& absoluteFilePath, QWidget* parent);

    //!< The browse filters of the FSM Includes page: C++ sources plus `.fsml` and `.dtml`.
    static QStringList getSupportedExtensions();

    //!< The browse filter for an imported state machine.
    static QStringList getMachineExtensions();

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

    /**
     * \brief   Returns the number of columns in the include list (IETableHelper).
     **/
    int getColumnCount() const override;

    /**
     * \brief   Returns the text of the given list cell (IETableHelper).
     * \param   cell    The index of the cell.
     **/
    QString getCellText(const QModelIndex& cell) const override;

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
private slots:
    void onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void onAddClicked();
    void onInsertClicked();
    void onRemoveClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();

    void onLocationCommitted();

    //!< Commits an inline edit of the Location column into the model (undo command); the derived
    //!< Type/Name columns then refresh from the notifier. Escape is handled by the delegate.
    void onInlineLocationEdited(const QModelIndex& index, const QString& newValue);

    void onBrowseClicked();
    void onDeprecatedToggled(bool checked);
    void onDeprecateHintCommitted();

    void onAliasCommitted();
    void onMachineLocationCommitted();
    void onMachineBrowseClicked();
    void onUpdateVersionClicked();

    //!< Re-derives the Type/Name/Version columns from the current include locations.
    void onUpdateClicked();

    //!< Rebuilds the whole list on any include-kind notifier signal.
    void onNotifierChanged();

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    void buildUi();
    QWidget* buildMachineDetails();
    void setupSignals();

    //!< Rebuilds the whole list from the live model and restores the selection by ID.
    void refreshAll();
    //!< Selects the include by ID; returns false if not found (and selects nothing).
    bool selectInclude(uint32_t id);
    //!< Populates the details panel for the given include and brings the matching page forward.
    void selectedInclude(const IncludeEntry* entry);
    //!< Clears the details panel and disables the row-only tool buttons.
    void showClean();

    void setNodeText(QTreeWidgetItem* node, const IncludeEntry& entry) const;
    void updateMoveButtons(int row, int rowCount);

    //!< A placeholder location for a new entry, unique in the registry.
    QString genName();

    /**
     * \brief   Runs the machine checks on a location the user typed, but only when it names a
     *          file that exists: a path that resolves to nothing cannot close a cycle or be too
     *          deep, and the user may be about to create it. Validation reports it either way.
     * \return  False when the location must not be committed.
     **/
    bool acceptTypedLocation(const QString& location);

    //!< The include ID stored on the currently selected row, or 0 if none.
    uint32_t currentIncludeId() const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    SMIncludeModel&     mModel;
    IncludeListView*    mList;
    QStackedWidget*     mDetailsStack;      //!< Swaps the editor by the selected row's kind.
    IncludeDetailsView* mDetails;           //!< Source and data type editor.
    QWidget*            mMachinePage;       //!< Imported machine editor.
    QLineEdit*          mMachineAlias;
    QLineEdit*          mMachineLocation;
    QPushButton*        mMachineBrowse;
    QPlainTextEdit*     mMachineDescription;
    QLabel*             mMachineStatus;     //!< Resolution and version-drift text for the selection.
    QPushButton*        mMachineUpdate;
    uint32_t            mNameCounter;       //!< Seeds the placeholder location of a new entry.
    TableCell*          mTableCell;         //!< Inline editor delegate for the Location column.
};

#endif  // LUSAN_VIEW_SM_SMINCLUDE_HPP
