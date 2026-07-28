#ifndef LUSAN_VIEW_SM_SMIMPORTS_HPP
#define LUSAN_VIEW_SM_SMIMPORTS_HPP
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
 *  \file        lusan/view/sm/SMImports.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM imported state machines section.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QEvent;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QToolButton;
class QTreeWidget;
class QTreeWidgetItem;
class SMImportModel;

/**
 * \brief   The Imported State Machines section of the Includes page: the `ImportList`
 *          registry with the list on the left and the selected import's details on the right.
 *
 *          Each row shows the alias a state names, the file, the version pinned at import time
 *          and the version currently in that file, so a drift is visible without opening
 *          anything. Re-pinning is the Update button and nothing else -- the editor never
 *          accepts a changed import behind the user's back.
 **/
class SMImports : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Column indexes of the import list.
     **/
    enum eColumn
    {
          ColAlias      = 0 //!< The alias a hosting state names.
        , ColLocation   = 1 //!< The stored location of the imported document.
        , ColPinned     = 2 //!< The version recorded at import (or the last Update).
        , ColActual     = 3 //!< The version currently in the file, or why it cannot be read.
    };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMImports(SMImportModel& model, QWidget* parent = nullptr);
    virtual ~SMImports() = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Selects the import with the given ID, for go-to-declaration from a hosting state.
     **/
    bool revealElement(uint32_t id);

    //!< The import ID on the selected row, or 0.
    uint32_t currentImportId() const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
private slots:
    void onSelectionChanged();
    void onAddClicked();
    void onRemoveClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();
    void onUpdateClicked();
    void onBrowseClicked();
    void onAliasCommitted();
    void onLocationCommitted();

    //!< Rebuilds the list on any Import-kind notifier signal, self-triggered or from undo/redo.
    void onNotifierChanged();

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    void buildUi();
    void setupSignals();

    void refreshAll();
    void fillRow(QTreeWidgetItem* node, uint32_t id);
    void showDetails(uint32_t id);
    void showClean();
    void updateButtons();

    //!< A unique alias for a newly registered file, derived from its base name.
    QString uniqueAlias(const QString& baseName) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    SMImportModel&      mModel;
    QTreeWidget*        mTable;
    QToolButton*        mButtonAdd;
    QToolButton*        mButtonRemove;
    QToolButton*        mButtonMoveUp;
    QToolButton*        mButtonMoveDown;
    QToolButton*        mButtonUpdate;
    QLineEdit*          mAlias;
    QLineEdit*          mLocation;
    QPushButton*        mBrowse;
    QPlainTextEdit*     mDescription;
    QLabel*             mStatus;        //!< Resolution and version-drift text for the selection.
};

#endif  // LUSAN_VIEW_SM_SMIMPORTS_HPP
