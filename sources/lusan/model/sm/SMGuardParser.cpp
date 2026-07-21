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
          Ident, Number, String, Lambda
        , And, Or, Not
        , Eq, Ne, Lt, Le, Gt, Ge
        , LParen, RParen, Comma, Dot, Minus
        , Unknown, End
    };

    struct Token
    {
        eTok    type  { eTok::End };
        QString text;               //!< The verbatim lexeme (Ident/Number/String/Lambda/Unknown).
        int     start { 0 };
        int     len   { 0 };
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

            const QChar c2 = (i + 1 < n) ? s.at(i + 1) : QChar();
            auto two = [&](eTok t) { out.append({ t, s.mid(start, 2), start, 2 }); i += 2; };
            auto one = [&](eTok t) { out.append({ t, s.mid(start, 1), start, 1 }); i += 1; };

            if      ((c == '&') && (c2 == '&')) { two(eTok::And); }
            else if ((c == '|') && (c2 == '|')) { two(eTok::Or);  }
            else if ((c == '=') && (c2 == '=')) { two(eTok::Eq);  }
            else if ((c == '!') && (c2 == '=')) { two(eTok::Ne);  }
            else if ((c == '<') && (c2 == '=')) { two(eTok::Le);  }
            else if ((c == '>') && (c2 == '=')) { two(eTok::Ge);  }
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

        SMGuardNode* parseOr()
        {
            QList<SMGuardNode*> parts;
            parts.append(parseAnd());
            while (peek().type == eTok::Or)
            {
                advance();
                parts.append(parseAnd());
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
                parts.append(parseCmp());
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

            // Accept and normalize a typed receiver: handler().X(...) -> X(...) (D6).
            if ((idTok.text == QStringLiteral("handler"))
                && (peek().type == eTok::LParen) && (peek(1).type == eTok::RParen)
                && (peek(2).type == eTok::Dot) && (peek(3).type == eTok::Ident))
            {
                advance(); advance(); advance();    // ( ) .
                idTok = advance();                  // the real symbol name
            }

            if (peek().type == eTok::LParen)
            {
                return parseCallOrGetter(idTok);
            }

            return resolveBare(idTok);
        }

        SMGuardNode* parseCallOrGetter(const Token& idTok)
        {
            advance();  // consume '('
            QList<SMGuardNode*> args;
            if (peek().type != eTok::RParen)
            {
                args.append(parseOr());
                while (peek().type == eTok::Comma)
                {
                    advance();
                    args.append(parseOr());
                }
            }

            const int end = peek().start + qMax(0, peek().len);
            if (peek().type == eTok::RParen) { advance(); }
            else { mSyntaxError = true; error(peek().start, qMax(1, peek().len), QStringLiteral("expected ')'")); }

            const int span = qMax(idTok.len, end - idTok.start);

            const SMMethodEntry* method = SMGuardSymbols::conditionMethod(mData, idTok.text);
            if (method != nullptr)
            {
                return SMGuardNode::makeCall(method->getId(), args);
            }

            // A zero-argument call binds an attribute-as-getter (v6 rule 1).
            if (args.isEmpty())
            {
                const uint32_t attrId = SMGuardSymbols::attributeId(mData, idTok.text);
                if (attrId != 0u)
                {
                    return SMGuardNode::makeRef(eKind::Attr, attrId);
                }
            }

            qDeleteAll(args);
            return unresolved(mText.mid(idTok.start, span), idTok.start, span
                            , QStringLiteral("unknown call '%1'").arg(idTok.text));
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
                // (parameter wins, v6 rule 2) -- quiet warning with a one-click fix in the UI.
                if ((aid != 0u) || (cid != 0u))
                {
                    warn(idTok.start, idTok.len
                       , QStringLiteral("'%1' is the stimulus parameter and hides an FSM symbol of the same name").arg(name));
                }
                return SMGuardNode::makeRef(eKind::Param, pid);
            }
            if (aid != 0u) { return SMGuardNode::makeRef(eKind::Attr, aid); }
            if (cid != 0u) { return SMGuardNode::makeRef(eKind::Const, cid); }

            return unresolved(name, idTok.start, idTok.len, QStringLiteral("unknown name '%1'").arg(name));
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
        // Nothing the user typed is lost: keep the raw text as a draft (D9).
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
