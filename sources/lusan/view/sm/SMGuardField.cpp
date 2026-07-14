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
 *  \file        lusan/view/sm/SMGuardField.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard field: the editable guard surface (v7 B2, B3).
 *
 ************************************************************************/

#include "lusan/view/sm/SMGuardField.hpp"

#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMAttributeModel.hpp"
#include "lusan/model/sm/SMGuardCodegenPreview.hpp"
#include "lusan/model/sm/SMGuardCommands.hpp"
#include "lusan/model/sm/SMGuardParser.hpp"
#include "lusan/model/sm/SMGuardRender.hpp"
#include "lusan/model/sm/SMGuardSymbols.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"

#include "lusan/view/sm/SMGuardHighlighter.hpp"
#include "lusan/view/sm/SMSignatureCard.hpp"
#include "lusan/view/sm/SMSymbolPopup.hpp"

#include <QAbstractTextDocumentLayout>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QMimeData>
#include <QRegularExpression>
#include <QTextDocument>
#include <QTextEdit>
#include <QTimer>

namespace
{
    const QRegularExpression& slotPattern()
    {
        static const QRegularExpression pattern(QStringLiteral("<[A-Za-z_][A-Za-z0-9_]*>"));
        return pattern;
    }

    bool isWordChar(QChar ch)
    {
        return (ch.isLetterOrNumber() || (ch == QLatin1Char('_')));
    }

    //!< Collects the names of the handler conditions the tree calls (for the chip row).
    void collectHandlerChips(const StateMachineData& data, const SMGuardNode* node, QStringList& chips)
    {
        if (node == nullptr)
        {
            return;
        }

        if (node->getKind() == SMGuardNode::eKind::Call)
        {
            const SMMethodEntry* method = SMGuardSymbols::method(data, node->getSymbolId());
            if ((method != nullptr) && method->isHandlerCondition() && (chips.contains(method->getName()) == false))
            {
                chips.append(method->getName());
            }
        }

        for (const SMGuardNode* child : node->getChildren())
        {
            collectHandlerChips(data, child, chips);
        }
    }

    //!< True when any node in the tree is a raw-C++ fragment.
    int countRawNodes(const SMGuardNode* node)
    {
        if (node == nullptr)
        {
            return 0;
        }

        int count = (node->getKind() == SMGuardNode::eKind::Raw) ? 1 : 0;
        for (const SMGuardNode* child : node->getChildren())
        {
            count += countRawNodes(child);
        }

        return count;
    }
}

//////////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////////

SMGuardField::SMGuardField(StateMachineModel& model, QWidget* parent /*= nullptr*/)
    : QPlainTextEdit    (parent)
    , mModel            (model)
    , mTransitionId     (0u)
    , mAllowRaw         (false)
    , mRebuildPending   (false)
    , mSuppressAnalyze  (false)
    , mHighlighter      (nullptr)
    , mPopup            (nullptr)
    , mSignature        (nullptr)
    , mDebounce         (nullptr)
    , mSlotMode         (false)
    , mCurrentSlot      (-1)
    , mErrorStart       (-1)
    , mErrorLength      (0)
{
    setObjectName(QStringLiteral("smGuardField"));
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    setLineWrapMode(QPlainTextEdit::WidgetWidth);
    setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setTabChangesFocus(true);
    setPlaceholderText(tr("type a condition -- or press Ctrl+Space to see everything you can use"));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    mHighlighter = new SMGuardHighlighter(document());
    mPopup = new SMSymbolPopup(this);
    mSignature = new SMSignatureCard(this);

    mDebounce = new QTimer(this);
    mDebounce->setSingleShot(true);
    mDebounce->setInterval(150);

    connect(mDebounce, &QTimer::timeout, this, &SMGuardField::onDebounce);
    connect(mPopup, &SMSymbolPopup::accepted, this, &SMGuardField::onCompletionAccepted);
    connect(this, &QPlainTextEdit::textChanged, this, [this]()
    {
        if (mSuppressAnalyze)
        {
            return;
        }

        updateHeight();
        mDebounce->start();
        updateCompletion();
    });

    DocModelNotifier& notifier = mModel.getNotifier();
    connect(&notifier, &DocModelNotifier::elementChanged, this, &SMGuardField::onElementChanged);
    connect(&notifier, &DocModelNotifier::elementRemoved, this, &SMGuardField::onElementRemoved);
    connect(&notifier, &DocModelNotifier::documentReloaded, this, &SMGuardField::onDocumentReloaded);

    updateHeight();
}

SMGuardField::~SMGuardField()
{
    mModel.getNotifier().disconnect(this);
}

//////////////////////////////////////////////////////////////////////////
// Public
//////////////////////////////////////////////////////////////////////////

void SMGuardField::setTransition(uint32_t transitionId)
{
    mTransitionId = transitionId;
    buildCatalog();
    rebuildFromModel();
}

void SMGuardField::buildCatalog()
{
    mCatalog = SMGuardCatalog::build(mModel.getData(), mTransitionId);
    mOwnerByName.clear();
    for (const SMGuardSymbol& sym : mCatalog)
    {
        mOwnerByName.insert(sym.name, static_cast<int>(sym.owner));
    }

    mPopup->setSymbols(mCatalog);
}

//////////////////////////////////////////////////////////////////////////
// Notifications
//////////////////////////////////////////////////////////////////////////

void SMGuardField::onElementChanged(uint32_t id, eDocElementKind /*kind*/)
{
    buildCatalog();
    if (id == mTransitionId)
    {
        scheduleRebuild();
    }
    else if (mSuppressAnalyze == false)
    {
        analyze();  // a registry change may resolve / break a name; keep the user's text
    }
}

void SMGuardField::onElementRemoved(uint32_t id, eDocElementKind /*kind*/)
{
    if (id == mTransitionId)
    {
        mTransitionId = 0u;
        scheduleRebuild();
    }
    else
    {
        buildCatalog();
        if (mSuppressAnalyze == false)
        {
            analyze();
        }
    }
}

void SMGuardField::onDocumentReloaded()
{
    mTransitionId = 0u;
    scheduleRebuild();
}

void SMGuardField::scheduleRebuild()
{
    if (mRebuildPending)
    {
        return;
    }

    // Never rebuild synchronously inside a notifier slot (the emitting command is still on the
    // stack). Defer to the next event turn -- the sm-condition-editor discipline.
    mRebuildPending = true;
    QTimer::singleShot(0, this, [this]()
    {
        mRebuildPending = false;
        rebuildFromModel();
    });
}

void SMGuardField::rebuildFromModel()
{
    clearSlotMode();

    SMTransitionEntry* transition = (mTransitionId != 0u) ? mModel.getData().findTransitionById(mTransitionId) : nullptr;

    QString text;
    mAllowRaw = false;
    if (transition != nullptr)
    {
        const SMGuard& guard = transition->getGuard();
        text = SMGuardRender::guardText(mModel.getData(), mTransitionId, guard);
        mAllowRaw = (guard.isDraft() && (countRawNodes(guard.getTree()) > 0))
                     || (guard.isOk() && (countRawNodes(guard.getTree()) > 0));
    }

    mSuppressAnalyze = true;
    setPlainText(text);
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
    mSuppressAnalyze = false;

    mCommittedText = text;
    setEnabled(transition != nullptr);
    updateHeight();
    analyze();
}

//////////////////////////////////////////////////////////////////////////
// Analysis + status
//////////////////////////////////////////////////////////////////////////

void SMGuardField::analyze()
{
    mErrorStart = -1;
    mErrorLength = 0;

    if (mTransitionId == 0u)
    {
        mHighlighter->setDecorations({}, {});
        emit statusUpdated(static_cast<int>(NEGuardStyle::eSeverity::Ok), QString(), QString(), {});
        emit fixesUpdated(QString(), {});
        emit badgeUpdated(false, false);
        return;
    }

    const StateMachineData& data = mModel.getData();
    const QString text = toPlainText();

    SMGuardParser::Result result = SMGuardParser::parse(data, mTransitionId, text, mAllowRaw);

    // ---- decorations -----------------------------------------------------
    QList<SMGuardHighlighter::OwnerSpan> owners;
    QList<SMGuardHighlighter::DiagSpan> diags;

    // Owner colors + raw dotted underline from a live token scan.
    int i = 0;
    const int n = text.length();
    while (i < n)
    {
        const QChar ch = text.at(i);
        if (ch == QLatin1Char('<'))
        {
            // Skip an unfilled slot placeholder so it is not flagged as an unknown name.
            const QRegularExpressionMatch m = slotPattern().match(text, i);
            if (m.hasMatch() && (m.capturedStart() == i))
            {
                i = m.capturedEnd();
                continue;
            }
        }

        if (ch.isDigit())
        {
            const int start = i;
            while ((i < n) && (text.at(i).isLetterOrNumber() || (text.at(i) == QLatin1Char('.')) || (text.at(i) == QLatin1Char('x'))))
            {
                ++i;
            }
            owners.append({ start, i - start, NEGuardStyle::eOwner::Literal });
            continue;
        }

        if (ch.isLetter() || (ch == QLatin1Char('_')))
        {
            const int start = i;
            while ((i < n) && isWordChar(text.at(i)))
            {
                ++i;
            }

            const QString word = text.mid(start, i - start);
            if (mOwnerByName.contains(word))
            {
                owners.append({ start, i - start, static_cast<NEGuardStyle::eOwner>(mOwnerByName.value(word)) });
            }
            else if ((word == QStringLiteral("true")) || (word == QStringLiteral("false")))
            {
                owners.append({ start, i - start, NEGuardStyle::eOwner::Literal });
            }
            else if (mAllowRaw)
            {
                owners.append({ start, i - start, NEGuardStyle::eOwner::Raw });
                diags.append({ start, i - start, SMGuardHighlighter::eUnderline::Raw });
            }
            continue;
        }

        ++i;
    }

    // Diagnostic underlines from the parser (authoritative for errors / warnings).
    for (const SMGuardParser::Diagnostic& diag : result.diagnostics)
    {
        if (diag.severity == SMGuardParser::eSeverity::Error)
        {
            diags.append({ diag.start, diag.length, SMGuardHighlighter::eUnderline::Error });
            if (mErrorStart < 0)
            {
                mErrorStart = diag.start;
                mErrorLength = diag.length;
            }
        }
        else
        {
            diags.append({ diag.start, diag.length, SMGuardHighlighter::eUnderline::Warning });
        }
    }

    mHighlighter->setDecorations(owners, diags);

    // ---- status + fixes --------------------------------------------------
    if (text.trimmed().isEmpty())
    {
        delete result.tree;
        emit statusUpdated(static_cast<int>(NEGuardStyle::eSeverity::Ok), QString(), QString(), {});
        emit fixesUpdated(QString(), {});
        emit badgeUpdated(false, false);
        return;
    }

    bool hasWarning = false;
    QString firstError;
    QString firstWarning;
    for (const SMGuardParser::Diagnostic& diag : result.diagnostics)
    {
        if (diag.severity == SMGuardParser::eSeverity::Warning)
        {
            hasWarning = true;
            if (firstWarning.isEmpty())
            {
                firstWarning = diag.message;
            }
        }
        else if (firstError.isEmpty())
        {
            firstError = diag.message;
        }
    }

    SMGuard guard;
    if (result.resolved())
    {
        guard.setTree(result.tree);
        result.tree = nullptr;
    }
    else
    {
        guard.setDraft(text);
        delete result.tree;
        result.tree = nullptr;
    }

    if (guard.isOk())
    {
        const QString preview = SMGuardCodegenPreview::ifStatement(data, mTransitionId, guard);
        QStringList chips;
        collectHandlerChips(data, guard.getTree(), chips);

        QString verdict = QStringLiteral("boolean");
        const int raw = countRawNodes(guard.getTree());
        if (raw > 0)
        {
            verdict += tr(" (raw code: %1 fragment%2)").arg(raw).arg(raw == 1 ? QString() : QStringLiteral("s"));
        }

        const NEGuardStyle::eSeverity severity = hasWarning ? NEGuardStyle::eSeverity::Warn : NEGuardStyle::eSeverity::Ok;
        emit statusUpdated(static_cast<int>(severity), verdict, preview, chips);

        if (hasWarning)
        {
            QList<SMFixBar::Fix> fixes;
            for (const SMGuardParser::Diagnostic& diag : result.diagnostics)
            {
                if (diag.severity == SMGuardParser::eSeverity::Warning)
                {
                    const QString name = text.mid(diag.start, diag.length);
                    fixes.append({ QStringLiteral("shadow-getter"), tr("Use the attribute: %1()").arg(name), name, true, QString() });
                    fixes.append({ QStringLiteral("shadow-rename"), tr("Rename attribute..."), name, true, QString() });
                    fixes.append({ QStringLiteral("suppress"), tr("Dismiss"), name, true, QString() });
                    break;
                }
            }
            mLastMessage = tr("(!) %1").arg(firstWarning);
            mLastFixes = fixes;
            emit fixesUpdated(mLastMessage, fixes);
        }
        else
        {
            mLastFixes.clear();
            emit fixesUpdated(QString(), {});
        }

        emit badgeUpdated(false, hasWarning);
    }
    else
    {
        const QString token = (mErrorStart >= 0) ? text.mid(mErrorStart, mErrorLength) : QString();
        emit statusUpdated(static_cast<int>(NEGuardStyle::eSeverity::Err)
                           , firstError.isEmpty() ? tr("unresolved guard") : firstError
                           , QString(), {});

        QList<SMFixBar::Fix> fixes;
        if (token.isEmpty() == false)
        {
            const QString suggestion = SMGuardCatalog::nearestName(SMGuardCatalog::completionWords(data, mTransitionId), token);
            if (suggestion.isEmpty() == false)
            {
                fixes.append({ QStringLiteral("use"), tr("Use %1").arg(suggestion), suggestion, true, QString() });
            }

            fixes.append({ QStringLiteral("declare"), tr("Declare..."), token, true, QString() });
            fixes.append({ QStringLiteral("raw"), tr("Keep as raw C++"), token, true, QString() });
            fixes.append({ QStringLiteral("map"), tr("Map arguments..."), token, false, tr("Session U3") });
        }

        mLastMessage = tr("(!) %1").arg(firstError.isEmpty() ? tr("unresolved guard") : firstError);
        mLastFixes = fixes;
        emit fixesUpdated(mLastMessage, fixes);
        emit badgeUpdated(true, false);
    }
}

//////////////////////////////////////////////////////////////////////////
// Commit gate (B3)
//////////////////////////////////////////////////////////////////////////

QString SMGuardField::committableText() const
{
    QString text = toPlainText();
    text.replace(QLatin1Char('\n'), QLatin1Char(' '));
    text.remove(slotPattern());                                     // never commit slot markers
    text.replace(QRegularExpression(QStringLiteral(",\\s*\\)")), QStringLiteral(")"));
    text.replace(QRegularExpression(QStringLiteral("\\(\\s*,")), QStringLiteral("("));
    text.replace(QRegularExpression(QStringLiteral(",\\s*,")), QStringLiteral(","));
    return text.trimmed();
}

void SMGuardField::commit()
{
    if (mTransitionId == 0u)
    {
        return;
    }

    const QString text = committableText();
    StateMachineData& data = mModel.getData();
    SMGuard guard = SMGuardParser::parseToGuard(data, mTransitionId, text, mAllowRaw);

    // No change -> no undo step.
    const QString canonical = SMGuardRender::guardText(data, mTransitionId, guard);
    if (canonical == mCommittedText)
    {
        return;
    }

    SMSetGuardCommand* command = SMGuardCommands::setGuard(data, mModel.getNotifier(), mTransitionId, guard, tr("Edit guard"));
    if (command != nullptr)
    {
        mModel.getUndoStack().push(command);
    }
}

void SMGuardField::revert()
{
    clearSlotMode();
    mSuppressAnalyze = true;
    setPlainText(mCommittedText);
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
    mSuppressAnalyze = false;
    mPopup->hide();
    updateHeight();
    analyze();
}

//////////////////////////////////////////////////////////////////////////
// Quick-fixes
//////////////////////////////////////////////////////////////////////////

void SMGuardField::reopenFixBar()
{
    if (mLastFixes.isEmpty() == false)
    {
        emit fixesUpdated(mLastMessage, mLastFixes);
    }
}

void SMGuardField::applyFix(const QString& id, const QString& payload)
{
    if (id == QStringLiteral("use"))
    {
        if (mErrorStart >= 0)
        {
            QTextCursor cursor = textCursor();
            cursor.setPosition(mErrorStart);
            cursor.setPosition(mErrorStart + mErrorLength, QTextCursor::KeepAnchor);
            cursor.insertText(payload);
            setTextCursor(cursor);
        }

        commit();
    }
    else if (id == QStringLiteral("raw"))
    {
        mAllowRaw = true;
        analyze();
        commit();
    }
    else if (id == QStringLiteral("shadow-getter"))
    {
        // Replace the shadowing bare name with its attribute getter form `name()`.
        const QString bare = payload;
        QTextCursor cursor(document());
        cursor.movePosition(QTextCursor::Start);
        const QString whole = toPlainText();
        const int at = whole.indexOf(bare);
        if (at >= 0)
        {
            cursor.setPosition(at);
            cursor.setPosition(at + bare.length(), QTextCursor::KeepAnchor);
            cursor.insertText(bare + QStringLiteral("()"));
        }

        commit();
    }
    else if (id == QStringLiteral("shadow-rename"))
    {
        // Rename lives on the Attributes page; U2 surfaces the fix but defers the dialog.
        emit fixesUpdated(QString(), {});
    }
    else if (id == QStringLiteral("suppress"))
    {
        emit fixesUpdated(QString(), {});
    }
    else if (id == QStringLiteral("declare"))
    {
        SMAttributeEntry* created = mModel.getAttributeModel().createAttribute(payload);
        if (created != nullptr)
        {
            mModel.getAttributeModel().setType(created->getId(), QStringLiteral("bool"));
        }

        buildCatalog();
        analyze();
        commit();
    }
}

//////////////////////////////////////////////////////////////////////////
// Completion
//////////////////////////////////////////////////////////////////////////

void SMGuardField::openCompletion()
{
    if (mTransitionId == 0u)
    {
        return;
    }

    const QRect rect = cursorRect();
    const QPoint pos = viewport()->mapToGlobal(QPoint(rect.left(), rect.bottom() + 2));
    mPopup->filterAndShow(QString(), pos);
}

void SMGuardField::updateCompletion()
{
    int start = 0;
    int length = 0;
    const QString word = wordUnderCursor(start, length);
    if (word.isEmpty())
    {
        mPopup->hide();
        return;
    }

    const QRect rect = cursorRect();
    const QPoint pos = viewport()->mapToGlobal(QPoint(rect.left(), rect.bottom() + 2));
    mPopup->filterAndShow(word, pos);
}

QString SMGuardField::wordUnderCursor(int& start, int& length) const
{
    const QString text = toPlainText();
    int pos = textCursor().position();
    int left = pos;
    while ((left > 0) && isWordChar(text.at(left - 1)))
    {
        --left;
    }

    int right = pos;
    while ((right < text.length()) && isWordChar(text.at(right)))
    {
        ++right;
    }

    start = left;
    length = right - left;
    return (length > 0) ? text.mid(left, length) : QString();
}

void SMGuardField::onCompletionAccepted(const SMGuardSymbol& symbol)
{
    int start = 0;
    int length = 0;
    wordUnderCursor(start, length);

    QTextCursor cursor = textCursor();
    cursor.setPosition(start);
    cursor.setPosition(start + length, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();

    if (symbol.isCall == false)
    {
        cursor.insertText(symbol.name);
        setTextCursor(cursor);
        commit();
        return;
    }

    clearSlotMode();
    cursor.insertText(symbol.name + QLatin1Char('('));
    mSlots.clear();
    for (int i = 0; i < symbol.paramNames.size(); ++i)
    {
        if (i > 0)
        {
            cursor.insertText(QStringLiteral(", "));
        }

        const int slotStart = cursor.position();
        const QString placeholder = QLatin1Char('<') + symbol.paramNames.at(i) + QLatin1Char('>');
        cursor.insertText(placeholder);

        QTextCursor slot(document());
        slot.setPosition(slotStart);
        slot.setPosition(slotStart + placeholder.length(), QTextCursor::KeepAnchor);
        mSlots.append(slot);
    }

    cursor.insertText(QStringLiteral(")"));

    mSlotMode = true;
    mSlotCall = symbol;
    mCurrentSlot = -1;
    runAutoMap();
    if (selectSlot(1) == false)
    {
        clearSlotMode();
        commit();
    }
}

//////////////////////////////////////////////////////////////////////////
// Mapping slots (B5)
//////////////////////////////////////////////////////////////////////////

void SMGuardField::runAutoMap()
{
    QList<QTextEdit::ExtraSelection> tints;
    for (int i = 0; i < mSlots.size(); ++i)
    {
        const QString type = (i < mSlotCall.paramTypes.size()) ? mSlotCall.paramTypes.at(i) : QString();
        if (type.isEmpty())
        {
            continue;
        }

        QString unique;
        int matches = 0;
        for (const SMGuardSymbol& sym : mCatalog)
        {
            if ((sym.isCall == false) && (sym.typeText == type))
            {
                unique = sym.name;
                ++matches;
            }
        }

        if (matches != 1)
        {
            continue;   // propose never guess: only a unique exact-type candidate pre-fills
        }

        QTextCursor slot = mSlots.at(i);
        slot.insertText(unique);
        const int end = slot.position();
        slot.setPosition(end - unique.length());
        slot.setPosition(end, QTextCursor::KeepAnchor);
        mSlots[i] = slot;

        QTextEdit::ExtraSelection tint;
        tint.cursor = slot;
        tint.format.setBackground(NEGuardStyle::autoMapTint());
        tints.append(tint);
    }

    setExtraSelections(tints);
}

bool SMGuardField::selectSlot(int direction)
{
    if (mSlots.isEmpty())
    {
        return false;
    }

    const int next = mCurrentSlot + ((direction >= 0) ? 1 : -1);
    if ((next < 0) || (next >= mSlots.size()))
    {
        return false;
    }

    mCurrentSlot = next;
    setTextCursor(mSlots.at(mCurrentSlot));

    // Visiting a slot clears its auto-map tint.
    QList<QTextEdit::ExtraSelection> tints = extraSelections();
    for (int i = tints.size() - 1; i >= 0; --i)
    {
        if (tints.at(i).cursor.selectedText() == mSlots.at(mCurrentSlot).selectedText())
        {
            tints.removeAt(i);
        }
    }

    setExtraSelections(tints);
    updateSignatureCard();
    return true;
}

int SMGuardField::activeSlotParam() const
{
    return mCurrentSlot;
}

void SMGuardField::updateSignatureCard()
{
    if ((mSlotMode == false) || (mCurrentSlot < 0))
    {
        mSignature->hide();
        return;
    }

    const QRect rect = cursorRect();
    const QPoint pos = viewport()->mapToGlobal(QPoint(rect.left(), rect.bottom() + 4));
    mSignature->showFor(mSlotCall, mCurrentSlot, pos);
}

void SMGuardField::clearSlotMode()
{
    mSlotMode = false;
    mSlots.clear();
    mCurrentSlot = -1;
    if (mSignature != nullptr)
    {
        mSignature->hide();
    }

    setExtraSelections({});
}

//////////////////////////////////////////////////////////////////////////
// Layout
//////////////////////////////////////////////////////////////////////////

void SMGuardField::updateHeight()
{
    const QFontMetrics metrics(font());
    const int lineHeight = metrics.lineSpacing();
    const qreal docHeight = document()->documentLayout()->documentSize().height();
    const int frame = 2 * static_cast<int>(frameWidth()) + 8;
    const int minHeight = lineHeight + frame;
    const int maxHeight = (lineHeight * 4) + frame;
    setFixedHeight(qBound(minHeight, static_cast<int>(docHeight) + frame, maxHeight));
}

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////

void SMGuardField::keyPressEvent(QKeyEvent* event)
{
    if (mPopup->isVisible())
    {
        switch (event->key())
        {
        case Qt::Key_Up:        mPopup->moveCurrent(-1);    return;
        case Qt::Key_Down:      mPopup->moveCurrent(1);     return;
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Tab:       mPopup->acceptCurrent();    return;
        case Qt::Key_Escape:    mPopup->hide();             return;
        default:                                            break;
        }
    }

    if ((event->key() == Qt::Key_Space) && (event->modifiers() & Qt::ControlModifier))
    {
        openCompletion();
        return;
    }

    if ((event->key() == Qt::Key_Period) && (event->modifiers() & Qt::ControlModifier))
    {
        reopenFixBar();
        return;
    }

    if (mSlotMode && (event->key() == Qt::Key_Tab))
    {
        if (selectSlot(1) == false)
        {
            clearSlotMode();
            commit();
        }

        return;
    }

    if (mSlotMode && (event->key() == Qt::Key_Backtab))
    {
        selectSlot(-1);
        return;
    }

    if ((event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Enter))
    {
        commit();
        return;
    }

    if (event->key() == Qt::Key_Escape)
    {
        revert();
        return;
    }

    QPlainTextEdit::keyPressEvent(event);
}

void SMGuardField::focusOutEvent(QFocusEvent* event)
{
    mPopup->hide();
    if (mSignature != nullptr)
    {
        mSignature->hide();
    }

    commit();
    QPlainTextEdit::focusOutEvent(event);
}

void SMGuardField::insertFromMimeData(const QMimeData* source)
{
    if (source->hasText())
    {
        QString text = source->text();
        text.replace(QLatin1Char('\n'), QLatin1Char(' '));
        text.replace(QLatin1Char('\r'), QString());
        insertPlainText(text);
    }
}

void SMGuardField::onDebounce()
{
    analyze();
}
