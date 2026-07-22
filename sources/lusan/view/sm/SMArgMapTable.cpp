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
 *  \file        lusan/view/sm/SMArgMapTable.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM shared argument-mapping table.
 *
 ************************************************************************/

#include "lusan/view/sm/SMArgMapTable.hpp"

#include "lusan/model/sm/SMLiteralValidator.hpp"
#include "lusan/model/sm/SMMappingSources.hpp"
#include "lusan/model/sm/SMTypeCompat.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"

#include "lusan/view/sm/IArgSink.hpp"
#include "lusan/view/sm/NEGuardStyle.hpp"

#include <QAbstractItemView>
#include <QComboBox>
#include <QCompleter>
#include <QEvent>
#include <QFontDatabase>
#include <QFormLayout>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPalette>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

namespace
{
    using eSource = SMArgumentEntry::eValueSource;

    //!< The value stack page indices, one per editor shape (Detailed rows).
    enum eValuePage { PageLiteral = 0, PageRef = 1, PageExpr = 2 };

    //!< Remembers a field's pre-edit text so Esc can put it back without committing.
    const char* const PropPreEdit { "smArgPreEdit" };

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

    /**
     * \brief   Makes \p combo shrinkable, so however long its entries are it can never push
     *          the hosting dock wider. It still expands to fill the space the
     *          layout offers -- only its minimum is pinned low.
     **/
    void makeShrinkable(QComboBox* combo)
    {
        combo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
        combo->setMinimumContentsLength(0);
    }
}

//////////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////////

SMArgMapTable::SMArgMapTable(StateMachineModel& model, QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mModel            (model)
    , mStyle            (eRowStyle::Detailed)
    , mTransId          (0u)
    , mAllowParam       (false)
    , mSink             (nullptr)
    , mParams           ( )
    , mAllowedSources   ( )
    , mHost             (nullptr)
    , mGrid             (nullptr)
    , mRows             ( )
    , mApplying         (false)
    , mRebuildQueued    (false)
{
    setObjectName(QStringLiteral("smArgMapTable"));

    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    buildHost();
}

//////////////////////////////////////////////////////////////////////////
// Binding
//////////////////////////////////////////////////////////////////////////

void SMArgMapTable::setRowStyle(eRowStyle style)
{
    if (mStyle != style)
    {
        mStyle = style;
        rebuild();
    }
}

void SMArgMapTable::setAllowedSources(const QList<SMArgumentEntry::eValueSource>& kinds)
{
    if (mAllowedSources != kinds)
    {
        mAllowedSources = kinds;
        if (mSink != nullptr)
        {
            scheduleRebuild();
        }
    }
}

void SMArgMapTable::bind(uint32_t transitionId, bool allowParam, IArgSink* sink, const QList<Param>& params)
{
    mTransId    = transitionId;
    mAllowParam = allowParam;
    mSink       = sink;
    mParams     = params;
    rebuild();
}

void SMArgMapTable::clearBinding(void)
{
    mSink = nullptr;
    mParams.clear();
    rebuild();
}

void SMArgMapTable::refresh(void)
{
    scheduleRebuild();
}

//////////////////////////////////////////////////////////////////////////
// Building
//////////////////////////////////////////////////////////////////////////

void SMArgMapTable::buildHost(void)
{
    QVBoxLayout* outer = static_cast<QVBoxLayout*>(layout());

    mHost = new QWidget(this);
    mGrid = nullptr;

    if (mStyle == eRowStyle::Compact)
    {
        // The Actions shape: a plain form host, indented under its group's picker. No inner
        // scroll area -- both cells shrink, and the operations editor already scrolls.
        QFormLayout* form = new QFormLayout(mHost);
        form->setContentsMargins(12, 2, 0, 0);
        outer->addWidget(mHost);
    }
    else
    {
        mGrid = new QGridLayout(mHost);
        mGrid->setContentsMargins(0, 0, 0, 0);
        mGrid->setHorizontalSpacing(10);
        mGrid->setVerticalSpacing(6);
        mGrid->setColumnStretch(3, 1);

        // The rich grid cannot shrink past its labels, so it scrolls inside itself rather
        // than widening the dock.
        QScrollArea* scroll = new QScrollArea(this);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setWidgetResizable(true);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scroll->setWidget(mHost);
        outer->addWidget(scroll);
    }
}

void SMArgMapTable::clearRows(void)
{
    mRows.clear();

    // The host is only ever torn down from bind() (driven by the host widget, never by one
    // of these rows' own signals) or from the deferred scheduleRebuild -- so no row is
    // destroyed while its signal is still unwinding.
    QLayoutItem* item = nullptr;
    while ((item = layout()->takeAt(0)) != nullptr)
    {
        delete item->widget();
        delete item;
    }

    mHost = nullptr;
    mGrid = nullptr;
}

void SMArgMapTable::rebuild(void)
{
    const bool wasApplying = mApplying;
    mApplying = true;

    clearRows();
    buildHost();

    if ((mSink == nullptr) || mParams.isEmpty())
    {
        // The Actions form simply shows nothing when there is nothing to map; the rich grid
        // says so explicitly.
        if (mStyle == eRowStyle::Detailed)
        {
            QLabel* empty = new QLabel(tr("No parameters to map."), mHost);
            empty->setEnabled(false);
            mGrid->addWidget(empty, 0, 0, 1, 5);
        }

        mApplying = wasApplying;
        return;
    }

    if (mStyle == eRowStyle::Detailed)
    {
        const QFont headFont = [this]() { QFont f = font(); f.setBold(true); return f; }();
        const char* const heads[] = { "Parameter", "Type", "Source", "Value" };
        for (int c = 0; c < 4; ++c)
        {
            QLabel* head = new QLabel(tr(heads[c]), mHost);
            head->setFont(headFont);
            mGrid->addWidget(head, 0, c);
        }
    }

    for (int i = 0; i < mParams.size(); ++i)
    {
        if (mStyle == eRowStyle::Compact)
        {
            buildCompactRow(i);
        }
        else
        {
            buildDetailedRow(i);
        }
    }

    mApplying = wasApplying;
}

void SMArgMapTable::buildCompactRow(int index)
{
    const Param& param = mParams.at(index);

    Row row {};
    row.param = param;

    row.name = new QLabel(param.name + QStringLiteral(" (") + param.type + QLatin1Char(')'), mHost);

    row.compact = new QComboBox(mHost);
    row.compact->setObjectName(QStringLiteral("smArgValue_%1").arg(index));
    row.compact->setEditable(true);
    row.compact->setInsertPolicy(QComboBox::NoInsert);
    makeShrinkable(row.compact);

    // The first entry is ALWAYS empty: picking it is how the developer un-maps a formal without
    // having to select-all-and-delete the text.
    row.compact->addItem(QString());

    // The offered entries, in resolution order: stimulus parameters first (transition scope
    // only), then attributes. Constants are deliberately NOT listed -- the Actions tab has never
    // offered them in a Compact row and the two tabs must stay in parity (SMArgMapTableTests).
    // Anything typed that matches neither stays a free literal -- the one "user value".
    const eSource offered[] = { eSource::Param, eSource::Attribute };
    for (eSource kind : offered)
    {
        if ((kind == eSource::Param)
            && ((mAllowParam == false) || (SMMappingSources::isKindLegal(mModel.getData(), mTransId, kind) == false)))
        {
            continue;
        }

        // An empty target type ranks every entry Unknown rather than Mismatch, so the list
        // stays unfiltered -- a Compact row offers every source, as the Actions tab always has.
        for (const SMSourceEntry& entry : SMMappingSources::candidates(mModel.getData(), mTransId, kind, QString()))
        {
            row.compact->addItem(entry.name);
        }
    }

    // Watch the drop-down container too: it grabs the mouse while open, so a click aimed back at
    // the combo (the second half of a double click) is only visible from there.
    if (row.compact->view() != nullptr)
    {
        row.compact->view()->window()->installEventFilter(this);
    }

    if (row.compact->completer() != nullptr)
    {
        row.compact->completer()->setCompletionMode(QCompleter::PopupCompletion);
        row.compact->completer()->setCaseSensitivity(Qt::CaseInsensitive);
    }

    {
        const QSignalBlocker block(row.compact);
        const SMArgumentEntry* cur = (mSink != nullptr) ? mSink->argFor(param.name) : nullptr;
        const QString value = (cur != nullptr) ? cur->getValue() : QString();
        // Select the mapped entry when it IS one of the offered sources, so re-opening the popup
        // lands on -- and highlights -- what is currently mapped instead of the first row.
        const int found = value.isEmpty() ? 0 : row.compact->findText(value);
        row.compact->setCurrentIndex((found >= 0) ? found : 0);
        row.compact->setEditText(value);
        row.committed = value;
    }

    row.compact->lineEdit()->setPlaceholderText(param.hasDefault
        ? tr("default: %1").arg(param.defaultText)
        : tr("value, or pick a source"));

    // A formal that still has no binding (nor a typed literal, nor a default) gets an amber wash on
    // its name cell. A parameter freshly added to a referenced condition lands here, so it reads as
    // "still needs mapping" rather than as a silently blank row the developer might miss.
    if (row.committed.isEmpty() && (param.hasDefault == false))
    {
        row.name->setStyleSheet(QStringLiteral("background-color: %1;").arg(NEGuardStyle::unmappedTint().name()));
    }

    static_cast<QFormLayout*>(mHost->layout())->addRow(row.name, row.compact);
    mRows.append(row);

    // `activated` is a real user pick. `editingFinished`, however, ALSO fires when the popup takes
    // focus away from the line edit -- committing there re-projected the table and destroyed the
    // combo mid-gesture, so the list appeared to open and snap shut and the caret was lost
    // . onCompactCommitted now ignores a no-op edit.
    connect(row.compact, QOverload<int>::of(&QComboBox::activated), this, [this, index](int) { onCompactCommitted(index); });
    connect(row.compact->lineEdit(), &QLineEdit::editingFinished, this, [this, index]() { onCompactCommitted(index); });
    watchEditor(row.compact->lineEdit());
}

void SMArgMapTable::buildDetailedRow(int index)
{
    if (mParams.at(index).orphan)
    {
        buildOrphanRow(index);
        return;
    }

    const Param& param = mParams.at(index);
    const int gridRow = index + 1;

    Row row {};
    row.param = param;

    row.name = new QLabel(param.name, mHost);
    row.type = new QLabel(param.type, mHost);

    row.source = new QComboBox(mHost);
    row.source->setObjectName(QStringLiteral("smArgSource_%1").arg(index));
    makeShrinkable(row.source);
    row.source->addItem(param.hasDefault ? tr("use default (%1)").arg(param.defaultText) : tr("(not mapped)"), -1);
    // The kind a stored mapping already uses is always offered, so a host source filter can
    // never hide an existing value (it would otherwise fall back to "(not mapped)").
    const SMArgumentEntry* curKind = (mSink != nullptr) ? mSink->argFor(param.name) : nullptr;
    const eSource offered[] = { eSource::Value, eSource::Param, eSource::Attribute, eSource::Constant, eSource::Condition, eSource::Expression };
    for (eSource kind : offered)
    {
        if ((kind == eSource::Param) && ((mAllowParam == false) || (SMMappingSources::isKindLegal(mModel.getData(), mTransId, kind) == false)))
        {
            continue;
        }

        // A host may narrow the picker (the Actions tab offers param/attribute/constant/literal
        // only); an empty filter keeps every kind.
        if ((mAllowedSources.isEmpty() == false) && (mAllowedSources.contains(kind) == false)
            && ((curKind == nullptr) || (curKind->getSource() != kind)))
        {
            continue;
        }

        row.source->addItem(glyphFor(kind) + QStringLiteral("  ") + labelFor(kind), static_cast<int>(kind));
    }

    row.value   = new QStackedWidget(mHost);
    row.literal = new QLineEdit(row.value);
    row.literal->setPlaceholderText(literalHint(param.type));
    row.ref     = new QComboBox(row.value);
    makeShrinkable(row.ref);
    row.expr    = new QLineEdit(row.value);
    row.expr->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    row.expr->setPlaceholderText(tr("verbatim C++"));
    row.value->addWidget(row.literal);
    row.value->addWidget(row.ref);
    row.value->addWidget(row.expr);

    row.status = new QLabel(mHost);

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

        const SMArgumentEntry* cur = (mSink != nullptr) ? mSink->argFor(param.name) : nullptr;
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

    watchEditor(row.literal);
    watchEditor(row.expr);

    refreshRow(index);
}

void SMArgMapTable::buildOrphanRow(int index)
{
    // Orphan case (b): the callee still exists but this parameter was removed on the Methods
    // page, so its stored value can no longer be re-typed as a live formal. The mapping is never
    // silently discarded: the value is shown in a red row that keeps it until the
    // developer removes it through the quick-fix. The same red row appears in both tabs.
    const Param& param = mParams.at(index);
    const int gridRow = index + 1;

    Row row {};
    row.param = param;

    const QColor err = NEGuardStyle::severityColor(NEGuardStyle::eSeverity::Err);
    const QString redStyle = QStringLiteral("color: %1;").arg(err.name());

    row.name = new QLabel(param.name, mHost);
    row.name->setStyleSheet(redStyle);

    row.type = new QLabel(tr("(removed)"), mHost);
    row.type->setStyleSheet(redStyle);

    const SMArgumentEntry* cur = (mSink != nullptr) ? mSink->argFor(param.name) : nullptr;
    QLabel* value = new QLabel((cur != nullptr) ? cur->getValue() : QString(), mHost);

    row.status = new QLabel(tr("orphaned"), mHost);
    row.status->setStyleSheet(redStyle);

    QToolButton* remove = new QToolButton(mHost);
    remove->setText(tr("x"));
    remove->setToolTip(tr("The mapped parameter no longer exists; remove the stale binding"));

    mGrid->addWidget(row.name,   gridRow, 0);
    mGrid->addWidget(row.type,   gridRow, 1);
    mGrid->addWidget(row.status, gridRow, 2);
    mGrid->addWidget(value,      gridRow, 3);
    mGrid->addWidget(remove,     gridRow, 4);

    mRows.append(row);

    // Removing the orphan clears the stale argument (one undo step); the deferred re-projection
    // then drops this row.
    connect(remove, &QToolButton::clicked, this, [this, index]()
    {
        if (index < mRows.size())
        {
            commit(index, false, eSource::Value, QString(), QString());
        }
    });
}

void SMArgMapTable::refreshRow(int row)
{
    Row& r = mRows[row];
    if (r.status == nullptr)
    {
        return;     // A Compact row carries no type or status cell.
    }

    const Param& param = r.param;
    const SMArgumentEntry* cur = argFor(row);

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
// Interaction -- Detailed rows
//////////////////////////////////////////////////////////////////////////

void SMArgMapTable::onSourceChanged(int row)
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
    const SMArgumentEntry* cur = argFor(row);

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

void SMArgMapTable::onLiteralCommitted(int row)
{
    if ((mApplying == false) && (row < mRows.size()))
    {
        commit(row, true, eSource::Value, mRows.at(row).literal->text(), QString());
    }
}

void SMArgMapTable::onRefActivated(int row)
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

void SMArgMapTable::onExpressionCommitted(int row)
{
    if ((mApplying == false) && (row < mRows.size()))
    {
        commit(row, true, eSource::Expression, QString(), mRows.at(row).expr->text());
    }
}

//////////////////////////////////////////////////////////////////////////
// Interaction -- Compact rows
//////////////////////////////////////////////////////////////////////////

void SMArgMapTable::onCompactCommitted(int row)
{
    if (mApplying || (row >= mRows.size()))
    {
        return;
    }

    const QString text = mRows.at(row).compact->currentText().trimmed();
    if (text == mRows.at(row).committed)
    {
        return;     // nothing changed (a popup open/close, or a focus hop): never re-project
    }

    mRows[row].committed = text;
    if (text.isEmpty())
    {
        commit(row, false, eSource::Value, QString(), QString());
    }
    else
    {
        commit(row, true, resolveCompactSource(text), text, QString());
    }
}

SMArgumentEntry::eValueSource SMArgMapTable::resolveCompactSource(const QString& text) const
{
    const eSource offered[] = { eSource::Param, eSource::Attribute };
    for (eSource kind : offered)
    {
        if ((kind == eSource::Param) && (mAllowParam == false))
        {
            continue;
        }

        for (const SMSourceEntry& entry : SMMappingSources::candidates(mModel.getData(), mTransId, kind, QString()))
        {
            if (entry.name == text)
            {
                return kind;
            }
        }
    }

    return eSource::Value;
}

//////////////////////////////////////////////////////////////////////////
// Commit
//////////////////////////////////////////////////////////////////////////

void SMArgMapTable::commit(int row, bool mapped, SMArgumentEntry::eValueSource source, const QString& value, const QString& expression)
{
    if ((mSink == nullptr) || (row >= mRows.size()))
    {
        return;
    }

    const QString name = mRows.at(row).param.name;

    mApplying = true;
    if (mapped)
    {
        mSink->setArg(name, source, value, expression);
    }
    else
    {
        mSink->clearArg(name);
    }
    mApplying = false;

    // A Compact row already shows what was committed and its host refreshes through the
    // model notification, so rebuilding here would only fight the caret. A Detailed row's
    // status, type and value cells all follow the new state, so it re-projects (deferred).
    if (mStyle == eRowStyle::Detailed)
    {
        scheduleRebuild();
    }
}

//////////////////////////////////////////////////////////////////////////
// Editor gestures
//////////////////////////////////////////////////////////////////////////

void SMArgMapTable::watchEditor(QLineEdit* edit)
{
    if (edit != nullptr)
    {
        edit->setProperty(PropPreEdit, edit->text());
        edit->installEventFilter(this);
    }
}

bool SMArgMapTable::eventFilter(QObject* watched, QEvent* event)
{
    // While a drop-down is up it holds the mouse grab, so the SECOND click of a double click never
    // reaches the line edit -- it lands on the popup container instead. Catching it here is what
    // makes the "open, close, select the text" fallback work.
    if (event->type() == QEvent::MouseButtonPress)
    {
        QComboBox* owner = comboOfPopup(watched);
        if (owner != nullptr)
        {
            const QPoint local = owner->mapFromGlobal(static_cast<QMouseEvent*>(event)->globalPosition().toPoint());
            if (owner->rect().contains(local))
            {
                owner->hidePopup();
                if (owner->lineEdit() != nullptr)
                {
                    owner->lineEdit()->setFocus();
                    owner->lineEdit()->selectAll();
                }

                return true;
            }
        }
    }

    QLineEdit* edit = qobject_cast<QLineEdit*>(watched);
    if (edit != nullptr)
    {
        switch (event->type())
        {
        case QEvent::FocusIn:
            // The value to restore if this edit is abandoned with Esc.
            edit->setProperty(PropPreEdit, edit->text());
            break;

        case QEvent::KeyPress:
            if (static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape)
            {
                // Revert to the value the field held when editing began and swallow the key:
                // nothing is committed, so the model and the undo stack stay untouched.
                const QSignalBlocker block(edit);
                edit->setText(edit->property(PropPreEdit).toString());
                return true;
            }
            break;

        case QEvent::MouseButtonPress:
            // NOT swallowed: the press must reach QLineEdit so the
            // caret lands exactly where the developer clicked. Opening the popup from the press
            // was also what made the list flash open and shut -- the matching release landed on
            // the freshly grabbed popup and dismissed it at once.
            break;

        case QEvent::MouseButtonRelease:
        {
            // A click in the field opens the value list, rather than leaving it reachable only
            // through the tiny drop arrow. Opening on the RELEASE keeps the list up: by then the
            // press has already positioned the caret and no stray release remains to dismiss it.
            QComboBox* combo = qobject_cast<QComboBox*>(edit->parentWidget());
            if ((combo != nullptr) && (combo->count() > 0) && (combo->view() != nullptr)
                && (combo->view()->isVisible() == false))
            {
                combo->showPopup();
            }
            break;
        }

        case QEvent::MouseButtonDblClick:
            // Double click selects the whole value, ready to be typed over.
            edit->selectAll();
            return true;

        default:
            break;
        }
    }

    return QWidget::eventFilter(watched, event);
}

//////////////////////////////////////////////////////////////////////////
// Helpers / accessors
//////////////////////////////////////////////////////////////////////////

void SMArgMapTable::scheduleRebuild(void)
{
    // Deferred: rebuilding now would delete the editor widget still emitting the signal this
    // call runs in (use-after-free on unwind)
    if (mRebuildQueued)
    {
        return;
    }

    mRebuildQueued = true;
    QTimer::singleShot(0, this, [this]()
    {
        mRebuildQueued = false;
        if (isEditing())
        {
            // The developer has a popup open or a caret in a cell: rebuilding would delete the
            // widget under their hands. The row already shows the live value; re-project when
            // the edit ends.
            return;
        }

        rebuild();
    });
}

QComboBox* SMArgMapTable::comboOfPopup(QObject* container) const
{
    for (const Row& row : mRows)
    {
        if ((row.compact != nullptr) && (row.compact->view() != nullptr)
            && (row.compact->view()->window() == container))
        {
            return row.compact;
        }
    }

    return nullptr;
}

bool SMArgMapTable::isEditing(void) const
{
    for (const Row& row : mRows)
    {
        if (row.compact == nullptr)
        {
            continue;
        }

        if ((row.compact->view() != nullptr) && row.compact->view()->isVisible())
        {
            return true;        // its drop-down is open
        }

        if ((row.compact->lineEdit() != nullptr) && row.compact->lineEdit()->hasFocus())
        {
            return true;        // the caret is in its editable text
        }
    }

    return false;
}

const SMArgumentEntry* SMArgMapTable::argFor(int row) const
{
    return ((mSink != nullptr) && (row >= 0) && (row < mRows.size()))
           ? mSink->argFor(mRows.at(row).param.name)
           : nullptr;
}

QSize SMArgMapTable::minimumSizeHint(void) const
{
    // Width 0: no signature length, constant name or status text may widen the dock.
    return QSize(0, QWidget::minimumSizeHint().height());
}

QLabel* SMArgMapTable::nameLabel(int row) const
{
    return ((row >= 0) && (row < mRows.size())) ? mRows.at(row).name : nullptr;
}

QComboBox* SMArgMapTable::sourceCombo(int row) const
{
    return ((row >= 0) && (row < mRows.size())) ? mRows.at(row).source : nullptr;
}

QWidget* SMArgMapTable::valueWidget(int row) const
{
    if ((row < 0) || (row >= mRows.size()))
    {
        return nullptr;
    }

    const Row& r = mRows.at(row);
    return (r.value != nullptr) ? r.value->currentWidget() : static_cast<QWidget*>(r.compact);
}

QLabel* SMArgMapTable::typeLabel(int row) const
{
    return ((row >= 0) && (row < mRows.size())) ? mRows.at(row).type : nullptr;
}

QLabel* SMArgMapTable::statusLabel(int row) const
{
    return ((row >= 0) && (row < mRows.size())) ? mRows.at(row).status : nullptr;
}
