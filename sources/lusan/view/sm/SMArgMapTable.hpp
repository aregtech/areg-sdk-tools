#ifndef LUSAN_VIEW_SM_SMARGMAPTABLE_HPP
#define LUSAN_VIEW_SM_SMARGMAPTABLE_HPP
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
 *  \file        lusan/view/sm/SMArgMapTable.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM shared argument-mapping table (spec 6).
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
class IArgSink;
class StateMachineModel;

class QComboBox;
class QGridLayout;
class QLabel;
class QLineEdit;
class QStackedWidget;

/**
 * \class   SMArgMapTable
 * \brief   The ONE argument-mapping widget of spec 6: one row per declared formal parameter
 *          of a call, shown in signature order. It PROJECTS its rows from whatever an
 *          \ref IArgSink reports and commits every edit back through that sink, so it owns
 *          no argument state and knows nothing about the backing store -- the Actions tab
 *          commits into an operation's `SMArgumentEntry` list, the Conditions tab (a later
 *          phase) into the guard AST, through the same rows.
 *
 *          Two row shapes are offered, so one widget can serve both hosts while the two
 *          tabs converge (design D-PARITY):
 *          - \c Compact  -- one editable combo per parameter: pick a stimulus parameter or
 *            an attribute, or type a free literal. This is the Actions tab's established
 *            shape and is what it renders today.
 *          - \c Detailed -- a source-kind dropdown plus a value cell that follows the kind
 *            (typed literal, monospaced verbatim C++, or a dropdown of only type-compatible
 *            entries), with the declared type and an inline status cell. Implicit widening
 *            is shown on the row, a defaulted parameter may stay unmapped, and a missing or
 *            incompatible mapping is flagged without blocking editing.
 *
 *          The reference universe and type fit come from the headless `SMMappingSources` /
 *          `SMTypeCompat` helpers and colors from `NEGuardStyle`, so this widget carries no
 *          business logic and no literal colors.
 *
 *          Width (hazard 12.4): \ref minimumSizeHint is width 0 and every cell is
 *          shrinkable, so however long a signature or a constant name gets, the table can
 *          never widen the dock that hosts it; \c Detailed additionally scrolls
 *          horizontally inside itself. \c Compact deliberately adds no inner scroll area --
 *          its two shrinkable cells have nothing to scroll, and its host already provides
 *          one.
 *
 *          Reentrancy (hazard 12.2): a row edit rewrites the model, which re-projects the
 *          table. Every rebuild that a child's own signal could reach is deferred through
 *          \ref scheduleRebuild, so a row is never destroyed while its signal is unwinding.
 **/
class SMArgMapTable : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Types
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \struct  Param
     * \brief   One declared formal parameter's signature: its name, declared type, and
     *          default (a defaulted parameter may be left unmapped).
     **/
    struct Param
    {
        QString name;
        QString type;
        QString defaultText;
        bool    hasDefault;
        bool    orphan;         //!< A stored mapping whose formal was removed (a red orphan row).
    };

    /**
     * \enum    eRowStyle
     * \brief   The shape of one row's value cell.
     **/
    enum class eRowStyle
    {
          Compact   //!< One editable combo: pick a source entry or type a literal.
        , Detailed  //!< Source-kind combo, a value editor that follows it, and a status cell.
    };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMArgMapTable(StateMachineModel& model, QWidget* parent = nullptr);
    virtual ~SMArgMapTable(void) = default;

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Selects the row shape. Takes effect on the next \ref bind, or immediately
     *          (deferred) when the table is already bound. Defaults to \c Detailed.
     **/
    void setRowStyle(eRowStyle style);

    inline eRowStyle rowStyle(void) const;

    /**
     * \brief   Restricts the Detailed source-kind picker to \p kinds (in the order given), so a
     *          host can offer a narrower universe than the full six. Passing an empty list
     *          restores the default (every source kind). A stored argument whose kind is not in
     *          \p kinds is still shown, so no existing mapping is hidden. Takes effect on the
     *          next \ref bind, or immediately (deferred) when the table is already bound.
     *          Ignored by the Compact shape, which always offers its fixed set.
     **/
    void setAllowedSources(const QList<SMArgumentEntry::eValueSource>& kinds);

    /**
     * \brief   Binds the table to \p sink and shows one row per \p params, in signature
     *          order. \p transitionId is the stimulus scope (0 for an entry/exit operation);
     *          \p allowParam gates the stimulus-parameter source to a transition scope
     *          (spec 6.8). Passing a null \p sink or an empty \p params clears the table.
     **/
    void bind(uint32_t transitionId, bool allowParam, IArgSink* sink, const QList<Param>& params);

    //!< Clears the binding and empties the table.
    void clearBinding(void);

    //!< Re-projects every row from the sink without rebinding. Safe to call from a slot.
    void refresh(void);

    inline int rowCount(void) const;

    //!< Test/host accessors for one data row (0-based, excluding any header).
    QLabel*    nameLabel(int row) const;
    QComboBox* sourceCombo(int row) const;
    QWidget*   valueWidget(int row) const;
    QLabel*    typeLabel(int row) const;
    QLabel*    statusLabel(int row) const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    //!< Width 0, so no signature length can ever widen the hosting dock (hazard 12.4).
    virtual QSize minimumSizeHint(void) const override;

protected:
    /**
     * \brief   Gives a value editor the two gestures of spec 6: a click in the field opens
     *          its source list (rather than only the tiny drop arrow), and Esc reverts the
     *          field to the value it held when editing began WITHOUT committing.
     **/
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \struct  Row
     * \brief   One projected row. Only the widgets of the active \ref eRowStyle are built;
     *          the rest stay null.
     **/
    struct Row
    {
        Param           param;
        QLabel*         name;       //!< `name (Type)` in Compact, the bare name in Detailed.
        QLabel*         type;       //!< Detailed only.
        QComboBox*      source;     //!< Detailed only -- the source-kind dropdown.
        QStackedWidget* value;      //!< Detailed only -- the kind-following value cell.
        QLineEdit*      literal;    //!< Detailed only.
        QComboBox*      ref;        //!< Detailed only.
        QLineEdit*      expr;       //!< Detailed only.
        QComboBox*      compact;    //!< Compact only -- the single editable value combo.
        QLabel*         status;     //!< Detailed only.
    };

    void rebuild(void);
    void clearRows(void);
    void buildHost(void);

    void buildCompactRow(int index);
    void buildDetailedRow(int index);
    void buildOrphanRow(int index);
    void refreshRow(int row);

    // Detailed interaction.
    void onSourceChanged(int row);
    void onLiteralCommitted(int row);
    void onRefActivated(int row);
    void onExpressionCommitted(int row);

    // Compact interaction.
    void onCompactCommitted(int row);

    //!< Prepares a value editor for click-to-open and Esc-revert.
    void watchEditor(QLineEdit* edit);

    /**
     * \brief   Routes one row's edit to the sink -- \c setArg when \p mapped, \c clearArg
     *          otherwise. The sink drops a commit that changes nothing, so one real edit is
     *          exactly one undo step.
     **/
    void commit(int row, bool mapped, SMArgumentEntry::eValueSource source, const QString& value, const QString& expression);

    //!< The current mapping of row \p row, or nullptr when the slot is unmapped.
    const SMArgumentEntry* argFor(int row) const;

    /**
     * \brief   The source kind that \p text names in this scope -- Param when it names a
     *          stimulus parameter, Attribute when it names an attribute, else Value (an
     *          unmatched entry is kept as a free literal). Resolved against
     *          `SMMappingSources`, never against an inlined list.
     **/
    SMArgumentEntry::eValueSource resolveCompactSource(const QString& text) const;

    //!< Defers a rebuild to the next event-loop turn (hazard 12.2); never rebuilds inline.
    void scheduleRebuild(void);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mModel;
    eRowStyle           mStyle;
    uint32_t            mTransId;
    bool                mAllowParam;
    IArgSink*           mSink;
    QList<Param>        mParams;
    QList<SMArgumentEntry::eValueSource> mAllowedSources;    //!< Detailed source filter; empty = all.
    QWidget*            mHost;          //!< Rebuilt per style; owns the row widgets.
    QGridLayout*        mGrid;          //!< Detailed only -- the row grid inside \ref mHost.
    QList<Row>          mRows;
    bool                mApplying;      //!< True while the table drives the model or itself.
    bool                mRebuildQueued;
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline SMArgMapTable::eRowStyle SMArgMapTable::rowStyle(void) const
{
    return mStyle;
}

inline int SMArgMapTable::rowCount(void) const
{
    return static_cast<int>(mRows.size());
}

#endif  // LUSAN_VIEW_SM_SMARGMAPTABLE_HPP
