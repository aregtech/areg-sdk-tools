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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/sm/SMInclude.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Includes page.
 *
 ************************************************************************/

#include <QScrollArea>
#include <QStringList>
#include <cstdint>

class IncludeEntry;
class QEvent;
class QTreeWidgetItem;
class SMIncludeDetails;
class SMIncludeList;
class SMIncludeModel;

/**
 * \brief   The FSM Includes page: header include locations, the counterpart of the Service
 *          Interface Includes page. The page splits into two equal-width
 *          panels — the multi-column include list (Location, Type, Name, Version) on the
 *          left and the selected-include details editor on the right. Every edit is
 *          committed through SMIncludeModel's undo commands; the page mutates no model state
 *          directly and rebuilds the list from the live model on every Include-kind notifier
 *          signal, self-triggered or from undo/redo alike.
 **/
class SMInclude : public QScrollArea
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMInclude(SMIncludeModel& model, QWidget* parent = nullptr);
    virtual ~SMInclude() = default;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

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
    void onBrowseClicked();
    void onDeprecatedToggled(bool checked);
    void onDeprecateHintCommitted();

    //!< Re-derives the Type/Name/Version columns from the current include locations.
    void onUpdateClicked();

    //!< Rebuilds the whole list on any Include-kind notifier signal.
    void onNotifierChanged();

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    void buildUi();
    void setupSignals();

    //!< Rebuilds the whole list from the live model and restores the selection by ID.
    void refreshAll();
    //!< Selects the include by ID; returns false if not found (and selects nothing).
    bool selectInclude(uint32_t id);
    //!< Populates the details panel for the given include.
    void selectedInclude(const IncludeEntry* entry);
    //!< Clears the details panel and disables the row-only tool buttons.
    void showClean();

    void setNodeText(QTreeWidgetItem* node, const IncludeEntry& entry) const;
    void updateMoveButtons(int row, int rowCount);

    //!< The include ID stored on the currently selected row, or 0 if none.
    uint32_t currentIncludeId() const;
    QString genName();

    static QStringList getSupportedExtensions();

    //!< The include Type derived from a location's file extension: "State Machine" for
    //!< `.fsml`, "Data Type" for `.dtml`, "Source" for anything else.
    static QString includeType(const QString& location);
    //!< The display Name derived from a location: the file name for a source include, the
    //!< base name (no extension) for a state machine or data type include.
    static QString includeName(const QString& location);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    SMIncludeModel&     mModel;
    SMIncludeList*      mList;
    SMIncludeDetails*   mDetails;
    uint32_t            mNameCounter;
};

#endif  // LUSAN_VIEW_SM_SMINCLUDE_HPP
