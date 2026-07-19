#ifndef LUSAN_VIEW_SM_SMMAPPINGEDITOR_HPP
#define LUSAN_VIEW_SM_SMMAPPINGEDITOR_HPP
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
 *  \file        lusan/view/sm/SMMappingEditor.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM parameter-mapping editor grid (spec 9.6).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include "lusan/data/sm/SMOperation.hpp"

#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineModel;
class ElementBase;
enum class eDocElementKind;

class QComboBox;
class QGridLayout;
class QLabel;
class QLineEdit;
class QStackedWidget;

/**
 * \class   SMMappingEditor
 * \brief   The parameter-mapping grid of spec 9.6: one row per declared parameter, with a
 *          source-kind dropdown (`Value`/`Param`/`Attribute`/`Constant`/`Condition`/
 *          `Expression`) and a value cell that follows the kind -- a typed-literal editor for
 *          `Value`, a monospaced cell for `Expression` (the declared type shown read-only), and
 *          otherwise a dropdown of only type-compatible entries. Implicit widening is shown on
 *          the row, defaulted parameters may stay unmapped (their default is shown), and
 *          missing or incompatible mappings are flagged inline without blocking editing.
 *
 *          It edits an operation's argument list (`ActionCall`, `EventSend`, condition-method
 *          calls) through undoable commands; every edit is one undo step and round-trips
 *          byte-exactly. The reference universe and type fit are computed by the headless
 *          `SMMappingSources` / `SMTypeCompat` helpers, so this widget carries no business logic.
 **/
class SMMappingEditor : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Types
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \struct  Param
     * \brief   One declared parameter's signature: its name, declared type, and default (a
     *          defaulted parameter may be left unmapped).
     **/
    struct Param
    {
        QString name;
        QString type;
        QString defaultText;
        bool    hasDefault;
    };

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMMappingEditor(StateMachineModel& model, QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Binds the grid to the argument list \p args of operation \p owner and shows one
     *          row per \p params. \p transitionId is the stimulus scope (0 for entry/exit
     *          operations); \p allowParam gates the `Param` source to the transition scope
     *          (spec 6.8). Passing a null owner clears the grid.
     **/
    void bind( uint32_t transitionId
             , bool allowParam
             , ElementBase* owner
             , QList<SMArgumentEntry>* args
             , const QList<Param>& params);

    //!< Clears the binding and empties the grid.
    void clearBinding();

    inline int rowCount() const;

    //!< Test/host accessors for one data row (0-based, excluding the header).
    QComboBox* sourceCombo(int row) const;
    QWidget*   valueWidget(int row) const;
    QLabel*    typeLabel(int row) const;
    QLabel*    statusLabel(int row) const;

//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private slots:
    void onNotifierChanged(uint32_t id, eDocElementKind kind);

private:
    struct Row
    {
        Param           param;
        QLabel*         name;
        QLabel*         type;
        QComboBox*      source;
        QStackedWidget* value;
        QLineEdit*      literal;
        QComboBox*      ref;
        QLineEdit*      expr;
        QLabel*         status;
    };

    void rebuild();
    void clearGrid();
    void buildRow(int index);
    void refreshRow(int row);

    void onSourceChanged(int row);
    void onLiteralCommitted(int row);
    void onRefActivated(int row);
    void onExpressionCommitted(int row);

    void commit(int row, bool mapped, SMArgumentEntry::eValueSource source, const QString& value, const QString& expression);

    const SMArgumentEntry* argFor(const QString& paramName) const;
    void scheduleRebuild();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&      mModel;
    uint32_t                mTransId;
    bool                    mAllowParam;
    ElementBase*            mOwner;
    QList<SMArgumentEntry>* mArgs;
    QList<Param>            mParams;
    QGridLayout*            mGrid;
    QList<Row>              mRows;
    bool                    mApplying;
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline int SMMappingEditor::rowCount() const
{
    return static_cast<int>(mRows.size());
}

#endif  // LUSAN_VIEW_SM_SMMAPPINGEDITOR_HPP
