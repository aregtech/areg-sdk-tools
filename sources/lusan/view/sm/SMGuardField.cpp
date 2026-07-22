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
 *  \brief       Lusan application, FSM guard field: the editable guard surface.
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
#include "lusan/view/sm/SMHoverCard.hpp"
#include "lusan/view/sm/SMInlineToken.hpp"
#include "lusan/view/sm/SMRefCompleter.hpp"
#include "lusan/view/sm/SMSignatureCard.hpp"

#include <QAbstractTextDocumentLayout>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QRegularExpression>
#include <QSet>
#include <QTextDocument>
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

    //!< Replaces every display ghost `<name>` with an equal-length identifier of underscores and
    //!< collects the names. Equal length is the point: the parser's diagnostic offsets keep
    //!< addressing the very same characters of the document, so the underlines stay correct.
    QString maskGhosts(const QString& text, QStringList& names)
    {
        QString out = text;
        QRegularExpressionMatchIterator it = slotPattern().globalMatch(text);
        while (it.hasNext())
        {
            const QRegularExpressionMatch match = it.next();
            const QString whole = match.captured();
            names.append(whole.mid(1, whole.length() - 2));
            out.replace(match.capturedStart(), whole.length(), QString(whole.length(), QLatin1Char('_')));
        }

        return out;
    }

    //!< The status text for one or more unmapped formals: `unmapped <name> parameter`.
    QString unmappedMessage(const QStringList& names)
    {
        QStringList shown;
        for (const QString& name : names)
        {
            shown.append(QLatin1Char('<') + name + QLatin1Char('>'));
        }

        return (names.size() == 1)
               ? QObject::tr("unmapped %1 parameter").arg(shown.first())
               : QObject::tr("unmapped parameters: %1").arg(shown.join(QStringLiteral(", ")));
    }

    //!< The index just past the matching '}' of the '{' at \p i (strings/comments skipped).
    int scanBrace(const QString& s, int i, int n)
    {
        int depth = 0;
        while (i < n)
        {
            const QChar c = s.at(i);
            if ((c == QLatin1Char('/')) && (i + 1 < n) && (s.at(i + 1) == QLatin1Char('/')))
            {
                i += 2;
                while ((i < n) && (s.at(i) != QLatin1Char('\n'))) { ++i; }
                continue;
            }
            if ((c == QLatin1Char('/')) && (i + 1 < n) && (s.at(i + 1) == QLatin1Char('*')))
            {
                i += 2;
                while ((i + 1 < n) && !((s.at(i) == QLatin1Char('*')) && (s.at(i + 1) == QLatin1Char('/')))) { ++i; }
                i += 2;
                continue;
            }
            if ((c == QLatin1Char('"')) || (c == QLatin1Char('\'')))
            {
                const QChar quote = c;
                ++i;
                while (i < n)
                {
                    if (s.at(i) == QLatin1Char('\\')) { i += 2; continue; }
                    if (s.at(i) == quote) { ++i; break; }
                    ++i;
                }
                continue;
            }
            if (c == QLatin1Char('{')) { ++depth; }
            else if (c == QLatin1Char('}'))
            {
                --depth;
                if (depth == 0) { return i + 1; }
            }
            ++i;
        }

        return n;   // unbalanced
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

    //!< The count of raw-C++ fragments in the tree.
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

    //!< The chip owner hue for a render role (chips reuse the render owner roles).
    NEGuardStyle::eOwner roleToOwner(SMGuardRender::eRole role)
    {
        switch (role)
        {
        case SMGuardRender::eRole::Stim:    return NEGuardStyle::eOwner::Stimulus;
        case SMGuardRender::eRole::Handler: return NEGuardStyle::eOwner::Handler;
        case SMGuardRender::eRole::Fsm:
        default:                            return NEGuardStyle::eOwner::Fsm;
        }
    }

    //!< True when the tree contains at least one Call node.
    bool hasCallNode(const SMGuardNode* node)
    {
        if (node == nullptr)
        {
            return false;
        }

        if (node->getKind() == SMGuardNode::eKind::Call)
        {
            return true;
        }

        for (const SMGuardNode* child : node->getChildren())
        {
            if (hasCallNode(child))
            {
                return true;
            }
        }

        return false;
    }
}

//////////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////////

SMGuardField::SMGuardField(StateMachineModel& model, QWidget* parent /*= nullptr*/)
    : QTextEdit         (parent)
    , mModel            (model)
    , mTransitionId     (0u)
    , mAllowRaw         (false)
    , mRebuildPending   (false)
    , mMinLines         (3)
    , mMaxLines         (12)
    , mAutoHeight       (true)
    , mSuppressAnalyze  (false)
    , mAutoCommit       (true)
    , mHighlighter      (nullptr)
    , mCompleter        (nullptr)
    , mCompleterDismissedAt (-1)
    , mSignature        (nullptr)
    , mDebounce         (nullptr)
    , mSlotMode         (false)
    , mCurrentSlot      (-1)
    , mErrorStart       (-1)
    , mErrorLength      (0)
    , mRawBindMode      (false)
    , mRawBindPath      ( )
    , mTokenHandler     (nullptr)
    , mHover            (nullptr)
    , mHoverTimer       (nullptr)
    , mHoverWord        ( )
    , mLastCursorPos    (0)
{
    setObjectName(QStringLiteral("smGuardField"));
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    setAcceptRichText(false);
    setLineWrapMode(QTextEdit::WidgetWidth);
    setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setTabChangesFocus(true);
    // An empty field discovers both the bare-typing and the picker paths. Qt
    // paints this as placeholder text, so it is never a real character and never reaches
    // committableText().
    setPlaceholderText(tr("type a condition, or @ to pick a symbol"));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMouseTracking(true);

    // The island token painter: registered on the rich-text layout, which is the
    // reason this field derives QTextEdit (QPlainTextDocumentLayout never paints these).
    mTokenHandler = new SMInlineToken(this);
    document()->documentLayout()->registerHandler(SMInlineToken::IslandType, mTokenHandler);
    // The same handler paints reference chips; it dispatches on the object type.
    document()->documentLayout()->registerHandler(SMInlineToken::ChipType, mTokenHandler);

    mHighlighter = new SMGuardHighlighter(document());
    mCompleter = new SMRefCompleter(this);
    mSignature = new SMSignatureCard(this);

    mDebounce = new QTimer(this);
    mDebounce->setSingleShot(true);
    mDebounce->setInterval(150);

    mHoverTimer = new QTimer(this);
    mHoverTimer->setSingleShot(true);
    mHoverTimer->setInterval(300);

    connect(mDebounce, &QTimer::timeout, this, &SMGuardField::onDebounce);
    connect(mCompleter, &SMRefCompleter::accepted, this, &SMGuardField::onCompleterAccepted);
    connect(mCompleter, &SMRefCompleter::newLambdaRequested, this, [this]()
    {
        const int index = insertIslandAtCaret(QString());
        emit islandEditRequested(index, QString());
    });
    connect(this, &QTextEdit::textChanged, this, [this]()
    {
        if (mSuppressAnalyze)
        {
            return;
        }

        updateHeight();
        mDebounce->start();
        updateCompleter();
        updateSignatureHelp();
    });
    connect(this, &QTextEdit::cursorPositionChanged, this, [this]()
    {
        // Caret-entry opens the island editor: crossing exactly one character that
        // is a token counts as entering it.
        const int pos = textCursor().position();
        const int old = mLastCursorPos;
        mLastCursorPos = pos;
        if (qAbs(pos - old) == 1)
        {
            maybeOpenIslandAt(qMin(pos, old));
        }
        updateSignatureHelp();
    });
    connect(mHoverTimer, &QTimer::timeout, this, [this]()
    {
        if ((mHover == nullptr) || mHoverWord.isEmpty())
        {
            return;
        }

        for (const SMGuardSymbol& sym : mCatalog)
        {
            if (sym.name == mHoverWord)
            {
                const QPoint pos = mapToGlobal(QPoint(0, height() + 2));
                mHover->showSymbol(mModel, mTransitionId, sym, pos);
                break;
            }
        }
    });

    DocModelNotifier& notifier = mModel.getNotifier();
    connect(&notifier, &DocModelNotifier::elementAdded, this, &SMGuardField::onElementAdded);
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

void SMGuardField::setHoverCard(SMHoverCard* card)
{
    mHover = card;
}

void SMGuardField::buildCatalog()
{
    mCatalog = SMGuardCatalog::build(mModel.getData(), mTransitionId);
    mOwnerByName.clear();
    for (const SMGuardSymbol& sym : mCatalog)
    {
        mOwnerByName.insert(sym.name, static_cast<int>(sym.owner));
    }

    mCompleter->setSymbols(mCatalog);
}

void SMGuardField::commitNow()
{
    commit();
}

void SMGuardField::setAutoCommit(bool enable)
{
    mAutoCommit = enable;
}

void SMGuardField::selectSpan(int start, int length)
{
    const int from = expandedToDoc(start);
    const int to = expandedToDoc(start + length);

    QTextCursor cursor = textCursor();
    cursor.setPosition(from);
    cursor.setPosition(qMax(to, from + 1), QTextCursor::KeepAnchor);
    setTextCursor(cursor);
    setFocus();
}

void SMGuardField::appendClause(const QString& combiner, const QString& clauseText)
{
    QList<int> docStarts;
    const bool empty = expandedText(docStarts).trimmed().isEmpty();

    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
    insertPlainText(empty ? clauseText
                          : (QLatin1Char(' ') + combiner + QLatin1Char(' ') + clauseText));
    setFocus();

    // The insertion stays visible for a beat before the commit reflows to canonical form:
    // the popover teaches the syntax by showing exactly what it typed for the user.
    QTimer::singleShot(350, this, [this]()
    {
        commit();
    });
}

void SMGuardField::appendIsland()
{
    QList<int> docStarts;
    const bool empty = expandedText(docStarts).trimmed().isEmpty();

    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
    if (empty == false)
    {
        insertPlainText(QStringLiteral(" && "));
    }

    const int index = insertIslandAtCaret(QString());
    emit islandEditRequested(index, QString());
}

//////////////////////////////////////////////////////////////////////////
// Islands
//////////////////////////////////////////////////////////////////////////

QList<int> SMGuardField::islandPositions() const
{
    QList<int> positions;
    const QString text = toPlainText();
    QTextCursor cursor(document());
    for (int i = 0; i < text.length(); ++i)
    {
        if (text.at(i) == QChar::ObjectReplacementCharacter)
        {
            cursor.setPosition(i + 1);
            if (SMInlineToken::isIsland(cursor.charFormat()))
            {
                positions.append(i);
            }
        }
    }

    return positions;
}

int SMGuardField::islandCount() const
{
    return static_cast<int>(islandPositions().size());
}

QString SMGuardField::islandBody(int index) const
{
    const QList<int> positions = islandPositions();
    if ((index < 0) || (index >= positions.size()))
    {
        return QString();
    }

    QTextCursor cursor(document());
    cursor.setPosition(positions.at(index) + 1);
    return SMInlineToken::bodyOf(cursor.charFormat());
}

void SMGuardField::setIslandBody(int index, const QString& body)
{
    const QList<int> positions = islandPositions();
    if ((index < 0) || (index >= positions.size()))
    {
        return;
    }

    mSuppressAnalyze = true;
    QTextCursor cursor(document());
    cursor.setPosition(positions.at(index));
    cursor.setPosition(positions.at(index) + 1, QTextCursor::KeepAnchor);
    cursor.insertText(QString(QChar::ObjectReplacementCharacter), SMInlineToken::makeFormat(body));
    mSuppressAnalyze = false;
    analyze();
}

int SMGuardField::insertIslandAtCaret(const QString& body)
{
    QTextCursor cursor = textCursor();
    const int at = cursor.position();
    cursor.insertText(QString(QChar::ObjectReplacementCharacter), SMInlineToken::makeFormat(body));
    // Typing continues after the token in the default format, not the token's.
    cursor.setCharFormat(QTextCharFormat());
    setTextCursor(cursor);

    int index = 0;
    for (int pos : islandPositions())
    {
        if (pos == at)
        {
            break;
        }

        ++index;
    }

    return index;
}

QString SMGuardField::expandedText(QList<int>& docStarts) const
{
    const QString doc = toPlainText();
    QString out;
    out.reserve(doc.length());
    docStarts.clear();
    docStarts.reserve(doc.length() + 1);

    QTextCursor cursor(document());
    for (int i = 0; i < doc.length(); ++i)
    {
        docStarts.append(static_cast<int>(out.length()));
        if (doc.at(i) == QChar::ObjectReplacementCharacter)
        {
            cursor.setPosition(i + 1);
            const QTextCharFormat format = cursor.charFormat();
            if (SMInlineToken::isIsland(format))
            {
                out += QLatin1Char('{') + SMInlineToken::bodyOf(format) + QLatin1Char('}');
                continue;
            }
            if (SMInlineToken::isChip(format))
            {
                out += SMInlineToken::bodyOf(format);   // the canonical #kind:name
                continue;
            }
        }

        out += doc.at(i);
    }

    docStarts.append(static_cast<int>(out.length()));
    return out;
}

int SMGuardField::expandedToDoc(int expandedPos) const
{
    QList<int> docStarts;
    expandedText(docStarts);

    // The last document position whose expanded offset is <= expandedPos: positions
    // inside an expansion collapse onto the token character.
    int doc = 0;
    for (int i = 0; i < docStarts.size(); ++i)
    {
        if (docStarts.at(i) <= expandedPos)
        {
            doc = i;
        }
        else
        {
            break;
        }
    }

    return doc;
}

void SMGuardField::foldIslands()
{
    const QString text = toPlainText();
    // Fold back-to-front so earlier offsets stay valid.
    QList<QPair<int, int>> blocks;  // start, end (past '}')
    int i = 0;
    const int n = text.length();
    while (i < n)
    {
        if (text.at(i) == QLatin1Char('{'))
        {
            const int end = scanBrace(text, i, n);
            blocks.append({ i, end });
            i = end;
            continue;
        }

        ++i;
    }

    mSuppressAnalyze = true;
    for (int b = static_cast<int>(blocks.size()) - 1; b >= 0; --b)
    {
        const int start = blocks.at(b).first;
        const int end = blocks.at(b).second;
        const QString body = text.mid(start + 1, qMax(0, end - start - 2));

        QTextCursor cursor(document());
        cursor.setPosition(start);
        cursor.setPosition(end, QTextCursor::KeepAnchor);
        cursor.insertText(QString(QChar::ObjectReplacementCharacter), SMInlineToken::makeFormat(body));
    }

    mSuppressAnalyze = false;
}

void SMGuardField::foldChips(const QList<SMGuardRender::Chip>& chips)
{
    if (chips.isEmpty())
    {
        return;
    }

    mSuppressAnalyze = true;
    // Back-to-front: replacing a later span never invalidates an earlier span's offset.
    for (int c = static_cast<int>(chips.size()) - 1; c >= 0; --c)
    {
        const SMGuardRender::Chip& chip = chips.at(c);
        const QString prefix = NEGuardText::refPrefix(chip.kind);

        QTextCursor cursor(document());
        cursor.setPosition(chip.start);
        cursor.setPosition(chip.start + chip.length, QTextCursor::KeepAnchor);
        // The committable body is EXACTLY the rendered span (bare `name`, or a minimally
        // `#kind:`-disambiguated form) so committableText and the clipboard stay byte-exact
        // with the canonical text. The explicit `#kind:name` edit form for the
        // double-click de-render is rebuilt from the prefix + name.
        const QString body = cursor.selectedText();
        cursor.insertText(QString(QChar::ObjectReplacementCharacter)
                        , SMInlineToken::makeChipFormat(body, chip.name, roleToOwner(chip.role), prefix, chip.reveal));
    }

    // Typing after a chip continues in the default format, not the token's.
    QTextCursor cursor = textCursor();
    cursor.setCharFormat(QTextCharFormat());
    mSuppressAnalyze = false;
}

void SMGuardField::reprojectChips()
{
    // A foreign change (a method / attribute / constant / parameter renamed or retyped on another
    // page) must update what the folded chips SHOW without reflowing the field text -- a reflow
    // would clobber a half-typed token the user is in the middle of writing. The
    // committed tree is unchanged (references key on ids, P1); only the names it renders through
    // have moved. So re-render the committed tree and rewrite each folded chip token's format in
    // place. Text length never changes, so offsets, the caret, and any uncommitted text are all
    // preserved; only the chips respell.
    SMTransitionEntry* transition = (mTransitionId != 0u) ? mModel.getData().findTransitionById(mTransitionId) : nullptr;
    if (transition == nullptr)
    {
        return;
    }

    const SMGuard& guard = transition->getGuard();
    if ((guard.isOk() == false) || (guard.getTree() == nullptr))
    {
        return;     // a draft carries no folded chips.
    }

    const QList<int> positions = chipPositions();
    if (positions.isEmpty())
    {
        return;
    }

    const SMGuardRender::Rendered rendered = SMGuardRender::render(mModel.getData(), mTransitionId, *guard.getTree());
    if (rendered.chips.size() != positions.size())
    {
        // A de-rendered chip (double-click edit) desyncs the count; leave the respell to the next
        // reflow rather than risk mapping a chip onto the wrong token.
        return;
    }

    mSuppressAnalyze = true;
    // Back-to-front, matching foldChips: a format-only edit never shifts offsets, but stay
    // defensive and consistent.
    for (int c = static_cast<int>(rendered.chips.size()) - 1; c >= 0; --c)
    {
        const SMGuardRender::Chip& chip = rendered.chips.at(c);
        const QString body   = rendered.text.mid(chip.start, chip.length);
        const QString prefix = NEGuardText::refPrefix(chip.kind);

        QTextCursor cursor(document());
        cursor.setPosition(positions.at(c));
        cursor.setPosition(positions.at(c) + 1, QTextCursor::KeepAnchor);
        cursor.setCharFormat(SMInlineToken::makeChipFormat(body, chip.name, roleToOwner(chip.role), prefix, chip.reveal));
    }
    mSuppressAnalyze = false;

    // Keep the committed-text snapshot in step (same convention as rebuildFromModel), so a later
    // focus-out commit still no-ops when the user made no edit of their own -- the foreign rename
    // must not manufacture a spurious undo step.
    mCommittedText = rendered.text;
}

int SMGuardField::chipCount() const
{
    return static_cast<int>(chipPositions().size());
}

QList<int> SMGuardField::chipPositions() const
{
    QList<int> positions;
    const QString text = toPlainText();
    QTextCursor cursor(document());
    for (int i = 0; i < text.length(); ++i)
    {
        if (text.at(i) == QChar::ObjectReplacementCharacter)
        {
            cursor.setPosition(i + 1);
            if (SMInlineToken::isChip(cursor.charFormat()))
            {
                positions.append(i);
            }
        }
    }

    return positions;
}

bool SMGuardField::deRenderChipAt(int docPos)
{
    if ((docPos < 0) || (document()->characterAt(docPos) != QChar::ObjectReplacementCharacter))
    {
        return false;
    }

    QTextCursor cursor(document());
    cursor.setPosition(docPos + 1);
    const QTextCharFormat format = cursor.charFormat();
    if (SMInlineToken::isChip(format) == false)
    {
        return false;
    }

    // Double-click de-renders the chip to its explicit, editable `#kind:name`; the next commit
    // re-chips. The explicit form (not the bare body) is shown so the user sees the kind.
    const QString edit = format.property(SMInlineToken::PropPrefix).toString()
                       + format.property(SMInlineToken::PropName).toString();
    cursor.setPosition(docPos);
    cursor.setPosition(docPos + 1, QTextCursor::KeepAnchor);
    cursor.insertText(edit, QTextCharFormat());
    setTextCursor(cursor);
    return true;
}

bool SMGuardField::revealChipAt(int docPos, const QPoint& viewportPos)
{
    if ((docPos < 0) || (document()->characterAt(docPos) != QChar::ObjectReplacementCharacter))
    {
        return false;
    }

    QTextCursor probe(document());
    probe.setPosition(docPos + 1);
    const QTextCharFormat format = probe.charFormat();
    if (SMInlineToken::isChip(format) == false)
    {
        return false;
    }

    // The reveal gesture is the leading glyph hot-zone only; a click on the name just places
    // the caret (a double-click there de-renders instead).
    QTextCursor at(document());
    at.setPosition(docPos);
    const QRect leftRect = cursorRect(at);
    const qreal glyphWidth = SMInlineToken::chipGlyphWidth(format, font());
    if (viewportPos.x() > (leftRect.left() + glyphWidth))
    {
        return false;
    }

    if (mHover != nullptr)
    {
        const QString name = format.property(SMInlineToken::PropName).toString();
        for (const SMGuardSymbol& sym : mCatalog)
        {
            if (sym.name == name)
            {
                const QPoint pos = mapToGlobal(QPoint(0, height() + 2));
                mHover->showSymbol(mModel, mTransitionId, sym, pos);
                return true;
            }
        }
    }

    return true;
}

bool SMGuardField::maybeOpenIslandAt(int docPos)
{
    if ((docPos < 0) || (document()->characterAt(docPos) != QChar::ObjectReplacementCharacter))
    {
        return false;
    }

    QTextCursor cursor(document());
    cursor.setPosition(docPos + 1);
    if (SMInlineToken::isIsland(cursor.charFormat()) == false)
    {
        return false;
    }

    const QList<int> positions = islandPositions();
    for (int i = 0; i < positions.size(); ++i)
    {
        if (positions.at(i) == docPos)
        {
            emit islandEditRequested(i, SMInlineToken::bodyOf(cursor.charFormat()));
            return true;
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////
// Notifications
//////////////////////////////////////////////////////////////////////////

void SMGuardField::onElementAdded(uint32_t /*id*/, eDocElementKind /*kind*/)
{
    buildCatalog();
    if (mSuppressAnalyze == false)
    {
        reprojectChips();   // a new symbol can introduce a same-name collision -> reveal a prefix
        analyze();
    }
}

void SMGuardField::onElementChanged(uint32_t id, eDocElementKind /*kind*/)
{
    buildCatalog();
    if (id == mTransitionId)
    {
        scheduleRebuild();
    }
    else if (mSuppressAnalyze == false)
    {
        // A foreign rename/retype: respell the chips in place but keep the user's text
        // reproject BEFORE analyze so the parse sees the new canonical chip bodies, not stale ones.
        reprojectChips();
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
            reprojectChips();
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
        if (mRebuildPending == false)
        {
            return;     // a reflowNow() already did this turn's work synchronously
        }

        mRebuildPending = false;
        rebuildFromModel();
    });
}

void SMGuardField::setAutoHeight(bool autoHeight)
{
    mAutoHeight = autoHeight;
    updateHeight();
}

void SMGuardField::setHeightLines(int minLines, int maxLines)
{
    // The pop-out is a big editor by definition; the docked field stays modest. Both use the same
    // grow-with-content rule, only the bounds differ.
    mMinLines = qMax(1, minLines);
    mMaxLines = qMax(mMinLines, maxLines);
    updateHeight();
}

void SMGuardField::reflowNow()
{
    // The click-to-insert path must show its chip in the SAME turn as the gesture:
    // waiting for the deferred rebuild made a double-click feel unresponsive. Safe to
    // run synchronously here because the caller is a plain widget signal, not a notifier slot.
    mDebounce->stop();
    mRebuildPending = false;
    rebuildFromModel();
}

void SMGuardField::rebuildFromModel()
{
    clearSlotMode();

    SMTransitionEntry* transition = (mTransitionId != 0u) ? mModel.getData().findTransitionById(mTransitionId) : nullptr;

    QString text;
    QList<SMGuardRender::Chip> chips;
    mAllowRaw = false;
    if (transition != nullptr)
    {
        const SMGuard& guard = transition->getGuard();
        if (guard.isOk() && (guard.getTree() != nullptr))
        {
            // A committed tree reflows to its canonical text AND its chips: every
            // bound reference folds to a compact pill. A draft keeps the user's raw text as-is.
            const SMGuardRender::Rendered rendered = SMGuardRender::render(mModel.getData(), mTransitionId, *guard.getTree());
            text  = rendered.text;
            chips = rendered.chips;
        }
        else
        {
            text = SMGuardRender::guardText(mModel.getData(), mTransitionId, guard);
        }
        mAllowRaw = (guard.isDraft() && (countRawNodes(guard.getTree()) > 0))
                     || (guard.isOk() && (countRawNodes(guard.getTree()) > 0));
    }

    mSuppressAnalyze = true;
    setPlainText(text);
    mSuppressAnalyze = false;
    // Chips fold FIRST (back-to-front, so earlier offsets stay valid); island `{...}` blocks
    // are then found by a fresh brace scan over the shortened text.
    foldChips(chips);
    foldIslands();

    mSuppressAnalyze = true;
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
    mLastCursorPos = cursor.position();
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

    // Applying highlighter formats re-emits QTextEdit::textChanged (Qt fires it for format-only
    // edits too). The textChanged handler restarts mDebounce, whose timeout runs analyze() again,
    // which re-applies the formats -- a self-sustaining 150 ms loop that keeps the process off
    // idle. Shield every decoration write with mSuppressAnalyze (the same guard used around
    // setPlainText / foldChips / reprojectChips) so our own formatting never re-triggers analyze.
    auto applyDecorations = [this](const QList<SMGuardHighlighter::OwnerSpan>& owners, const QList<SMGuardHighlighter::DiagSpan>& diags)
    {
        const bool prevSuppress = mSuppressAnalyze;
        mSuppressAnalyze = true;
        mHighlighter->setDecorations(owners, diags);
        mSuppressAnalyze = prevSuppress;
    };

    if (mTransitionId == 0u)
    {
        applyDecorations({}, {});
        emit statusUpdated(static_cast<int>(NEGuardStyle::eSeverity::Ok), QString(), QString(), {});
        emit fixesUpdated(QString(), {});
        emit badgeUpdated(false, false);
        return;
    }

    const StateMachineData& data = mModel.getData();
    const QString docText = toPlainText();
    QList<int> docStarts;
    const QString expText = expandedText(docStarts);

    // An unmapped formal renders as a ghost `<name>` -- a DISPLAY marker, never guard syntax. Feeding
    // it to the parser produced a bogus `unexpected '<'` and demoted a perfectly good guard to a
    // draft. Neutralize each ghost with an EQUAL-LENGTH filler so every
    // diagnostic offset still maps back to the document, and report the real problem ourselves.
    QStringList ghostNames;
    const QString parseText = maskGhosts(expText, ghostNames);

    // The parser sees the masked, expanded text (islands as `{...}`); its diagnostic offsets are
    // remapped back to document positions for the underlines.
    SMGuardParser::Result result = SMGuardParser::parse(data, mTransitionId, parseText, mAllowRaw);

    // ---- decorations ------------------------------------------------------
    QList<SMGuardHighlighter::OwnerSpan> owners;
    QList<SMGuardHighlighter::DiagSpan> diags;

    // Owner colors + raw dotted underline from a live token scan over the DOCUMENT text
    // (island tokens paint themselves and are skipped as non-word characters).
    int i = 0;
    const int n = docText.length();
    while (i < n)
    {
        const QChar ch = docText.at(i);
        if (ch == QLatin1Char('<'))
        {
            // Skip an unfilled slot placeholder so it is not flagged as an unknown name.
            const QRegularExpressionMatch m = slotPattern().match(docText, i);
            if (m.hasMatch() && (m.capturedStart() == i))
            {
                i = m.capturedEnd();
                continue;
            }
        }

        if (ch.isDigit())
        {
            const int start = i;
            while ((i < n) && (docText.at(i).isLetterOrNumber() || (docText.at(i) == QLatin1Char('.')) || (docText.at(i) == QLatin1Char('x'))))
            {
                ++i;
            }
            owners.append({ start, i - start, NEGuardStyle::eOwner::Literal });
            continue;
        }

        if (ch.isLetter() || (ch == QLatin1Char('_')))
        {
            const int start = i;
            while ((i < n) && isWordChar(docText.at(i)))
            {
                ++i;
            }

            const QString word = docText.mid(start, i - start);
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

    // Diagnostic underlines from the parser (authoritative), remapped to document offsets.
    for (const SMGuardParser::Diagnostic& diag : result.diagnostics)
    {
        const int docFrom = expandedToDoc(diag.start);
        const int docTo = expandedToDoc(diag.start + diag.length);
        const int docLen = qMax(1, docTo - docFrom);
        if (diag.severity == SMGuardParser::eSeverity::Error)
        {
            diags.append({ docFrom, docLen, SMGuardHighlighter::eUnderline::Error });
            if (mErrorStart < 0)
            {
                mErrorStart = docFrom;
                mErrorLength = docLen;
            }
        }
        else
        {
            diags.append({ docFrom, docLen, SMGuardHighlighter::eUnderline::Warning });
        }
    }

    applyDecorations(owners, diags);

    // ---- status + fixes ---------------------------------------------------
    if (expText.trimmed().isEmpty())
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

    const bool hasCalls = hasCallNode(result.tree);

    SMGuard guard;
    if (result.resolved())
    {
        guard.setTree(result.tree);
        result.tree = nullptr;
    }
    else
    {
        guard.setDraft(expText);
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

        if (ghostNames.isEmpty() == false)
        {
            // The guard itself is sound -- what is missing is an argument. Say exactly that, and
            // name the formal, instead of leaking a parser token the user never typed.
            emit statusUpdated(static_cast<int>(NEGuardStyle::eSeverity::Err)
                              , unmappedMessage(ghostNames), preview, chips);
            emit fixesUpdated(QString(), {});
            emit badgeUpdated(true, false);
            return;
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
                    const QString name = expText.mid(diag.start, diag.length);
                    fixes.append({ QStringLiteral("shadow-getter"), tr("Use the attribute: %1()").arg(name), name, true, QString() });
                    fixes.append({ QStringLiteral("shadow-rename"), tr("Rename attribute..."), name, true, QString() });
                    fixes.append({ QStringLiteral("suppress"), tr("Dismiss"), name, true, QString() });
                    break;
                }
            }

            if (hasCalls)
            {
                fixes.append({ QStringLiteral("map"), tr("Map arguments..."), QString(), true, QString() });
            }

            mLastMessage = tr("(!) %1").arg(firstWarning);
            mLastFixes = fixes;
            emit fixesUpdated(mLastMessage, fixes);
        }
        else
        {
            // Quiet when ok: the bar stays hidden, but Ctrl+. still offers the grid for
            // guards that call a condition (the fix-bar route).
            mLastFixes.clear();
            if (hasCalls)
            {
                mLastMessage = tr("map call arguments");
                mLastFixes.append({ QStringLiteral("map"), tr("Map arguments..."), QString(), true, QString() });
            }

            emit fixesUpdated(QString(), {});
        }

        emit badgeUpdated(false, hasWarning);
    }
    else
    {
        const QString token = (mErrorStart >= 0) ? docText.mid(mErrorStart, mErrorLength) : QString();
        QString message = firstError.isEmpty() ? tr("unresolved guard") : firstError;
        if (ghostNames.isEmpty() == false)
        {
            message = unmappedMessage(ghostNames);
        }

        emit statusUpdated(static_cast<int>(NEGuardStyle::eSeverity::Err), message, QString(), {});

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
            fixes.append({ QStringLiteral("map"), tr("Map arguments..."), QString(), hasCalls, hasCalls ? QString() : tr("no resolved call in the guard") });
        }

        mLastMessage = tr("(!) %1").arg(message);
        mLastFixes = fixes;
        emit fixesUpdated(mLastMessage, fixes);
        emit badgeUpdated(true, false);
    }
}

//////////////////////////////////////////////////////////////////////////
// Commit gate
//////////////////////////////////////////////////////////////////////////

QString SMGuardField::committableText() const
{
    // Cleanup runs on the document text (tokens are single characters there), THEN the tokens
    // expand -- an island to `{body}`, a chip to its canonical `#kind:name` (12.7: no chip
    // label or ghost marker ever reaches the stored guard) -- so a multi-line island body and
    // a folded reference both survive byte-exact.
    QStringList replacements;   // one per ORC in document order (island or chip)
    QTextCursor cursor(document());
    const QString doc = toPlainText();
    for (int i = 0; i < doc.length(); ++i)
    {
        if (doc.at(i) == QChar::ObjectReplacementCharacter)
        {
            cursor.setPosition(i + 1);
            const QTextCharFormat format = cursor.charFormat();
            if (SMInlineToken::isIsland(format))
            {
                replacements.append(QLatin1Char('{') + SMInlineToken::bodyOf(format) + QLatin1Char('}'));
            }
            else if (SMInlineToken::isChip(format))
            {
                replacements.append(SMInlineToken::bodyOf(format));
            }
            else
            {
                replacements.append(QString());     // a stray replacement char -> drop it
            }
        }
    }

    QString text = doc;
    text.replace(QLatin1Char('\n'), QLatin1Char(' '));
    text.remove(slotPattern());                                     // never commit slot markers
    text.replace(QRegularExpression(QStringLiteral(",\\s*\\)")), QStringLiteral(")"));
    text.replace(QRegularExpression(QStringLiteral("\\(\\s*,")), QStringLiteral("("));
    text.replace(QRegularExpression(QStringLiteral(",\\s*,")), QStringLiteral(","));

    for (const QString& replacement : replacements)
    {
        const int at = text.indexOf(QChar::ObjectReplacementCharacter);
        if (at < 0)
        {
            break;
        }

        text.replace(at, 1, replacement);
    }

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
    // Esc discards the in-progress edit and restores the committed guard. Reflowing from the
    // model (rather than the cached text) re-folds the chips too, so the reverted state looks
    // exactly like a fresh commit.
    mCompleter->hide();
    rebuildFromModel();
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
    else if (id == QStringLiteral("map"))
    {
        emit mapArgumentsRequested();
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
        // Rename lives on the Attributes page; the editor surfaces the fix but defers the dialog.
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
// Raw-collision bind
//////////////////////////////////////////////////////////////////////////

void SMGuardField::applyRawBind(const QList<int>& rawPath, const SMGuardSymbol& symbol)
{
    if (mTransitionId == 0u)
    {
        return;
    }

    SMGuardNode* node = nullptr;
    if (symbol.refkind == SMGuardSymbol::eRefKind::Cond)
    {
        // A condition is always a call in the guard grammar (`name()`); any formals fall to
        // ghosts, exactly as typing `@cond:name()` would.
        node = SMGuardNode::makeCall(symbol.symbolId, QList<SMGuardNode*>());
    }
    else
    {
        SMGuardNode::eKind kind = SMGuardNode::eKind::Attr;
        switch (symbol.refkind)
        {
        case SMGuardSymbol::eRefKind::Param: kind = SMGuardNode::eKind::Param; break;
        case SMGuardSymbol::eRefKind::Const: kind = SMGuardNode::eKind::Const; break;
        case SMGuardSymbol::eRefKind::Attr:
        default:                             kind = SMGuardNode::eKind::Attr;  break;
        }

        node = SMGuardNode::makeRef(kind, symbol.symbolId);
    }

    // One undo step; the command notifies elementChanged(transitionId) and the field reflows the
    // committed tree (the raw text becomes a bound chip). Never a text edit -- the exact node at
    // rawPath is swapped, so a same-name raw fragment elsewhere is untouched.
    StateMachineData& data = mModel.getData();
    SMSetGuardCommand* command = SMGuardCommands::replaceSubtree(data, mModel.getNotifier(), mTransitionId, rawPath, node, tr("Bind '%1'").arg(symbol.name));
    if (command != nullptr)
    {
        mModel.getUndoStack().push(command);
    }
}

bool SMGuardField::rawNodeSpan(const QList<int>& rawPath, int& start, int& length) const
{
    if (mTransitionId == 0u)
    {
        return false;
    }

    const SMTransitionEntry* transition = mModel.getData().findTransitionById(mTransitionId);
    if ((transition == nullptr) || (transition->getGuard().isOk() == false))
    {
        return false;
    }

    const SMGuardNode* root = transition->getGuard().getTree();
    if (root == nullptr)
    {
        return false;
    }

    const QList<SMGuardRender::NodeSpan> spans = SMGuardRender::nodeSpans(mModel.getData(), mTransitionId, *root);
    for (const SMGuardRender::NodeSpan& span : spans)
    {
        if (span.path == rawPath)
        {
            start = span.start;
            length = span.length;
            return true;
        }
    }

    return false;
}

void SMGuardField::bindRaw(const QList<int>& rawPath, const QString& name)
{
    if (mTransitionId == 0u)
    {
        return;
    }

    // Re-resolve against the CURRENT catalog: the collision may already be gone (a rename removed
    // it), or the name may now match several kinds. Never auto-bind on ambiguity -- defer to the
    // picker so the developer states the kind.
    QList<SMGuardSymbol> matches;
    for (const SMGuardSymbol& sym : mCatalog)
    {
        if (sym.name == name)
        {
            matches.append(sym);
        }
    }

    if (matches.isEmpty())
    {
        return;     // the collision vanished before the click resolved -- nothing to bind.
    }

    if (matches.size() == 1)
    {
        applyRawBind(rawPath, matches.first());
        return;
    }

    // Multi-kind: open the same reference completer used for bare typing, filtered to this name,
    // and bind on the pick (onCompleterAccepted routes through mRawBindMode). Selecting the raw
    // token first shows the developer exactly what will be bound.
    int spanStart = 0;
    int spanLength = 0;
    if (rawNodeSpan(rawPath, spanStart, spanLength))
    {
        selectSpan(spanStart, spanLength);
    }
    else
    {
        setFocus();
    }

    QSet<SMGuardSymbol::eRefKind> kinds;
    for (const SMGuardSymbol& sym : matches)
    {
        kinds.insert(sym.refkind);
    }

    mRawBindMode = true;
    mRawBindPath = rawPath;

    const QRect rect = cursorRect();
    const QRect caretGlobal(viewport()->mapToGlobal(rect.topLeft()), rect.size());
    mCompleter->showFor(kinds, name, caretGlobal);
}

//////////////////////////////////////////////////////////////////////////
// Completion
//////////////////////////////////////////////////////////////////////////

namespace
{
    //!< Maps a `#kind` word to its reference kind; false when the word is not a valid kind.
    bool kindFromWord(const QString& word, SMGuardSymbol::eRefKind& kind)
    {
        if (word == QStringLiteral("param")) { kind = SMGuardSymbol::eRefKind::Param; return true; }
        if (word == QStringLiteral("attr"))  { kind = SMGuardSymbol::eRefKind::Attr;  return true; }
        if (word == QStringLiteral("const")) { kind = SMGuardSymbol::eRefKind::Const; return true; }
        if (word == QStringLiteral("cond"))  { kind = SMGuardSymbol::eRefKind::Cond;  return true; }
        return false;
    }
}

void SMGuardField::openCompletion()
{
    if (mTransitionId == 0u)
    {
        return;
    }

    // Ctrl+Space is a deliberate open: show every kind at the caret and clear any Esc dismissal.
    mCompleterDismissedAt = -1;
    mRawBindMode = false;       // an explicit completion open is never a raw-bind pick.
    const QRect rect = cursorRect();
    const QRect caretGlobal(viewport()->mapToGlobal(rect.topLeft()), rect.size());
    mCompleter->showFor({}, QString(), caretGlobal);
}

void SMGuardField::insertReference(const SMGuardSymbol& symbol)
{
    if (mTransitionId == 0u)
    {
        return;
    }

    // The Data-catalog / Insert path: insert the canonical reference at the plain caret. This is
    // the tail of onCompleterAccepted without a mention to remove, so a clicked insert and a
    // typed reference commit the identical tree (P3).
    setFocus();
    mCompleter->hide();
    mCompleterDismissedAt = -1;

    QTextCursor cursor = textCursor();
    if (symbol.isCall)
    {
        cursor.insertText(symbol.mention() + QStringLiteral("()"));
        cursor.movePosition(QTextCursor::Left);     // caret between the parens
        setTextCursor(cursor);
        updateSignatureHelp();                      // the developer fills the arguments, then commits
        return;
    }

    cursor.insertText(symbol.mention());
    setTextCursor(cursor);
    commit();
}

bool SMGuardField::caretCallee(QString& name) const
{
    int argIndex = 0;
    return callContextAtCaret(name, argIndex);
}

bool SMGuardField::mentionUnderCursor(int& atPos, QString& kindWord, QString& namePart, bool& hasColon) const
{
    const QString text = toPlainText();
    const int caret = textCursor().position();
    int i = caret;
    while (i > 0)
    {
        const QChar c = text.at(i - 1);
        if (c == NEGuardText::RefSigil) { break; }
        if (isWordChar(c) || (c == QLatin1Char(':'))) { --i; continue; }
        return false;   // a non-mention character before any sigil -> not inside a mention
    }

    if ((i <= 0) || (text.at(i - 1) != NEGuardText::RefSigil))
    {
        return false;
    }

    atPos = i - 1;
    const QString body = text.mid(i, caret - i);
    const int colon = body.indexOf(QLatin1Char(':'));
    hasColon = (colon >= 0);
    if (hasColon)
    {
        kindWord = body.left(colon);
        namePart = body.mid(colon + 1);
        if (namePart.contains(QLatin1Char(':'))) { return false; }  // more than one ':' -> malformed
    }
    else
    {
        kindWord = QString();
        namePart = body;
    }

    return true;
}

void SMGuardField::updateCompleter()
{
    // Any text edit reevaluates the mention-driven completer, so we are no longer in the
    // standalone pick opened by bindRaw -- clear the one-shot before it could misroute an
    // ordinary completion into a raw bind.
    mRawBindMode = false;

    if (mTransitionId == 0u)
    {
        mCompleter->hide();
        return;
    }

    int atPos = 0;
    QString kindWord;
    QString namePart;
    bool hasColon = false;
    if (mentionUnderCursor(atPos, kindWord, namePart, hasColon) == false)
    {
        mCompleter->hide();
        mCompleterDismissedAt = -1;      // left the mention; a future sigil may reopen
        return;
    }

    // Once dismissed, this exact mention stays closed until a fresh sigil (a new atPos).
    if (atPos == mCompleterDismissedAt)
    {
        mCompleter->hide();
        return;
    }
    mCompleterDismissedAt = -1;

    QSet<SMGuardSymbol::eRefKind> kinds;
    if (hasColon)
    {
        SMGuardSymbol::eRefKind kind;
        if (kindFromWord(kindWord, kind)) { kinds.insert(kind); }
    }

    const QRect rect = cursorRect();
    const QRect caretGlobal(viewport()->mapToGlobal(rect.topLeft()), rect.size());
    mCompleter->showFor(kinds, namePart, caretGlobal);

    // The completer opens OVER the signature-help tooltip; the tooltip returns
    // when the completer closes (see updateSignatureHelp).
    if (mCompleter->isVisible() && (mSignature != nullptr))
    {
        mSignature->hide();
    }
}

bool SMGuardField::callContextAtCaret(QString& calleeName, int& argIndex) const
{
    // Scan the EXPANDED text (chips -> `#kind:name`, islands -> `{body}`) so the callee is
    // always plain text; the caret's document position maps to its expanded offset.
    QList<int> docStarts;
    const QString exp = expandedText(docStarts);
    const int caretDoc = textCursor().position();
    if ((caretDoc < 0) || (caretDoc >= docStarts.size()))
    {
        return false;
    }

    const int caret = docStarts.at(caretDoc);
    int depth = 0;
    int commas = 0;
    int i = caret;
    while (i > 0)
    {
        const QChar c = exp.at(i - 1);
        if (c == QLatin1Char(')')) { ++depth; }
        else if (c == QLatin1Char('('))
        {
            if (depth == 0) { break; }      // the opening paren of the enclosing call
            --depth;
        }
        else if ((c == QLatin1Char(',')) && (depth == 0)) { ++commas; }
        --i;
    }

    if ((i <= 0) || (exp.at(i - 1) != QLatin1Char('(')))
    {
        return false;   // the caret is not inside a call's parentheses
    }

    // The callee is the `#cond:name` / bare identifier immediately before the '('.
    int nameEnd = i - 1;
    int nameStart = nameEnd;
    while ((nameStart > 0)
        && (isWordChar(exp.at(nameStart - 1)) || (exp.at(nameStart - 1) == QLatin1Char(':')) || (exp.at(nameStart - 1) == NEGuardText::RefSigil)))
    {
        --nameStart;
    }

    QString callee = exp.mid(nameStart, nameEnd - nameStart);
    if (callee.startsWith(NEGuardText::RefSigil))
    {
        const int colon = callee.indexOf(QLatin1Char(':'));
        if (colon >= 0) { callee = callee.mid(colon + 1); }
    }
    if (callee.isEmpty())
    {
        return false;
    }

    calleeName = callee;
    argIndex   = commas;
    return true;
}

void SMGuardField::updateSignatureHelp()
{
    if (mSignature == nullptr)
    {
        return;
    }

    // The completer owns the surface while it is open; the two never overlap.
    if (mCompleter->isVisible())
    {
        mSignature->hide();
        return;
    }

    QString callee;
    int argIndex = 0;
    if (callContextAtCaret(callee, argIndex) == false)
    {
        mSignature->hide();
        return;
    }

    for (const SMGuardSymbol& sym : mCatalog)
    {
        if (sym.isCall && (sym.name == callee))
        {
            const QRect rect = cursorRect();
            const QPoint pos = viewport()->mapToGlobal(QPoint(rect.left(), rect.bottom() + 4));
            mSignature->showFor(sym, argIndex, pos);
            return;
        }
    }

    mSignature->hide();
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

void SMGuardField::onCompleterAccepted(const SMGuardSymbol& symbol)
{
    // Multi-kind pick: the picker was opened standalone by bindRaw (no `@`-mention
    // in the text), so a pick binds the Raw node at mRawBindPath through the model, not by text
    // surgery. Consume the one-shot mode first so a later ordinary completion is unaffected.
    if (mRawBindMode)
    {
        mRawBindMode = false;
        const QList<int> rawPath = mRawBindPath;
        mRawBindPath.clear();
        mCompleter->hide();
        applyRawBind(rawPath, symbol);
        return;
    }

    // Replace the `@[kind][:][name]` mention under the caret with the picked symbol's canonical
    // `#kind:name` (typed == clicked, P3). A call inserts `#cond:name()` with the caret between
    // the parens; the developer types the arguments inline (signature help arrives
    // phase 5), and the Arguments table projects them.
    int atPos = 0;
    QString kindWord;
    QString namePart;
    bool hasColon = false;
    if (mentionUnderCursor(atPos, kindWord, namePart, hasColon) == false)
    {
        return;
    }

    const int caret = textCursor().position();
    QTextCursor cursor = textCursor();
    cursor.setPosition(atPos);
    cursor.setPosition(caret, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();

    mCompleter->hide();
    mCompleterDismissedAt = -1;

    if (symbol.isCall)
    {
        cursor.insertText(symbol.mention() + QStringLiteral("()"));
        cursor.movePosition(QTextCursor::Left);     // caret between the parens
        setTextCursor(cursor);
        updateSignatureHelp();                      // show the parameter info for the first argument
        return;                                     // the developer fills the arguments, then commits
    }

    cursor.insertText(symbol.mention());
    setTextCursor(cursor);
    commit();
}

//////////////////////////////////////////////////////////////////////////
// Mapping slots
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

bool SMGuardField::selectFirstGhost()
{
    // After a reflow the caret sits at the end of the guard, which is the wrong place to leave the
    // developer when the condition they just inserted still has unmapped formals: the very next
    // thing they want to do is fill the first one. The ghost `<name>` is
    // display-only text, so SELECTING it means the first keystroke replaces it.
    const QRegularExpressionMatch match = slotPattern().match(toPlainText());
    if (match.hasMatch() == false)
    {
        return false;
    }

    QTextCursor cursor = textCursor();
    cursor.setPosition(match.capturedStart());
    cursor.setPosition(match.capturedEnd(), QTextCursor::KeepAnchor);
    setTextCursor(cursor);
    return true;
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
    for (int i = static_cast<int>(tints.size()) - 1; i >= 0; --i)
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
    // Three lines minimum, up to twelve: the Conditions tab gained room when the advisory strips and
    // the Try-it evaluator went away, and a guard the developer can break over several lines
    // (Shift+Enter) needs somewhere to put them.
    const int minHeight = (lineHeight * mMinLines) + frame;
    if (mAutoHeight == false)
    {
        // The pop-out owns its own geometry: the field fills whatever the dialog gives it instead
        // of pinning itself to its content.
        setMinimumHeight(minHeight);
        setMaximumHeight(QWIDGETSIZE_MAX);
        return;
    }

    const int maxHeight = (lineHeight * mMaxLines) + frame;
    setFixedHeight(qBound(minHeight, static_cast<int>(docHeight) + frame, maxHeight));
}

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////

void SMGuardField::keyPressEvent(QKeyEvent* event)
{
    if (mCompleter->isVisible())
    {
        switch (event->key())
        {
        case Qt::Key_Up:        mCompleter->moveCurrent(-1);    return;
        case Qt::Key_Down:      mCompleter->moveCurrent(1);     return;
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Tab:       mCompleter->acceptCurrent();    return;
        case Qt::Key_Escape:
            {
                // Close but KEEP the typed characters; do NOT reopen for this same
                // mention on the next keystroke -- only a fresh sigil (a new position) reopens.
                int atPos = 0;
                QString kindWord;
                QString namePart;
                bool hasColon = false;
                mCompleterDismissedAt = mentionUnderCursor(atPos, kindWord, namePart, hasColon) ? atPos : -1;
                mRawBindMode = false;   // Dismissing the picker leaves the raw node untouched.
                mCompleter->hide();
                updateSignatureHelp();  // The tooltip returns when the completer closes
            }
            return;
        default:
            mRawBindMode = false;       // Typing over the selection exits the multi-kind picker.
            break;      // PASS-THROUGH: the key reaches the document AND re-filters the list
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
        if (event->modifiers() & Qt::ShiftModifier)
        {
            // Shift+Enter breaks a long guard over several lines while it is being written
            // . The break is an authoring aid only: committableText
            // folds the lines back into the one-line canonical guard the tree stores.
            textCursor().insertText(QStringLiteral("\n"));
            updateHeight();
            return;
        }

        commit();
        return;
    }

    if (event->key() == Qt::Key_Escape)
    {
        // The Esc chain: an open card/slot session closes first; the next Esc reverts.
        if (mSlotMode || ((mSignature != nullptr) && mSignature->isVisible()))
        {
            if (mSignature != nullptr)
            {
                mSignature->hide();
            }

            clearSlotMode();
            return;
        }

        revert();
        return;
    }

    // Typing `{` in an operand position creates an island and opens its editor.
    if (event->text() == QStringLiteral("{"))
    {
        const int index = insertIslandAtCaret(QString());
        emit islandEditRequested(index, QString());
        return;
    }

    QTextEdit::keyPressEvent(event);
}

void SMGuardField::focusOutEvent(QFocusEvent* event)
{
    mRawBindMode = false;       // A blur abandons the open multi-kind picker.
    mCompleter->hide();
    if (mSignature != nullptr)
    {
        mSignature->hide();
    }

    // The pop-out editor disables auto-commit so its Cancel button (which blurs the field before
    // its slot runs) cannot commit the discarded text; it commits explicitly on OK instead.
    if (mAutoCommit)
    {
        commit();
    }

    QTextEdit::focusOutEvent(event);
}

void SMGuardField::mouseReleaseEvent(QMouseEvent* event)
{
    QTextEdit::mouseReleaseEvent(event);

    const QTextCursor hit = cursorForPosition(event->pos());

    // A click on a chip's leading glyph hot-zone reveals it; a click elsewhere on
    // the chip falls through to normal caret placement (a double-click there de-renders it).
    if (revealChipAt(hit.position(), event->pos()) || revealChipAt(hit.position() - 1, event->pos()))
    {
        return;
    }

    // A click on an island token opens its editor.
    if (maybeOpenIslandAt(hit.position()) == false)
    {
        maybeOpenIslandAt(hit.position() - 1);
    }
}

void SMGuardField::mouseDoubleClickEvent(QMouseEvent* event)
{
    // Double-click a chip -> de-render to editable `#kind:name`: a chip IS one
    // token, so a double-click that would select a word selects the whole chip instead.
    const QTextCursor hit = cursorForPosition(event->pos());
    if (deRenderChipAt(hit.position()) || deRenderChipAt(hit.position() - 1))
    {
        return;
    }

    QTextEdit::mouseDoubleClickEvent(event);
}

void SMGuardField::mouseMoveEvent(QMouseEvent* event)
{
    QTextEdit::mouseMoveEvent(event);

    if (mHover == nullptr)
    {
        return;
    }

    // Hover a known symbol word for 300 ms -> the symbol card.
    const QTextCursor hit = cursorForPosition(event->pos());
    const QString text = toPlainText();
    int left = hit.position();
    while ((left > 0) && (left <= text.length()) && isWordChar(text.at(left - 1)))
    {
        --left;
    }

    int right = hit.position();
    while ((right < text.length()) && isWordChar(text.at(right)))
    {
        ++right;
    }

    const QString word = (right > left) ? text.mid(left, right - left) : QString();
    if (word == mHoverWord)
    {
        return;
    }

    mHoverWord = word;
    mHoverTimer->stop();
    if ((word.isEmpty() == false) && mOwnerByName.contains(word))
    {
        mHoverTimer->start();
    }
    else
    {
        mHover->scheduleHide();
    }
}

void SMGuardField::leaveEvent(QEvent* event)
{
    mHoverTimer->stop();
    mHoverWord.clear();
    if (mHover != nullptr)
    {
        mHover->scheduleHide();
    }

    QTextEdit::leaveEvent(event);
}

QMimeData* SMGuardField::createMimeDataFromSelection() const
{
    // The clipboard receives the EXPANDED text: an island copies as `{body}` byte-exact
    // (acceptance 23a), so a paste anywhere -- including outside Lusan -- keeps the code.
    const QTextCursor cursor = textCursor();
    const int from = cursor.selectionStart();
    const int to = cursor.selectionEnd();

    const QString doc = toPlainText();
    QString out;
    QTextCursor probe(document());
    for (int i = from; (i < to) && (i < doc.length()); ++i)
    {
        if (doc.at(i) == QChar::ObjectReplacementCharacter)
        {
            probe.setPosition(i + 1);
            const QTextCharFormat format = probe.charFormat();
            if (SMInlineToken::isIsland(format))
            {
                out += QLatin1Char('{') + SMInlineToken::bodyOf(format) + QLatin1Char('}');
                continue;
            }
            if (SMInlineToken::isChip(format))
            {
                out += SMInlineToken::bodyOf(format);   // the canonical #kind:name / bare name
                continue;
            }
        }

        out += doc.at(i);
    }

    QMimeData* mime = new QMimeData();
    mime->setText(out);
    return mime;
}

void SMGuardField::insertFromMimeData(const QMimeData* source)
{
    if (source->hasText() == false)
    {
        return;
    }

    // Balanced `{...}` blocks re-enter as island tokens (their bodies byte-exact,
    // newlines preserved); everything between them is inserted as single-line text.
    const QString text = source->text();
    const int n = text.length();
    int i = 0;
    int plainStart = 0;
    auto flushPlain = [this, &text](int from, int to)
    {
        if (to <= from)
        {
            return;
        }

        QString plain = text.mid(from, to - from);
        plain.replace(QLatin1Char('\n'), QLatin1Char(' '));
        plain.replace(QLatin1Char('\r'), QString());
        insertPlainText(plain);
    };

    while (i < n)
    {
        if (text.at(i) == QLatin1Char('{'))
        {
            const int end = scanBrace(text, i, n);
            flushPlain(plainStart, i);
            const QString body = text.mid(i + 1, qMax(0, end - i - 2));
            insertIslandAtCaret(body);
            i = end;
            plainStart = end;
            continue;
        }

        ++i;
    }

    flushPlain(plainStart, n);
}

void SMGuardField::onDebounce()
{
    analyze();
}
