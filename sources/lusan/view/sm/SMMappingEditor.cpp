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
 *  \file        lusan/view/sm/SMMappingEditor.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM parameter-mapping editor grid (spec 9.6).
 *
 ************************************************************************/

#include "lusan/view/sm/SMMappingEditor.hpp"

#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMArgumentCommands.hpp"
#include "lusan/model/sm/SMLiteralValidator.hpp"
#include "lusan/model/sm/SMMappingSources.hpp"
#include "lusan/model/sm/SMTypeCompat.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"

#include "lusan/view/sm/NEGuardStyle.hpp"

#include <QComboBox>
#include <QFontDatabase>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPalette>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QTimer>
#include <QUndoStack>

namespace
{
    using eSource = SMArgumentEntry::eValueSource;

    //!< The value stack page indices, one per editor shape.
    enum eValuePage { PageLiteral = 0, PageRef = 1, PageExpr = 2 };

    //!< The owner-language glyph for a source kind (matches the guard catalog's cues).
    QString glyphFor(eSource source)
    {
        switch (source)
        {
        case eSource::Value:        return QStringLiteral("=");
        case eSource::Param:        return QStringLiteral("a");
        case eSource::Attribute:    return QStringLiteral("#");
        case eSource::Constant:     return QStringLiteral("K");
        case eSource::Condition:    return QStringLiteral("h");
        case eSource::Expression:   return QStringLiteral("{}");
        default:                    return QString();
        }
    }

    QString labelFor(eSource source)
    {
        return QString::fromLatin1(SMArgumentEntry::toString(source));
    }

    //!< A short literal-form hint used as the typed-literal placeholder.
    QString literalHint(const QString& type)
    {
        if (type == QStringLiteral("bool"))                                     { return QStringLiteral("true / false"); }
        if (type == QStringLiteral("char"))                                     { return QStringLiteral("one character"); }
        if (type == QStringLiteral("float") || type == QStringLiteral("double")){ return QStringLiteral("e.g. 1.5"); }
        if (type == QStringLiteral("String"))                                   { return QStringLiteral("text"); }
        if (SMTypeCompat::isPrimitive(type))                                    { return QStringLiteral("e.g. 42"); }
        return QString();
    }

    //!< True when the source kind carries a reference value (a dropdown of entries).
    bool isReferenceKind(eSource source)
    {
        return (source == eSource::Param) || (source == eSource::Attribute)
            || (source == eSource::Constant) || (source == eSource::Condition);
    }
}

//////////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////////

SMMappingEditor::SMMappingEditor(StateMachineModel& model, QWidget* parent /*= nullptr*/)
    : QWidget       (parent)
    , mModel        (model)
    , mTransId      (0u)
    , mAllowParam   (false)
    , mOwner        (nullptr)
    , mArgs         (nullptr)
    , mParams       ( )
    , mGrid         (nullptr)
    , mRows         ( )
    , mApplying     (false)
{
    setObjectName(QStringLiteral("smMappingEditor"));

    mGrid = new QGridLayout(this);
    mGrid->setContentsMargins(0, 0, 0, 0);
    mGrid->setHorizontalSpacing(10);
    mGrid->setVerticalSpacing(6);
    mGrid->setColumnStretch(3, 1);

    connect(&mModel.getNotifier(), &DocModelNotifier::elementChanged, this, &SMMappingEditor::onNotifierChanged);
}

//////////////////////////////////////////////////////////////////////////
// Binding
//////////////////////////////////////////////////////////////////////////

void SMMappingEditor::bind( uint32_t transitionId
                          , bool allowParam
                          , ElementBase* owner
                          , QList<SMArgumentEntry>* args
                          , const QList<Param>& params)
{
    mTransId    = transitionId;
    mAllowParam = allowParam;
    mOwner      = owner;
    mArgs       = args;
    mParams     = params;
    rebuild();
}

void SMMappingEditor::clearBinding()
{
    mOwner = nullptr;
    mArgs  = nullptr;
    mParams.clear();
    rebuild();
}

//////////////////////////////////////////////////////////////////////////
// Building
//////////////////////////////////////////////////////////////////////////

void SMMappingEditor::clearGrid()
{
    while (mGrid->count() > 0)
    {
        QLayoutItem* item = mGrid->takeAt(0);
        delete item->widget();
        delete item;
    }

    mRows.clear();
}

void SMMappingEditor::rebuild()
{
    clearGrid();

    if ((mOwner == nullptr) || (mArgs == nullptr) || mParams.isEmpty())
    {
        QLabel* empty = new QLabel(tr("No parameters to map."), this);
        empty->setEnabled(false);
        mGrid->addWidget(empty, 0, 0, 1, 5);
        return;
    }

    const QFont headFont = [this]() { QFont f = font(); f.setBold(true); return f; }();
    const char* const heads[] = { "Parameter", "Type", "Source", "Value" };
    for (int c = 0; c < 4; ++c)
    {
        QLabel* head = new QLabel(tr(heads[c]), this);
        head->setFont(headFont);
        mGrid->addWidget(head, 0, c);
    }

    for (int i = 0; i < mParams.size(); ++i)
    {
        buildRow(i);
    }
}

void SMMappingEditor::buildRow(int index)
{
    const Param& param = mParams.at(index);
    const int gridRow = index + 1;

    Row row;
    row.param = param;

    row.name = new QLabel(param.name, this);
    row.type = new QLabel(param.type, this);

    row.source = new QComboBox(this);
    row.source->setObjectName(QStringLiteral("smMapSource_%1").arg(index));
    row.source->addItem(param.hasDefault ? tr("use default (%1)").arg(param.defaultText) : tr("(not mapped)"), -1);
    const eSource offered[] = { eSource::Value, eSource::Param, eSource::Attribute, eSource::Constant, eSource::Condition, eSource::Expression };
    for (eSource kind : offered)
    {
        if ((kind == eSource::Param) && ((mAllowParam == false) || (SMMappingSources::isKindLegal(mModel.getData(), mTransId, kind) == false)))
        {
            continue;
        }

        row.source->addItem(glyphFor(kind) + QStringLiteral("  ") + labelFor(kind), static_cast<int>(kind));
    }

    row.value   = new QStackedWidget(this);
    row.literal = new QLineEdit(row.value);
    row.literal->setPlaceholderText(literalHint(param.type));
    row.ref     = new QComboBox(row.value);
    row.expr    = new QLineEdit(row.value);
    row.expr->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    row.expr->setPlaceholderText(tr("verbatim C++"));
    row.value->addWidget(row.literal);
    row.value->addWidget(row.ref);
    row.value->addWidget(row.expr);

    row.status = new QLabel(this);

    mGrid->addWidget(row.name,   gridRow, 0);
    mGrid->addWidget(row.type,   gridRow, 1);
    mGrid->addWidget(row.source, gridRow, 2);
    mGrid->addWidget(row.value,  gridRow, 3);
    mGrid->addWidget(row.status, gridRow, 4);

    mRows.append(row);

    // Initial widget state -- programmatic, so signals stay silent.
    {
        const QSignalBlocker b1(row.source);
        const QSignalBlocker b2(row.ref);
        const QSignalBlocker b3(row.literal);
        const QSignalBlocker b4(row.expr);

        const SMArgumentEntry* cur = argFor(param.name);
        if (cur == nullptr)
        {
            row.source->setCurrentIndex(0);
            row.value->setCurrentIndex(PageLiteral);
            row.value->setEnabled(false);
        }
        else
        {
            const eSource src = cur->getSource();
            const int itemIndex = row.source->findData(static_cast<int>(src));
            row.source->setCurrentIndex((itemIndex >= 0) ? itemIndex : 0);
            row.value->setEnabled(true);

            if (src == eSource::Expression)
            {
                row.expr->setText(cur->getExpression());
                row.value->setCurrentIndex(PageExpr);
            }
            else if (isReferenceKind(src))
            {
                for (const SMSourceEntry& entry : SMMappingSources::candidates(mModel.getData(), mTransId, src, param.type))
                {
                    row.ref->addItem(entry.name);
                }
                row.ref->setCurrentText(cur->getValue());
                row.value->setCurrentIndex(PageRef);
            }
            else
            {
                row.literal->setText(cur->getValue());
                row.value->setCurrentIndex(PageLiteral);
            }
        }
    }

    connect(row.source, QOverload<int>::of(&QComboBox::activated), this, [this, index](int) { onSourceChanged(index); });
    connect(row.ref,    QOverload<int>::of(&QComboBox::activated), this, [this, index](int) { onRefActivated(index); });
    connect(row.literal, &QLineEdit::editingFinished, this, [this, index]() { onLiteralCommitted(index); });
    connect(row.expr,    &QLineEdit::editingFinished, this, [this, index]() { onExpressionCommitted(index); });

    refreshRow(index);
}

void SMMappingEditor::refreshRow(int row)
{
    Row& r = mRows[row];
    const Param& param = r.param;
    const SMArgumentEntry* cur = argFor(param.name);

    QString typeText = param.type;
    QString statusText;
    NEGuardStyle::eSeverity severity = NEGuardStyle::eSeverity::Ok;
    bool muted = false;

    if (cur == nullptr)
    {
        // Resolution order: an unmapped parameter falls back to its default, else it is an error.
        if (param.hasDefault)
        {
            statusText = tr("default: %1").arg(param.defaultText);
            muted = true;
        }
        else
        {
            statusText = tr("required");
            severity = NEGuardStyle::eSeverity::Err;
        }
    }
    else if (cur->getSource() == eSource::Value)
    {
        const QString err = SMLiteralValidator::validate(param.type, cur->getValue());
        if (err.isEmpty() == false)
        {
            statusText = err;
            severity = NEGuardStyle::eSeverity::Err;
        }
        else if (cur->getValue().isEmpty())
        {
            statusText = tr("enter a value");
            muted = true;
        }
        else
        {
            statusText = tr("ok");
        }
    }
    else if (cur->getSource() == eSource::Expression)
    {
        statusText = tr("verbatim C++");
        muted = true;
    }
    else if (cur->getValue().isEmpty())
    {
        statusText = tr("select an entry");
        severity = NEGuardStyle::eSeverity::Err;
    }
    else
    {
        const QString srcType = SMMappingSources::referencedType(mModel.getData(), mTransId, cur->getSource(), cur->getValue());
        switch (SMTypeCompat::rank(srcType, param.type))
        {
        case SMTypeCompat::eRank::Exact:
            statusText = tr("ok");
            break;

        case SMTypeCompat::eRank::Converts:
            statusText = tr("ok, converts");
            typeText = srcType + QStringLiteral(" -> ") + param.type;
            break;

        case SMTypeCompat::eRank::Narrows:
            statusText = tr("(!) narrows");
            severity = NEGuardStyle::eSeverity::Warn;
            break;

        case SMTypeCompat::eRank::Mismatch:
            statusText = tr("type mismatch");
            severity = NEGuardStyle::eSeverity::Err;
            break;

        case SMTypeCompat::eRank::Unknown:
        default:
            statusText = tr("ok");
            break;
        }
    }

    r.type->setText(typeText);
    r.status->setText(statusText);
    const QColor color = muted ? palette().color(QPalette::Disabled, QPalette::WindowText)
                               : NEGuardStyle::severityColor(severity);
    r.status->setStyleSheet(QStringLiteral("color: %1;").arg(color.name()));
}

//////////////////////////////////////////////////////////////////////////
// Interaction
//////////////////////////////////////////////////////////////////////////

void SMMappingEditor::onSourceChanged(int row)
{
    if (mApplying || (row >= mRows.size()))
    {
        return;
    }

    const int code = mRows.at(row).source->currentData().toInt();
    if (code < 0)
    {
        commit(row, false, eSource::Value, QString(), QString());
        return;
    }

    const eSource src = static_cast<eSource>(code);
    const Param& param = mRows.at(row).param;
    const SMArgumentEntry* cur = argFor(param.name);

    QString value;
    QString expr;
    if (src == eSource::Expression)
    {
        expr = ((cur != nullptr) && (cur->getSource() == eSource::Expression)) ? cur->getExpression() : QString();
    }
    else if (isReferenceKind(src))
    {
        const QList<SMSourceEntry> cands = SMMappingSources::candidates(mModel.getData(), mTransId, src, param.type);
        const QString keep = ((cur != nullptr) && (cur->getSource() == src)) ? cur->getValue() : QString();
        bool found = false;
        for (const SMSourceEntry& entry : cands)
        {
            if (entry.name == keep) { found = true; break; }
        }

        value = found ? keep : (cands.isEmpty() ? QString() : cands.first().name);
    }
    else
    {
        value = ((cur != nullptr) && (cur->getSource() == eSource::Value)) ? cur->getValue() : QString();
    }

    commit(row, true, src, value, expr);
}

void SMMappingEditor::onLiteralCommitted(int row)
{
    if ((mApplying == false) && (row < mRows.size()))
    {
        commit(row, true, eSource::Value, mRows.at(row).literal->text(), QString());
    }
}

void SMMappingEditor::onRefActivated(int row)
{
    if (mApplying || (row >= mRows.size()))
    {
        return;
    }

    const int code = mRows.at(row).source->currentData().toInt();
    if (code >= 0)
    {
        commit(row, true, static_cast<eSource>(code), mRows.at(row).ref->currentText(), QString());
    }
}

void SMMappingEditor::onExpressionCommitted(int row)
{
    if ((mApplying == false) && (row < mRows.size()))
    {
        commit(row, true, eSource::Expression, QString(), mRows.at(row).expr->text());
    }
}

void SMMappingEditor::commit(int row, bool mapped, SMArgumentEntry::eValueSource source, const QString& value, const QString& expression)
{
    if ((mOwner == nullptr) || (mArgs == nullptr) || (row >= mRows.size()))
    {
        return;
    }

    const QString name = mRows.at(row).param.name;
    const SMArgumentEntry* cur = argFor(name);
    const bool same = mapped
                      ? ((cur != nullptr) && (cur->getSource() == source) && (cur->getValue() == value) && (cur->getExpression() == expression))
                      : (cur == nullptr);
    if (same)
    {
        return;
    }

    mApplying = true;
    SMSetArgumentCommand* command = new SMSetArgumentCommand(mModel.getNotifier(), *mOwner, *mArgs, name, mapped, source, value, expression, tr("Map argument '%1'").arg(name));
    mModel.getUndoStack().push(command);
    mApplying = false;

    scheduleRebuild();
}

//////////////////////////////////////////////////////////////////////////
// Notifications
//////////////////////////////////////////////////////////////////////////

void SMMappingEditor::onNotifierChanged(uint32_t id, eDocElementKind /*kind*/)
{
    if ((mApplying == false) && (mOwner != nullptr) && (id == mOwner->getId()))
    {
        scheduleRebuild();
    }
}

void SMMappingEditor::scheduleRebuild()
{
    // Deferred: rebuilding now would delete the editor widget still emitting the signal this
    // call runs in (use-after-free on unwind) -- the same discipline as SMMappingGrid.
    QTimer::singleShot(0, this, [this]() { rebuild(); });
}

//////////////////////////////////////////////////////////////////////////
// Helpers / accessors
//////////////////////////////////////////////////////////////////////////

const SMArgumentEntry* SMMappingEditor::argFor(const QString& paramName) const
{
    if (mArgs != nullptr)
    {
        for (const SMArgumentEntry& entry : *mArgs)
        {
            if (entry.getName() == paramName)
            {
                return &entry;
            }
        }
    }

    return nullptr;
}

QComboBox* SMMappingEditor::sourceCombo(int row) const
{
    return (row >= 0) && (row < mRows.size()) ? mRows.at(row).source : nullptr;
}

QWidget* SMMappingEditor::valueWidget(int row) const
{
    return (row >= 0) && (row < mRows.size()) ? mRows.at(row).value->currentWidget() : nullptr;
}

QLabel* SMMappingEditor::typeLabel(int row) const
{
    return (row >= 0) && (row < mRows.size()) ? mRows.at(row).type : nullptr;
}

QLabel* SMMappingEditor::statusLabel(int row) const
{
    return (row >= 0) && (row < mRows.size()) ? mRows.at(row).status : nullptr;
}
