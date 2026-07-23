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
#include <QApplication>
#include <QComboBox>
#include <QCompleter>
#include <QEvent>
#include <QFontDatabase>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
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

    //!< The owner hue of a source kind, so a marker is tinted like the same symbol everywhere else.
    NEGuardStyle::eOwner ownerFor(eSource source)
    {
        switch (source)
        {
        case eSource::Param:        return NEGuardStyle::eOwner::Stimulus;
        case eSource::Attribute:    return NEGuardStyle::eOwner::Fsm;
        case eSource::Constant:     return NEGuardStyle::eOwner::Fsm;
        case eSource::Condition:    return NEGuardStyle::eOwner::Handler;
        default:                    return NEGuardStyle::eOwner::Literal;
        }
    }

    /**
     * \brief   The per-entry marker of a merged value cell: the kind's owner glyph, drawn in the
     *          kind's owner hue. It tells a stimulus parameter from an attribute, a constant or a
     *          typed value at a glance, WITHOUT touching the item's text -- the text is the bare
     *          name, because it is also what the editable field commits.
     **/
    QIcon markerIcon(eSource source)
    {
        const qreal ratio = qApp->devicePixelRatio();
        const int   side  = 14;

        QPixmap pix(qRound(side * ratio), qRound(side * ratio));
        pix.setDevicePixelRatio(ratio);
        pix.fill(Qt::transparent);

        QPainter painter(&pix);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::TextAntialiasing, true);

        QFont font = QApplication::font();
        font.setPointSizeF(font.pointSizeF() * 0.85);
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(NEGuardStyle::ownerColor(ownerFor(source)));
        painter.drawText(QRectF(0.0, 0.0, side, side), Qt::AlignCenter, glyphFor(source));

        return QIcon(pix);
    }

    //!< The human name of a source kind, shown as an entry's tooltip in a merged value cell.
    QString kindTip(eSource source)
    {
        switch (source)
        {
        case eSource::Param:        return SMArgMapTable::tr("A parameter of the stimulus that fires this transition");
        case eSource::Attribute:    return SMArgMapTable::tr("A state-machine attribute");
        case eSource::Constant:     return SMArgMapTable::tr("A state-machine constant");
        case eSource::Condition:    return SMArgMapTable::tr("A declared condition");
        default:                    return SMArgMapTable::tr("A fixed value typed here");
        }
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
        // scroll area -- both cells shrink, and the operations editor already scrolls. The form
        // gives every row one shared field column, so the merged cells all line up at one width
        // however long or short the parameter names beside them are.
        QFormLayout* form = new QFormLayout(mHost);
        form->setContentsMargins(12, 2, 0, 0);
        form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
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
    if (mParams.at(index).orphan)
    {
        buildCompactOrphanRow(index);
        return;
    }

    const Param& param = mParams.at(index);

    Row row {};
    row.param = param;
    row.customIndex = -1;

    row.name = new QLabel(param.name + QStringLiteral(" (") + param.type + QLatin1Char(')'), mHost);

    row.compact = new QComboBox(mHost);
    row.compact->setObjectName(QStringLiteral("smArgValue_%1").arg(index));
    row.compact->setEditable(true);
    row.compact->setInsertPolicy(QComboBox::NoInsert);
    makeShrinkable(row.compact);
    // The merged cell is the widest thing on the row and every row shares one form column, so all
    // cells line up at the same width whatever their parameter names are.
    row.compact->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // The list is built in a fixed reading order and NOTHING is listed that does not exist -- a
    // kind with no members contributes no rows at all rather than a placeholder:
    //   [0]      the empty entry (always) -- picking it un-maps the formal
    //   [1]      the one custom value, when this formal currently holds a typed literal
    //   then     stimulus parameters, then attributes, then constants -- each entry marked with
    //            its kind's glyph, so what a name refers to is readable without opening anything.
    row.compact->addItem(QString());

    const SMArgumentEntry* cur = (mSink != nullptr) ? mSink->argFor(param.name) : nullptr;
    const QString value = (cur != nullptr) ? cur->getValue() : QString();
    if ((cur != nullptr) && (cur->getSource() == eSource::Value) && (value.isEmpty() == false))
    {
        row.customIndex = row.compact->count();
        row.compact->addItem(markerIcon(eSource::Value), value);
        row.compact->setItemData(row.customIndex, kindTip(eSource::Value), Qt::ToolTipRole);
    }

    // An empty target type ranks every entry Unknown rather than Mismatch, so the list stays
    // unfiltered -- an entry is offered whatever its type, and the row's status says how it fits.
    for (eSource kind : compactKinds())
    {
        for (const SMSourceEntry& entry : SMMappingSources::candidates(mModel.getData(), mTransId, kind, QString()))
        {
            row.compact->addItem(markerIcon(kind), entry.name);
            row.compact->setItemData(row.compact->count() - 1, kindTip(kind), Qt::ToolTipRole);
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
        // Select the mapped entry -- the custom value or an offered source -- so opening the popup
        // lands on, highlights and scrolls to what is currently mapped instead of the first row.
        const int found = value.isEmpty() ? 0 : row.compact->findText(value);
        row.compact->setCurrentIndex((found >= 0) ? found : 0);
        row.compact->setEditText(value);
        row.committed = value;
    }

    row.compact->lineEdit()->setPlaceholderText(param.hasDefault
        ? tr("default: %1").arg(param.defaultText)
        : tr("value, or pick a source"));

    // How the current mapping fits, on the cell itself: a Compact row has no status column, so the
    // verdict (required / converts / narrows / mismatch) rides the tooltip and the name tint.
    row.compact->setToolTip(compactStatus(param, cur));

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

void SMArgMapTable::buildCompactOrphanRow(int index)
{
    // The same orphan case the Detailed grid renders, in one form row: the formal behind this stored
    // mapping was removed on the Methods page, so the value can no longer be re-typed as a live
    // argument. It is never silently discarded -- it stays in a red row until the developer clears
    // it through the quick-fix beside it.
    const Param& param = mParams.at(index);

    Row row {};
    row.param = param;
    row.customIndex = -1;

    const QString redStyle = QStringLiteral("color: %1;")
                             .arg(NEGuardStyle::severityColor(NEGuardStyle::eSeverity::Err).name());

    row.name = new QLabel(param.name + tr(" (removed)"), mHost);
    row.name->setStyleSheet(redStyle);

    const SMArgumentEntry* cur = (mSink != nullptr) ? mSink->argFor(param.name) : nullptr;

    QWidget* cell = new QWidget(mHost);
    QHBoxLayout* cellBox = new QHBoxLayout(cell);
    cellBox->setContentsMargins(0, 0, 0, 0);

    QLabel* value = new QLabel((cur != nullptr) ? cur->getValue() : QString(), cell);
    value->setStyleSheet(redStyle);
    cellBox->addWidget(value, 1);

    QToolButton* remove = new QToolButton(cell);
    remove->setText(tr("x"));
    remove->setToolTip(tr("The mapped parameter no longer exists; remove the stale binding"));
    cellBox->addWidget(remove);

    static_cast<QFormLayout*>(mHost->layout())->addRow(row.name, cell);
    mRows.append(row);

    connect(remove, &QToolButton::clicked, this, [this, index]()
    {
        if (index < mRows.size())
        {
            commit(index, false, eSource::Value, QString(), QString());
        }
    });
}

QList<SMArgumentEntry::eValueSource> SMArgMapTable::compactKinds(void) const
{
    // The reference kinds a merged cell offers, in list order. A host that declared a source filter
    // gets exactly its reference kinds (the Actions tab: parameters, attributes, constants); a host
    // that declared none keeps the historical pair. A kind that is illegal in this scope -- a
    // stimulus parameter outside a transition -- is dropped, and a kind with no members contributes
    // nothing, so the list never carries an entry that cannot be picked.
    const eSource ordered[] = { eSource::Param, eSource::Attribute, eSource::Constant };

    QList<eSource> kinds;
    for (eSource kind : ordered)
    {
        if (mAllowedSources.isEmpty())
        {
            if (kind == eSource::Constant)
            {
                continue;
            }
        }
        else if (mAllowedSources.contains(kind) == false)
        {
            continue;
        }

        if ((kind == eSource::Param)
            && ((mAllowParam == false) || (SMMappingSources::isKindLegal(mModel.getData(), mTransId, kind) == false)))
        {
            continue;
        }

        kinds.append(kind);
    }

    return kinds;
}

QString SMArgMapTable::compactStatus(const Param& param, const SMArgumentEntry* cur) const
{
    if (cur == nullptr)
    {
        return param.hasDefault
               ? tr("Not mapped: the declared default (%1) is used").arg(param.defaultText)
               : tr("Required: pick a source or type a value");
    }

    if (cur->getSource() == eSource::Value)
    {
        const QString err = SMLiteralValidator::validate(param.type, cur->getValue());
        return err.isEmpty() ? tr("A fixed value of type %1").arg(param.type) : err;
    }

    const QString kind = labelFor(cur->getSource());
    const QString srcType = SMMappingSources::referencedType(mModel.getData(), mTransId, cur->getSource(), cur->getValue());
    switch (SMTypeCompat::rank(srcType, param.type))
    {
    case SMTypeCompat::eRank::Converts:
        return tr("%1 %2 (%3) converts to %4").arg(kind, cur->getValue(), srcType, param.type);

    case SMTypeCompat::eRank::Narrows:
        return tr("(!) %1 %2 (%3) narrows to %4").arg(kind, cur->getValue(), srcType, param.type);

    case SMTypeCompat::eRank::Mismatch:
        return tr("%1 %2 (%3) does not fit %4").arg(kind, cur->getValue(), srcType, param.type);

    default:
        return tr("%1 %2").arg(kind, cur->getValue());
    }
}

void SMArgMapTable::updateCompactCustom(int row)
{
    // The one custom value: a merged cell lists at most a single typed literal, so a newly typed
    // value REPLACES the one listed before it rather than piling up. Done in place -- a Compact
    // commit deliberately does not re-project (it would fight the caret), so without this the list
    // would still be offering the value the developer just typed over.
    Row& r = mRows[row];
    if (r.compact == nullptr)
    {
        return;
    }

    const QSignalBlocker block(r.compact);
    const QString text = r.committed;
    const bool isCustom = (text.isEmpty() == false) && (resolveCompactSource(text) == eSource::Value);

    if (isCustom == false)
    {
        if (r.customIndex >= 0)
        {
            r.compact->removeItem(r.customIndex);
            r.customIndex = -1;
        }
    }
    else if (r.customIndex >= 0)
    {
        r.compact->setItemText(r.customIndex, text);
    }
    else
    {
        r.customIndex = 1;      // straight after the empty entry, ahead of every reference kind
        r.compact->insertItem(r.customIndex, markerIcon(eSource::Value), text);
        r.compact->setItemData(r.customIndex, kindTip(eSource::Value), Qt::ToolTipRole);
    }

    const int found = text.isEmpty() ? 0 : r.compact->findText(text);
    r.compact->setCurrentIndex((found >= 0) ? found : 0);
    r.compact->setEditText(text);
    r.compact->setToolTip(compactStatus(r.param, argFor(row)));
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

    updateCompactCustom(row);
}

SMArgumentEntry::eValueSource SMArgMapTable::resolveCompactSource(const QString& text) const
{
    for (eSource kind : compactKinds())
    {
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
