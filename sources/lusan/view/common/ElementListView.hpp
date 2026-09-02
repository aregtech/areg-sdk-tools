#ifndef LUSAN_VIEW_COMMON_ELEMENTLISTVIEW_HPP
#define LUSAN_VIEW_COMMON_ELEMENTLISTVIEW_HPP
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
 *  \file        lusan/view/common/ElementListView.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \brief       Lusan application, the list panel every document editor page is built on.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include <QString>
#include <QStringList>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QBoxLayout;
class QGroupBox;
class QMenu;
class QToolButton;
class QTreeWidget;

/**
 * \brief   What one document editor page calls the list it shows.
 **/
struct ElementListConfig
{
    QString     groupTitle; //!< The caption of the group the list sits in.
    QStringList headers;    //!< The column captions; the first column takes the horizontal slack.
    QString     entryName;  //!< What one top level row is called, in the tool tips and the menu.
    QString     childName;  //!< What a child row is called; empty when the list has no child rows.
    bool        flatList;   //!< The list holds no child rows and draws no expander.
};

/**
 * \class   ElementListView
 * \brief   The list panel shared by every document editor page: a toolbar over a tree, inside a
 *          titled group. The toolbar carries add, delete and insert for a top level row, the same
 *          three for a child row where the page has them, and move up and down.
 *
 *          The same commands are on the row context menu, which is built from the buttons
 *          themselves, so a menu entry always carries the icon, the shortcut and the enabled
 *          state of the button beside it.
 *
 *          The panel is controller-agnostic: it builds the widgets and hands them out through the
 *          ctrl*() accessors. The page controller owns the rows, the selection and the button
 *          states.
 **/
class ElementListView : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Builds the panel.
     * \param   config  What this page calls its list.
     * \param   parent  The parent widget.
     **/
    explicit ElementListView(const ElementListConfig& config, QWidget* parent = nullptr);

    virtual ~ElementListView(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    QTreeWidget* ctrlTableList(void) const;

    QToolButton* ctrlButtonAdd(void) const;
    QToolButton* ctrlButtonRemove(void) const;
    QToolButton* ctrlButtonInsert(void) const;

    //!< The child row buttons, or nullptr when the list has no child rows.
    QToolButton* ctrlButtonAddChild(void) const;
    QToolButton* ctrlButtonRemoveChild(void) const;
    QToolButton* ctrlButtonInsertChild(void) const;

    QToolButton* ctrlButtonMoveUp(void) const;
    QToolButton* ctrlButtonMoveDown(void) const;

    //!< True when the list offers the child row buttons.
    inline bool hasChildRows(void) const;

    /**
     * \brief   Fills the given menu with the entries the row context menu offers for the current
     *          selection. The panel calls this itself on a right click.
     * \param   menu    The menu to fill.
     **/
    void buildContextMenu(QMenu& menu);

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:
    /**
     * \brief   The context menu asked for the selected row to be renamed. The page puts the caret
     *          in the field that names the row.
     **/
    void signalRenameRequested(void);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Fills the row context menu. A subclass calls the base first, then appends the
     *          entries only it has.
     * \param   menu    The menu about to be shown.
     **/
    virtual void fillContextMenu(QMenu& menu);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
protected:
    //!< The group the list sits in, and the toolbar the buttons sit on.
    inline QGroupBox* ctrlGroupBox(void) const;
    inline QWidget* ctrlToolbar(void) const;

    //!< Appends a vertical separator to the toolbar, left of the trailing stretch.
    void addToolbarSeparator(void);

    //!< Appends a button of the subclass to the toolbar, left of the trailing stretch.
    void addToolbarButton(QToolButton* button);

    /**
     * \brief   Appends one menu entry mirroring a toolbar button: the icon, the shortcut and the
     *          enabled state come from the button, and the entry clicks it.
     * \param   menu    The menu to append to.
     * \param   button  The button the entry stands for; a null button appends nothing.
     * \param   text    The entry text.
     **/
    static void addButtonEntry(QMenu& menu, QToolButton* button, const QString& text);

private slots:
    void onContextMenuRequested(const QPoint& pos);

private:
    void buildUi(void);

    //!< Puts the cell texts of the current row on the clipboard, separated by tabs.
    void copyCurrentRow(void) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    const ElementListConfig mListConfig;         //!< What this page calls its list.
    QGroupBox*              mGroup;              //!< The titled group holding the toolbar and the tree.
    QWidget*                mToolbar;            //!< The toolbar the buttons sit on.
    QBoxLayout*             mToolbarLayout;      //!< Its layout, ending with a stretch.
    QTreeWidget*            mTable;              //!< The list itself.
    QToolButton*            mButtonAdd;          //!< Adds a top level row at the end.
    QToolButton*            mButtonRemove;       //!< Deletes the selected top level row.
    QToolButton*            mButtonInsert;       //!< Inserts a top level row above the selected one.
    QToolButton*            mButtonAddChild;     //!< Adds a child row at the end.
    QToolButton*            mButtonRemoveChild;  //!< Deletes the selected child row.
    QToolButton*            mButtonInsertChild;  //!< Inserts a child row above the selected one.
    QToolButton*            mButtonMoveUp;       //!< Moves the selected row one position up.
    QToolButton*            mButtonMoveDown;     //!< Moves the selected row one position down.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    ElementListView(const ElementListView& /*src*/) = delete;
    ElementListView& operator = (const ElementListView& /*src*/) = delete;
};

//////////////////////////////////////////////////////////////////////////
// ElementListView inline methods
//////////////////////////////////////////////////////////////////////////

inline bool ElementListView::hasChildRows(void) const
{
    return (mButtonAddChild != nullptr);
}

inline QGroupBox* ElementListView::ctrlGroupBox(void) const
{
    return mGroup;
}

inline QWidget* ElementListView::ctrlToolbar(void) const
{
    return mToolbar;
}

#endif  // LUSAN_VIEW_COMMON_ELEMENTLISTVIEW_HPP
