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
#include "lusan/view/sm/SMHoverCard.hpp"
#include "lusan/view/sm/SMInlineToken.hpp"
#include "lusan/view/sm/SMSignatureCard.hpp"
#include "lusan/view/sm/SMSymbolPopup.hpp"

#include <QAbstractTextDocumentLayout>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QRegularExpression>
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
    , mSuppressAnalyze  (false)
    , mHighlighter      (nullptr)
    , mPopup            (nullptr)
    , mSignature        (nullptr)
    , mDebounce         (nullptr)
    , mSlotMode         (false)
    , mCurrentSlot      (-1)
    , mErrorStart       (-1)
    , mErrorLength      (0)
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
    setPlaceholderText(tr("type a condition -- or press Ctrl+Space to see everything you can use"));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMouseTracking(true);

    // The island token painter (E4): registered on the rich-text layout, which is the
    // reason this field derives QTextEdit (QPlainTextDocumentLayout never paints these).
    mTokenHandler = new SMInlineToken(this);
    document()->documentLayout()->registerHandler(SMInlineToken::IslandType, mTokenHandler);

    mHighlighter = new SMGuardHighlighter(document());
    mPopup = new SMSymbolPopup(this);
    mSignature = new SMSignatureCard(this);

    mDebounce = new QTimer(this);
    mDebounce->setSingleShot(true);
    mDebounce->setInterval(150);

    mHoverTimer = new QTimer(this);
    mHoverTimer->setSingleShot(true);
    mHoverTimer->setInterval(300);

    connect(mDebounce, &QTimer::timeout, this, &SMGuardField::onDebounce);
    connect(mPopup, &SMSymbolPopup::accepted, this, &SMGuardField::onCompletionAccepted);
    connect(mPopup, &SMSymbolPopup::newLambdaRequested, this, [this]()
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
        updateCompletion();
    });
    connect(this, &QTextEdit::cursorPositionChanged, this, [this]()
    {
        // Caret-entry opens the island editor (B8): crossing exactly one character that
        // is a token counts as entering it.
        const int pos = textCursor().position();
        const int old = mLastCursorPos;
        mLastCursorPos = pos;
        if (qAbs(pos - old) == 1)
        {
            maybeOpenIslandAt(qMin(pos, old));
        }
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

    mPopup->setSymbols(mCatalog);
}

void SMGuardField::commitNow()
{
    commit();
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
// Islands (E4)
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
            if (SMInlineToken::isIsland(cursor.charFormat()))
            {
                out += QLatin1Char('{') + SMInlineToken::bodyOf(cursor.charFormat()) + QLatin1Char('}');
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
    mSuppressAnalyze = false;
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

    if (mTransitionId == 0u)
    {
        mHighlighter->setDecorations({}, {});
        emit statusUpdated(static_cast<int>(NEGuardStyle::eSeverity::Ok), QString(), QString(), {});
        emit fixesUpdated(QString(), {});
        emit badgeUpdated(false, false);
        return;
    }

    const StateMachineData& data = mModel.getData();
    const QString docText = toPlainText();
    QList<int> docStarts;
    const QString expText = expandedText(docStarts);

    // The parser sees the expanded text (islands as `{...}`); its diagnostic offsets are
    // remapped back to document positions for the underlines.
    SMGuardParser::Result result = SMGuardParser::parse(data, mTransitionId, expText, mAllowRaw);

    // ---- decorations -----------------------------------------------------
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

    mHighlighter->setDecorations(owners, diags);

    // ---- status + fixes --------------------------------------------------
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
            // guards that call a condition (the B6 fix-bar route).
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
            fixes.append({ QStringLiteral("map"), tr("Map arguments..."), QString(), hasCalls, hasCalls ? QString() : tr("no resolved call in the guard") });
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
    // Cleanup runs on the document text (tokens are single characters there), THEN the
    // tokens expand to `{body}` -- so a multi-line island body survives byte-exact.
    QStringList bodies;
    QTextCursor cursor(document());
    const QString doc = toPlainText();
    for (int i = 0; i < doc.length(); ++i)
    {
        if (doc.at(i) == QChar::ObjectReplacementCharacter)
        {
            cursor.setPosition(i + 1);
            if (SMInlineToken::isIsland(cursor.charFormat()))
            {
                bodies.append(SMInlineToken::bodyOf(cursor.charFormat()));
            }
        }
    }

    QString text = doc;
    text.replace(QLatin1Char('\n'), QLatin1Char(' '));
    text.remove(slotPattern());                                     // never commit slot markers
    text.replace(QRegularExpression(QStringLiteral(",\\s*\\)")), QStringLiteral(")"));
    text.replace(QRegularExpression(QStringLiteral("\\(\\s*,")), QStringLiteral("("));
    text.replace(QRegularExpression(QStringLiteral(",\\s*,")), QStringLiteral(","));

    for (const QString& body : bodies)
    {
        const int at = text.indexOf(QChar::ObjectReplacementCharacter);
        if (at < 0)
        {
            break;
        }

        text.replace(at, 1, QLatin1Char('{') + body + QLatin1Char('}'));
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
    clearSlotMode();
    mSuppressAnalyze = true;
    setPlainText(mCommittedText);
    mSuppressAnalyze = false;
    foldIslands();

    mSuppressAnalyze = true;
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
    mLastCursorPos = cursor.position();
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

    // Typing `{` in an operand position creates an island (E4) and opens its editor.
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
    mPopup->hide();
    if (mSignature != nullptr)
    {
        mSignature->hide();
    }

    commit();
    QTextEdit::focusOutEvent(event);
}

void SMGuardField::mouseReleaseEvent(QMouseEvent* event)
{
    QTextEdit::mouseReleaseEvent(event);

    // A click on an island token opens its editor (B8).
    const QTextCursor hit = cursorForPosition(event->pos());
    if (maybeOpenIslandAt(hit.position()) == false)
    {
        maybeOpenIslandAt(hit.position() - 1);
    }
}

void SMGuardField::mouseMoveEvent(QMouseEvent* event)
{
    QTextEdit::mouseMoveEvent(event);

    if (mHover == nullptr)
    {
        return;
    }

    // Hover a known symbol word for 300 ms -> the symbol card (S10).
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
            if (SMInlineToken::isIsland(probe.charFormat()))
            {
                out += QLatin1Char('{') + SMInlineToken::bodyOf(probe.charFormat()) + QLatin1Char('}');
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
