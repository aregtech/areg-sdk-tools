#ifndef LUSAN_VIEW_COMMON_INCLUDELISTVIEW_HPP
#define LUSAN_VIEW_COMMON_INCLUDELISTVIEW_HPP
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
 *  \file        lusan/view/common/IncludeListView.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared "include list" panel used by the Service
 *               Interface, State Machine and (future) Data Type Includes pages.
 *
 ************************************************************************/

#include "lusan/data/common/IncludeEntry.hpp"

#include <QIcon>
#include <QKeySequence>
#include <QList>
#include <QString>
#include <QWidget>

#include <cstdint>

class QModelIndex;
class QToolButton;
class QTreeWidget;
class QTreeWidgetItem;

/**
 * \brief   The one difference between the Includes pages of the two editors: whether the host
 *          document may include a document of its own kind, and if so under which extension,
 *          label and group heading. Everything besides a C++ header and a `.dtml` data type
 *          document is decided here.
 *
 *          State Machine    : `"fsml"`, `"State Machine"`, `"State Machines"` -- a machine may
 *                             import another machine.
 *          Service Interface: all empty -- an interface is an API contract and includes no
 *                             other contract, so the page shows no document group at all.
 *
 *          \a takesDataTypes is the second answer: a data type document is itself a leaf and
 *          includes C++ headers only, so its page shows no Data Types group either.
 **/
struct IncludeTypeConfig
{
    QString docExtension;   //!< The document file extension (no dot), e.g. "fsml"; empty for none.
    QString docTypeLabel;   //!< The Type column label for that extension, e.g. "State Machine".
    QString groupDocLabel;  //!< The heading of the group that collects those documents.
    QIcon   docIcon;        //!< The mark of that document kind, on its heading and its rows.
    bool    takesDataTypes{ true }; //!< False where the host includes no `.dtml` at all.

    //!< True when the host document may include a document of its own kind.
    inline bool hasDocuments() const { return (docExtension.isEmpty() == false); }

    //!< True when the host document may include a data type document.
    inline bool hasDataTypes() const { return takesDataTypes; }
};

/**
 * \brief   The shared include list panel: an "Include Files:" group holding the
 *          add/insert/delete + move up/down + update toolbar above a 4-column tree
 *          (Location / Type / Name / Version).
 *
 *          Rows sit under permanent headings -- source files, data types, and documents of the
 *          host's own kind where the host has them -- so the reader does not have to know which
 *          list a file belongs to before looking for it. The headings stay visible when empty;
 *          that is what makes them read as places to put something.
 *
 *          The view is controller-agnostic: it builds the widgets and exposes ctrl*() accessors
 *          plus the config-driven classification helpers, so both editors derive the group, the
 *          Type and the Name of a location identically. Row population and selection belong to
 *          the page controller.
 **/
class IncludeListView : public QWidget
{
    Q_OBJECT

public:
    /**
     * \brief   Column indexes of the include list.
     **/
    enum eColumn
    {
          ColLocation   = 0 //!< The include file location (editable inline).
        , ColType       = 1 //!< The derived include type (read-only).
        , ColName       = 2 //!< The include name: the alias for a document, derived otherwise.
        , ColVersion    = 3 //!< The pinned version of a document include; empty otherwise.
    };

    explicit IncludeListView(const IncludeTypeConfig& config, QWidget* parent = nullptr);

    QTreeWidget* ctrlTableList() const;

    QToolButton* ctrlButtonAdd() const;
    QToolButton* ctrlButtonInsert() const;
    QToolButton* ctrlButtonRemove() const;
    QToolButton* ctrlButtonMoveUp() const;
    QToolButton* ctrlButtonMoveDown() const;
    QToolButton* ctrlButtonUpdate() const;

    //!< The permanent heading row that collects \p kind. Never null: a kind with no heading of
    //!< its own falls back to the source heading, which is where an unclassified row belongs.
    QTreeWidgetItem* ctrlGroup(eIncludeKind kind) const;

    //!< The configuration the page was built with.
    inline const IncludeTypeConfig& getConfig() const;

    //!< The kind a location belongs to, using the host document's own extension.
    eIncludeKind kindForLocation(const QString& location) const;

    //!< The mark of one kind, on its heading and on every row under it. One place, so a row and
    //!< its heading can never disagree about what they are.
    QIcon iconForKind(eIncludeKind kind) const;

    /**
     * \brief   Returns the Type column text for a location: the configured document label for
     *          the document extension, "Data Type" for `.dtml`, "Source" for anything else.
     **/
    QString typeForLocation(const QString& location) const;

    /**
     * \brief   Returns the Name column text for a location: the base name (no extension) for a
     *          document or data type include, the file name for a source include.
     **/
    QString nameForLocation(const QString& location) const;

    //!< Refreshes the per-group entry counts shown in the headings. The document count is
    //!< ignored by a page that has no document group.
    void updateGroupCounts(int sourceCount, int dataTypeCount, int documentCount);

    //!< The same, counted from the rows themselves. For callers that move a single row and have
    //!< no tally of their own.
    void refreshGroupCounts();

    //!< Removes every entry row, leaving the three headings in place.
    void clearRows();

    /**
     * \brief   Creates (or moves) the row of \p id so that it sits last in the group of \p kind.
     *          The row carries \p id in the Location column's UserRole -- the one place an ID is
     *          stored, so every lookup agrees on where to read it.
     **/
    QTreeWidgetItem* placeRow(QTreeWidgetItem* existing, eIncludeKind kind, uint32_t id);

    //!< The entry row carrying \p id, or nullptr. Heading rows are never returned.
    QTreeWidgetItem* findRow(uint32_t id) const;

    //!< The element ID stored on an entry row; 0 for a heading row or nullptr.
    static uint32_t rowId(const QTreeWidgetItem* item);

    //!< True when \p item is one of the three permanent headings.
    bool isGroup(const QTreeWidgetItem* item) const;

    /**
     * \brief   The item a model index of this tree denotes. The tree is two levels deep, so this
     *          replaces the flat topLevelItem(row) lookup the delegate used to do.
     **/
    QTreeWidgetItem* itemAt(const QModelIndex& index) const;

protected:
    void changeEvent(QEvent* event) override;

private:
    void buildUi();
    void decorateGroup(QTreeWidgetItem* group);

    //!< The headings this page actually has, in display order.
    QList<QTreeWidgetItem*> groups() const;

private:
    const IncludeTypeConfig mConfig;
    QTreeWidget*    mTable;
    QTreeWidgetItem* mGroupSource;
    QTreeWidgetItem* mGroupDataType;
    QTreeWidgetItem* mGroupDocument;   //!< Null when the host includes no document of its own kind.
    QToolButton*    mButtonAdd;
    QToolButton*    mButtonInsert;
    QToolButton*    mButtonRemove;
    QToolButton*    mButtonMoveUp;
    QToolButton*    mButtonMoveDown;
    QToolButton*    mButtonUpdate;
};

//////////////////////////////////////////////////////////////////////////
// IncludeListView inline methods
//////////////////////////////////////////////////////////////////////////

inline const IncludeTypeConfig& IncludeListView::getConfig() const
{
    return mConfig;
}

#endif  // LUSAN_VIEW_COMMON_INCLUDELISTVIEW_HPP
