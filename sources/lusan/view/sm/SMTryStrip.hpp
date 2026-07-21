#ifndef LUSAN_VIEW_SM_SMTRYSTRIP_HPP
#define LUSAN_VIEW_SM_SMTRYSTRIP_HPP
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
 *  \file        lusan/view/sm/SMTryStrip.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard Try-it strip (v7 B9 / S6).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include "lusan/model/sm/SMGuardEval.hpp"

#include <QHash>
#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QLabel;
class QToolButton;
class QVBoxLayout;
class StateMachineModel;
enum class eDocElementKind;

/**
 * \class   SMTryStrip
 * \brief   The Try-it strip (S6): a collapsible what-if panel under the structure lens.
 *          One value field per stimulus parameter and referenced attribute (literal
 *          validator by declared type; enumerations get a member combo), one stub row per
 *          non-evaluable call/island/raw fragment (they NEVER execute in the tool --
 *          always stubbed, labeled `stub`), and a result line. While open it publishes
 *          per-clause truths so the lens tints its pills green/red, and when sibling
 *          transitions react to the same stimulus it names the priority winner.
 **/
class SMTryStrip : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMTryStrip(StateMachineModel& model, QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    //!< Points the strip at a transition (0 clears it); collapses and resets the inputs.
    void setTransition(uint32_t transitionId);

    //!< True while the strip is expanded (the lens tints only then).
    bool isOpen() const;

    //!< Expands / collapses programmatically (tests).
    void setOpen(bool open);

signals:
    /**
     * \brief   The per-clause truths changed: lens pathName (`root`, `0_1`, ...) -> truth
     *          (1 true, 0 false; Unknown clauses are omitted). Empty when the strip closes.
     **/
    void truthTintsChanged(const QHash<QString, int>& tints);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:
    void onElementAdded(uint32_t id, eDocElementKind kind);
    void onElementChanged(uint32_t id, eDocElementKind kind);
    void onElementRemoved(uint32_t id, eDocElementKind kind);
    void onDocumentReloaded();

private:
    void scheduleRebuild();
    void rebuild();
    void recompute();

    //!< One `name = [value]` field row widget for a Param/Attr reference.
    QWidget* makeValueField(SMGuardNode::eKind kind, uint32_t symbolId, const QString& name, const QString& typeName);

    //!< One stub row for a non-evaluable node.
    void addStubRow(QVBoxLayout* into, const SMGuardEval::StubSite& site);

    //!< The default literal of a value field: declared value, else a type default.
    QString defaultValue(SMGuardNode::eKind kind, uint32_t symbolId, const QString& typeName) const;

    //!< The sibling label shown in the winner line (`-> Target` / `(internal)`).
    QString siblingLabel(uint32_t transitionId) const;

    //!< The winner sentence appended to the result line when siblings exist.
    QString winnerText(SMGuardEval::eTruth selfTruth, const SMGuardEval::Inputs& inputs) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&          mModel;         //!< The document facade.
    uint32_t                    mTransitionId;  //!< The shown transition (0 = none).
    QToolButton*                mToggle;        //!< The `> Try it` collapse toggle.
    QWidget*                    mContent;       //!< The value/stub/result host.
    QLabel*                     mResult;        //!< The result line.
    QLabel*                     mNote;          //!< Shown when there is nothing to evaluate.
    QHash<uint32_t, QString>    mValues;        //!< Entered values (symbol ID -> literal).
    QHash<QString, bool>        mStubs;         //!< Stub combos (path key -> value).
    bool                        mRebuildPending;//!< Coalesces deferred rebuilds.
};

#endif  // LUSAN_VIEW_SM_SMTRYSTRIP_HPP
