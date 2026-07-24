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
 *  \file        lusan/model/sm/SMGuardParser.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard parser: text -> ID-bound AST + diagnostics.
 *
 ************************************************************************/

#include "lusan/model/sm/SMGuardParser.hpp"

#include "lusan/data/sm/SMCondition.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMConditionText.hpp"
#include "lusan/model/sm/SMGuardSymbols.hpp"

namespace
{
    using eKind  = SMGuardNode::eKind;
    using eCmpOp = SMGuardNode::eCmpOp;

    enum class eTok
    {
          Ident, Number, String, Lambda, Ref
        , And, Or, Not
        , Eq, Ne, Lt, Le, Gt, Ge, Assign
        , LParen, RParen, Comma, Dot, Minus
        , Unknown, End
    };

    struct Token
    {
        eTok    type  { eTok::End };
        QString text;               //!< The verbatim lexeme (Ident/Number/String/Lambda/Ref/Unknown).
        int     start { 0 };
        int     len   { 0 };
        QString refKind;            //!< For a Ref token: the `#kind` (param/attr/const/cond/arg), no sigil.
        QString refName;            //!< For a Ref token: the symbol/formal name after the ':'.
        bool    brk   { false };    //!< A line break precedes this token in the source (R18 layout).
        int     indent{ 0 };        //!< The leading spaces on the token's line when \ref brk is set.
    };

    //!< True for an identifier start character.
    bool isIdentStart(QChar c)
    {
        return (c.isLetter() || (c == QLatin1Char('_')));
    }

    //!< True for an identifier continuation character.
    bool isIdentPart(QChar c)
    {
        return (c.isLetterOrNumber() || (c == QLatin1Char('_')));
    }

    //!< Scans a brace-balanced `{ ... }` block from \p i (at the '{'), respecting strings
    //!< and comments; returns the index just past the closing '}' (or n on imbalance).
    int scanLambda(const QString& s, int i, int n)
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

    //!< Tokenizes the guard source into \p out; unknown characters become Unknown tokens.
    void tokenize(const QString& s, QList<Token>& out)
    {
        const int n = s.size();
        int i = 0;
        while (i < n)
        {
            const QChar c = s.at(i);
            if (c.isSpace()) { ++i; continue; }

            const int start = i;
            if (isIdentStart(c))
            {
                ++i;
                while ((i < n) && isIdentPart(s.at(i))) { ++i; }
                out.append({ eTok::Ident, s.mid(start, i - start), start, i - start });
                continue;
            }
            if (c.isDigit())
            {
                ++i;
                while ((i < n) && (s.at(i).isLetterOrNumber() || (s.at(i) == QLatin1Char('.')))) { ++i; }
                out.append({ eTok::Number, s.mid(start, i - start), start, i - start });
                continue;
            }
            if (c == QLatin1Char('"'))
            {
                ++i;
                while (i < n)
                {
                    if (s.at(i) == QLatin1Char('\\')) { i += 2; continue; }
                    if (s.at(i) == QLatin1Char('"')) { ++i; break; }
                    ++i;
                }
                out.append({ eTok::String, s.mid(start, i - start), start, i - start });
                continue;
            }
            if (c == QLatin1Char('{'))
            {
                const int end = scanLambda(s, i, n);
                // Store the inner body only (between the outer braces).
                const int bodyStart = start + 1;
                const int bodyLen   = (end > start + 1) ? (end - 1 - bodyStart) : 0;
                Token tk;
                tk.type  = eTok::Lambda;
                tk.text  = s.mid(bodyStart, bodyLen);
                tk.start = start;
                tk.len   = end - start;
                out.append(tk);
                i = end;
                continue;
            }
            if (c == NEGuardText::RefSigil)
            {
                // A typed reference `#kind:name`. The whole mention is one Ref
                // token; the parser resolves `kind`+`name` to a symbol id (or a formal slot
                // for `#arg:`). A malformed sigil (missing kind/colon/name) yields a Ref token
                // with empty parts, which the parser reports.
                int j = start + 1;
                const int kindStart = j;
                while ((j < n) && isIdentPart(s.at(j))) { ++j; }
                const QString kind = s.mid(kindStart, j - kindStart);
                QString name;
                if ((j < n) && (s.at(j) == QLatin1Char(':')))
                {
                    ++j;
                    const int nameStart = j;
                    while ((j < n) && isIdentPart(s.at(j))) { ++j; }
                    name = s.mid(nameStart, j - nameStart);
                }
                Token tk;
                tk.type    = eTok::Ref;
                tk.text    = s.mid(start, j - start);
                tk.start   = start;
                tk.len     = j - start;
                tk.refKind = kind;
                tk.refName = name;
                out.append(tk);
                i = j;
                continue;
            }

            const QChar c2 = (i + 1 < n) ? s.at(i + 1) : QChar();
            auto two = [&](eTok t) { out.append({ t, s.mid(start, 2), start, 2 }); i += 2; };
            auto one = [&](eTok t) { out.append({ t, s.mid(start, 1), start, 1 }); i += 1; };

            if      ((c == '&') && (c2 == '&')) { two(eTok::And); }
            else if ((c == '|') && (c2 == '|')) { two(eTok::Or);  }
            else if ((c == '=') && (c2 == '=')) { two(eTok::Eq);  }
            else if ((c == '!') && (c2 == '=')) { two(eTok::Ne);  }
            else if ((c == '<') && (c2 == '=')) { two(eTok::Le);  }
            else if ((c == '>') && (c2 == '=')) { two(eTok::Ge);  }
            else if (c == '=')  { one(eTok::Assign); }
            else if (c == '!')  { one(eTok::Not);    }
            else if (c == '<')  { one(eTok::Lt);     }
            else if (c == '>')  { one(eTok::Gt);     }
            else if (c == '(')  { one(eTok::LParen); }
            else if (c == ')')  { one(eTok::RParen); }
            else if (c == ',')  { one(eTok::Comma);  }
            else if (c == '.')  { one(eTok::Dot);    }
            else if (c == '-')  { one(eTok::Minus);  }
            else                { one(eTok::Unknown);}
        }

        out.append({ eTok::End, QString(), n, 0 });
    }

    //!< Marks each token that opens a new source line and records that line's leading indent
    //!< (R18). The whitespace between two tokens is either all spaces/tabs or a single run
    //!< containing newlines; the indent is the character count after the LAST newline, so a
    //!< blank line collapses and the operand keeps only its own line's indent.
    void annotateLayout(const QString& s, QList<Token>& out)
    {
        int prevEnd = 0;
        for (Token& tk : out)
        {
            int lastNewline = -1;
            for (int i = prevEnd; i < tk.start; ++i)
            {
                if (s.at(i) == QLatin1Char('\n')) { lastNewline = i; }
            }
            if (lastNewline >= 0)
            {
                tk.brk    = true;
                tk.indent = tk.start - (lastNewline + 1);
            }
            prevEnd = tk.start + tk.len;
        }
    }

    /**
     * \brief   Recursive-descent parser over the token stream, resolving names to symbol
     *          IDs as it goes. Precedence (loosest first): Or, And, comparison, unary Not,
     *          primary -- the C++ boolean subset.
     **/
    class Parser
    {
    public:
        Parser(const StateMachineData& data, uint32_t transitionId, const QString& text, bool allowRaw, QList<Token>& tokens)
            : mData     (data)
            , mTransId  (transitionId)
            , mText     (text)
            , mAllowRaw (allowRaw)
            , mTokens   (tokens)
            , mPos      (0)
            , mSyntaxError(false)
        {
        }

        SMGuardNode* run(QList<SMGuardParser::Diagnostic>& diags)
        {
            if (peek().type == eTok::End)
            {
                return nullptr;     // empty input -> empty guard
            }

            SMGuardNode* node = parseOr();
            if ((mSyntaxError == false) && (peek().type != eTok::End))
            {
                // Trailing tokens after a complete expression are a structural error, so the
                // whole input is kept as raw when the caller opted in.
                mSyntaxError = true;
                error(peek().start, qMax(1, peek().len), QStringLiteral("unexpected '%1'").arg(peek().text));
            }

            if (mSyntaxError)
            {
                // A structural error: keep the whole input as a raw fragment when the caller
                // opted in (the "Keep as raw C++" quick-fix), otherwise report and let the
                // caller store a draft.
                delete node;
                node = SMGuardNode::makeVerbatim(eKind::Raw, mText.trimmed());
                if (mAllowRaw)
                {
                    mDiags.clear();
                }
            }

            diags = mDiags;
            return node;
        }

    private:
        const Token& peek(int ahead = 0) const
        {
            const int idx = qMin(mPos + ahead, mTokens.size() - 1);
            return mTokens.at(idx);
        }

        const Token& advance()
        {
            const Token& tk = mTokens.at(qMin(mPos, mTokens.size() - 1));
            if (mPos < mTokens.size() - 1) { ++mPos; }
            return tk;
        }

        void error(int start, int len, const QString& message)
        {
            mDiags.append({ SMGuardParser::eSeverity::Error, start, len, message });
        }

        void warn(int start, int len, const QString& message)
        {
            mDiags.append({ SMGuardParser::eSeverity::Warning, start, len, message });
        }

        //!< Wraps an unresolvable span: a Raw node when allowed, else an error + placeholder.
        SMGuardNode* unresolved(const QString& fragment, int start, int len, const QString& message)
        {
            if (mAllowRaw)
            {
                return SMGuardNode::makeVerbatim(eKind::Raw, fragment);
            }

            error(start, len, message);
            return SMGuardNode::makeVerbatim(eKind::Raw, fragment);
        }

        //!< Carries a break/indent that preceded an operand (captured before it was parsed) onto
        //!< the resulting node, so the renderer can restore the line the user opened (R18).
        static void carryLayout(SMGuardNode* node, bool brk, int indent)
        {
            if ((node != nullptr) && brk)
            {
                node->setBreakBefore(true);
                node->setIndent(qMax(0, indent));
            }
        }

        SMGuardNode* parseOr()
        {
            QList<SMGuardNode*> parts;
            parts.append(parseAnd());
            while (peek().type == eTok::Or)
            {
                advance();
                const bool brk = peek().brk;
                const int  ind = peek().indent;
                SMGuardNode* rhs = parseAnd();
                carryLayout(rhs, brk, ind);
                parts.append(rhs);
            }

            return (parts.size() == 1) ? parts.first() : SMGuardNode::makeGroup(eKind::Or, parts);
        }

        SMGuardNode* parseAnd()
        {
            QList<SMGuardNode*> parts;
            parts.append(parseCmp());
            while (peek().type == eTok::And)
            {
                advance();
                const bool brk = peek().brk;
                const int  ind = peek().indent;
                SMGuardNode* rhs = parseCmp();
                carryLayout(rhs, brk, ind);
                parts.append(rhs);
            }

            return (parts.size() == 1) ? parts.first() : SMGuardNode::makeGroup(eKind::And, parts);
        }

        SMGuardNode* parseCmp()
        {
            SMGuardNode* lhs = parseUnary();
            eCmpOp op;
            if (cmpOp(peek().type, op))
            {
                advance();
                SMGuardNode* rhs = parseUnary();
                return SMGuardNode::makeCmp(op, lhs, rhs);
            }
            if (peek().type == eTok::Assign)
            {
                // A single '=' in a boolean guard is almost always a typo for '=='.
                // Flag it (a warning, not a gate) and parse it as an equality comparison --
                // a guard is read-only, so we never build a setter.
                const Token assign = peek();
                warn(assign.start, qMax(1, assign.len), QStringLiteral("assignment in a guard; did you mean '=='?"));
                advance();
                SMGuardNode* rhs = parseUnary();
                return SMGuardNode::makeCmp(eCmpOp::Eq, lhs, rhs);
            }

            return lhs;
        }

        SMGuardNode* parseUnary()
        {
            if (peek().type == eTok::Not)
            {
                advance();
                return SMGuardNode::makeNot(parseUnary());
            }

            return parsePrimary();
        }

        SMGuardNode* parsePrimary()
        {
            const Token tk = peek();
            switch (tk.type)
            {
            case eTok::LParen:
                {
                    advance();
                    SMGuardNode* inner = parseOr();
                    if (peek().type == eTok::RParen) { advance(); }
                    else { mSyntaxError = true; error(peek().start, qMax(1, peek().len), QStringLiteral("expected ')'")); }
                    return inner;
                }

            case eTok::Lambda:
                advance();
                return SMGuardNode::makeVerbatim(eKind::Lambda, tk.text);

            case eTok::Number:
            case eTok::String:
                advance();
                return SMGuardNode::makeVerbatim(eKind::Lit, tk.text);

            case eTok::Minus:
                if (peek(1).type == eTok::Number)
                {
                    advance();
                    const Token num = advance();
                    return SMGuardNode::makeVerbatim(eKind::Lit, QStringLiteral("-") + num.text);
                }
                mSyntaxError = true;
                error(tk.start, 1, QStringLiteral("unexpected '-'"));
                advance();
                return SMGuardNode::makeVerbatim(eKind::Raw, tk.text);

            case eTok::Ident:
                return parseIdentPrimary();

            case eTok::Ref:
                return parseRefPrimary();

            default:
                mSyntaxError = true;
                error(tk.start, qMax(1, tk.len), QStringLiteral("unexpected '%1'").arg(tk.text.isEmpty() ? QStringLiteral("end") : tk.text));
                if (tk.type != eTok::End) { advance(); }
                return SMGuardNode::makeVerbatim(eKind::Raw, tk.text);
            }
        }

        SMGuardNode* parseIdentPrimary()
        {
            Token idTok = advance();

            // Accept and normalize a typed receiver: handler().X(...) -> X(...).
            if ((idTok.text == QStringLiteral("handler"))
                && (peek().type == eTok::LParen) && (peek(1).type == eTok::RParen)
                && (peek(2).type == eTok::Dot) && (peek(3).type == eTok::Ident))
            {
                advance(); advance(); advance();    // ( ) .
                idTok = advance();                  // the real symbol name
            }

            if (peek().type == eTok::LParen)
            {
                return parseIdentCall(idTok);
            }

            return resolveBare(idTok);
        }

        /**
         * \brief   One parsed call argument -- a positional value, or a `@arg:formal = value`
         *          named binding. Formal-id resolution happens after the whole list is read
         *          (see \ref bindArgs), so a positional resolves to the nth unfilled formal.
         **/
        struct ArgEntry
        {
            bool         named { false };
            QString      formalName;    //!< The `@arg:` formal name (named entries only).
            int          nameStart { 0 };
            int          nameLen   { 0 };
            SMGuardNode* value { nullptr };
        };

        //!< Parses `( ... )` (peek is at the '('); returns the raw arg entries. \p endOut gets
        //!< the offset just past the ')'. Enforces the Python mixed rule: a positional
        //!< after a named argument is a syntax error.
        QList<ArgEntry> parseArgList(int& endOut)
        {
            advance();  // consume '('
            QList<ArgEntry> entries;
            bool seenNamed = false;
            if (peek().type != eTok::RParen)
            {
                for (;;)
                {
                    // Capture a break BEFORE the entry (a call broken across lines) so it lands on
                    // the value node once parsed; a named `#arg:` entry keeps it on its value too.
                    const bool entryBrk = peek().brk;
                    const int  entryInd = peek().indent;
                    ArgEntry entry;
                    if ((peek().type == eTok::Ref) && (peek().refKind == QStringLiteral("arg")))
                    {
                        const Token argTok = advance();
                        entry.named      = true;
                        entry.formalName = argTok.refName;
                        entry.nameStart  = argTok.start;
                        entry.nameLen    = argTok.len;
                        if (argTok.refName.isEmpty())
                        {
                            mSyntaxError = true;
                            error(argTok.start, qMax(1, argTok.len), QStringLiteral("expected a formal name after '@arg:'"));
                        }
                        if (peek().type == eTok::Assign) { advance(); }
                        else
                        {
                            mSyntaxError = true;
                            error(peek().start, qMax(1, peek().len)
                                , QStringLiteral("expected '=' after '@arg:%1'").arg(argTok.refName));
                        }
                        entry.value = parseOr();
                        seenNamed   = true;
                    }
                    else
                    {
                        if (seenNamed)
                        {
                            // Once a named argument appears, a bare positional is
                            // ambiguous (which slot?) -- a hard syntax error.
                            mSyntaxError = true;
                            error(peek().start, qMax(1, peek().len), QStringLiteral("positional argument after named argument"));
                        }
                        entry.named = false;
                        entry.value = parseOr();
                    }
                    carryLayout(entry.value, entryBrk, entryInd);
                    entries.append(entry);

                    if (peek().type == eTok::Comma) { advance(); continue; }
                    break;
                }
            }

            endOut = peek().start + qMax(0, peek().len);
            if (peek().type == eTok::RParen) { advance(); }
            else { mSyntaxError = true; error(peek().start, qMax(1, peek().len), QStringLiteral("expected ')'")); }
            return entries;
        }

        //!< Resolves \p entries to value nodes for a known \p method, keying each on the formal's
        //!< document id (Option A): a positional fills the nth unfilled formal; a named binds its
        //!< formal by name. Extra positionals past the declared count keep id 0.
        QList<SMGuardNode*> bindArgs(const SMMethodEntry* method, QList<ArgEntry>& entries)
        {
            QList<SMGuardNode*> args;
            const QList<MethodParameter>& formals = method->getElements();
            QList<bool> filled;
            filled.fill(false, formals.size());

            for (ArgEntry& entry : entries)
            {
                uint32_t formalId = 0u;
                if (entry.named)
                {
                    int found = -1;
                    for (int fi = 0; fi < formals.size(); ++fi)
                    {
                        if (formals.at(fi).getName() == entry.formalName) { found = fi; break; }
                    }
                    if (found < 0)
                    {
                        warn(entry.nameStart, qMax(1, entry.nameLen)
                           , QStringLiteral("'%1' is not a parameter of '%2'").arg(entry.formalName, method->getName()));
                    }
                    else
                    {
                        if (filled.at(found))
                        {
                            warn(entry.nameStart, qMax(1, entry.nameLen)
                               , QStringLiteral("'%1' is already bound").arg(entry.formalName));
                        }
                        formalId      = formals.at(found).getId();
                        filled[found] = true;
                    }
                }
                else
                {
                    for (int fi = 0; fi < formals.size(); ++fi)
                    {
                        if (filled.at(fi) == false) { formalId = formals.at(fi).getId(); filled[fi] = true; break; }
                    }
                }

                if (entry.value != nullptr) { entry.value->setArgFormalId(formalId); }
                args.append(entry.value);
            }

            return args;
        }

        //!< Frees the value nodes of \p entries (used when the callee did not resolve).
        static void deleteArgs(QList<ArgEntry>& entries)
        {
            for (ArgEntry& entry : entries) { delete entry.value; }
        }

        SMGuardNode* parseIdentCall(const Token& idTok)
        {
            int end = 0;
            QList<ArgEntry> entries = parseArgList(end);
            const int span = qMax(idTok.len, end - idTok.start);

            const SMMethodEntry* method = SMGuardSymbols::conditionMethod(mData, idTok.text);
            if (method != nullptr)
            {
                return SMGuardNode::makeCall(method->getId(), bindArgs(method, entries));
            }

            // A zero-argument call binds an attribute-as-getter.
            if (entries.isEmpty())
            {
                const uint32_t attrId = SMGuardSymbols::attributeId(mData, idTok.text);
                if (attrId != 0u)
                {
                    return SMGuardNode::makeRef(eKind::Attr, attrId);
                }
            }

            deleteArgs(entries);
            return unresolved(mText.mid(idTok.start, span), idTok.start, span
                            , QStringLiteral("unknown call '%1'").arg(idTok.text));
        }

        //!< A `@cond:name(...)` reference: an explicit condition-method call. An unresolved name
        //!< is an error (the user stated the kind), never a silent raw fragment.
        SMGuardNode* parseCondCall(const Token& refTok)
        {
            const SMMethodEntry* method = SMGuardSymbols::conditionMethod(mData, refTok.refName);
            if (peek().type == eTok::LParen)
            {
                int end = 0;
                QList<ArgEntry> entries = parseArgList(end);
                if (method != nullptr)
                {
                    return SMGuardNode::makeCall(method->getId(), bindArgs(method, entries));
                }
                deleteArgs(entries);
                const int span = qMax(refTok.len, end - refTok.start);
                return unresolved(refTok.text, refTok.start, span, QStringLiteral("unknown condition '%1'").arg(refTok.refName));
            }

            // A parenless `#cond:name` is a zero-argument boolean call.
            if (method != nullptr) { return SMGuardNode::makeCall(method->getId(), QList<SMGuardNode*>()); }
            return unresolved(refTok.text, refTok.start, refTok.len, QStringLiteral("unknown condition '%1'").arg(refTok.refName));
        }

        //!< A typed reference `#kind:name`. An explicit kind must resolve within that
        //!< kind or it is an error -- unlike a bare identifier, which is raw.
        SMGuardNode* parseRefPrimary()
        {
            const Token refTok = advance();
            const QString& kind = refTok.refKind;
            const QString& name = refTok.refName;

            if (kind == QStringLiteral("cond"))
            {
                return parseCondCall(refTok);
            }
            if (name.isEmpty())
            {
                mSyntaxError = true;
                error(refTok.start, qMax(1, refTok.len), QStringLiteral("expected a name after '@%1:'").arg(kind));
                return SMGuardNode::makeVerbatim(eKind::Raw, refTok.text);
            }
            if (kind == QStringLiteral("param"))
            {
                const uint32_t pid = SMGuardSymbols::paramId(mData, mTransId, name);
                if (pid != 0u) { return SMGuardNode::makeRef(eKind::Param, pid); }
                return unresolved(refTok.text, refTok.start, refTok.len, QStringLiteral("unknown parameter '%1'").arg(name));
            }
            if (kind == QStringLiteral("attr"))
            {
                const uint32_t aid = SMGuardSymbols::attributeId(mData, name);
                if (aid != 0u) { return SMGuardNode::makeRef(eKind::Attr, aid); }
                return unresolved(refTok.text, refTok.start, refTok.len, QStringLiteral("unknown attribute '%1'").arg(name));
            }
            if (kind == QStringLiteral("const"))
            {
                const uint32_t cid = SMGuardSymbols::constantId(mData, name);
                if (cid != 0u) { return SMGuardNode::makeRef(eKind::Const, cid); }
                return unresolved(refTok.text, refTok.start, refTok.len, QStringLiteral("unknown constant '%1'").arg(name));
            }
            if (kind == QStringLiteral("arg"))
            {
                // `@arg:` names a callee's formal slot; it is only meaningful as the LHS of a
                // named call argument (handled inside parseArgList). Standalone, it is a misuse.
                mSyntaxError = true;
                error(refTok.start, qMax(1, refTok.len), QStringLiteral("'@arg:' is valid only inside a condition call's arguments"));
                return SMGuardNode::makeVerbatim(eKind::Raw, refTok.text);
            }

            mSyntaxError = true;
            error(refTok.start, qMax(1, refTok.len), QStringLiteral("unknown reference kind '@%1'").arg(kind));
            return SMGuardNode::makeVerbatim(eKind::Raw, refTok.text);
        }

        SMGuardNode* resolveBare(const Token& idTok)
        {
            const QString name = idTok.text;
            if ((name == QStringLiteral("true")) || (name == QStringLiteral("false")))
            {
                return SMGuardNode::makeVerbatim(eKind::Lit, name);
            }

            const uint32_t pid = SMGuardSymbols::paramId(mData, mTransId, name);
            const uint32_t aid = SMGuardSymbols::attributeId(mData, name);
            const uint32_t cid = SMGuardSymbols::constantId(mData, name);

            if (pid != 0u)
            {
                // A stimulus parameter shadows an FSM attribute/constant of the same name
                // (parameter wins) -- quiet warning with a one-click fix in the UI.
                if ((aid != 0u) || (cid != 0u))
                {
                    warn(idTok.start, idTok.len
                       , QStringLiteral("'%1' is the stimulus parameter and hides an FSM symbol of the same name").arg(name));
                }
                return SMGuardNode::makeRef(eKind::Param, pid);
            }
            if (aid != 0u) { return SMGuardNode::makeRef(eKind::Attr, aid); }
            if (cid != 0u) { return SMGuardNode::makeRef(eKind::Const, cid); }

            // Every bare name in a guard must resolve to a DEFINED, typed symbol: a stimulus
            // parameter, an FSM attribute, or an FSM constant (all data objects that must be
            // declared). A name that resolves to none of these is undefined -- an error, NOT silent
            // "raw C++". The one exception is the explicit "raw code" mode (allowRaw): there the user
            // deliberately opted out of validation and owns correctness, so unresolved() keeps the
            // text verbatim without error. A named condition method is a call (`name(...)`) resolved
            // elsewhere; an inline lambda is an island (`{...}`); literals are handled before here.
            return unresolved(name, idTok.start, idTok.len
                            , QStringLiteral("unknown symbol '%1' -- not a defined parameter, attribute, or constant").arg(name));
        }

        static bool cmpOp(eTok t, eCmpOp& op)
        {
            switch (t)
            {
            case eTok::Eq: op = eCmpOp::Eq; return true;
            case eTok::Ne: op = eCmpOp::Ne; return true;
            case eTok::Lt: op = eCmpOp::Lt; return true;
            case eTok::Le: op = eCmpOp::Le; return true;
            case eTok::Gt: op = eCmpOp::Gt; return true;
            case eTok::Ge: op = eCmpOp::Ge; return true;
            default:       return false;
            }
        }

    private:
        const StateMachineData&             mData;
        uint32_t                            mTransId;
        const QString&                      mText;
        bool                                mAllowRaw;
        QList<Token>&                       mTokens;
        int                                 mPos;
        bool                                mSyntaxError;
        QList<SMGuardParser::Diagnostic>    mDiags;
    };
}

//////////////////////////////////////////////////////////////////////////
// SMGuardParser::Result
//////////////////////////////////////////////////////////////////////////

bool SMGuardParser::Result::hasError() const
{
    for (const Diagnostic& d : diagnostics)
    {
        if (d.severity == eSeverity::Error)
            return true;
    }

    return false;
}

bool SMGuardParser::Result::resolved() const
{
    return (tree != nullptr) && (hasError() == false);
}

//////////////////////////////////////////////////////////////////////////
// SMGuardParser operations
//////////////////////////////////////////////////////////////////////////

SMGuardParser::Result SMGuardParser::parse(const StateMachineData& data, uint32_t transitionId, const QString& text, bool allowRaw /*= false*/)
{
    Result result;
    if (text.trimmed().isEmpty())
    {
        return result;
    }

    QList<Token> tokens;
    tokenize(text, tokens);
    annotateLayout(text, tokens);

    Parser parser(data, transitionId, text, allowRaw, tokens);
    result.tree = parser.run(result.diagnostics);
    return result;
}

SMGuard SMGuardParser::parseToGuard(const StateMachineData& data, uint32_t transitionId, const QString& text, bool allowRaw /*= false*/)
{
    SMGuard guard;
    if (text.trimmed().isEmpty())
    {
        return guard;   // empty guard
    }

    Result result = parse(data, transitionId, text, allowRaw);
    if (result.resolved())
    {
        guard.setTree(result.tree);
    }
    else
    {
        // Nothing the user typed is lost: keep the raw text as a draft.
        delete result.tree;
        guard.setDraft(text);
    }

    return guard;
}

SMGuard SMGuardParser::fromLegacy(const SMConditionList& legacy)
{
    SMGuard guard;
    if (legacy.getChildren().isEmpty())
    {
        return guard;   // no legacy condition -> empty guard
    }

    // The legacy tree renders to text with the legacy renderer and re-enters as a draft:
    // the user re-resolves it in the editor; nothing is silently dropped.
    guard.setDraft(SMConditionText::preview(legacy));
    return guard;
}
