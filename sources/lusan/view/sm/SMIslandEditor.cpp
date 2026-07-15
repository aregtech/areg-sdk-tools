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
 *  \file        lusan/view/sm/SMIslandEditor.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard island editor (v7 B8 / S4).
 *
 ************************************************************************/

#include "lusan/view/sm/SMIslandEditor.hpp"

#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMGuardSymbols.hpp"
#include "lusan/model/sm/SMSymbolIndex.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"

#include "lusan/view/sm/NEGuardStyle.hpp"
#include "lusan/view/sm/SMCodeEditor.hpp"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace
{
    //!< Brace balance of \p text, string/char literals and comments skipped.
    int braceBalance(const QString& text)
    {
        int depth = 0;
        int i = 0;
        const int n = text.length();
        while (i < n)
        {
            const QChar c = text.at(i);
            if ((c == QLatin1Char('/')) && (i + 1 < n) && (text.at(i + 1) == QLatin1Char('/')))
            {
                while ((i < n) && (text.at(i) != QLatin1Char('\n'))) { ++i; }
                continue;
            }
            if ((c == QLatin1Char('/')) && (i + 1 < n) && (text.at(i + 1) == QLatin1Char('*')))
            {
                i += 2;
                while ((i + 1 < n) && !((text.at(i) == QLatin1Char('*')) && (text.at(i + 1) == QLatin1Char('/')))) { ++i; }
                i += 2;
                continue;
            }
            if ((c == QLatin1Char('"')) || (c == QLatin1Char('\'')))
            {
                const QChar quote = c;
                ++i;
                while (i < n)
                {
                    if (text.at(i) == QLatin1Char('\\')) { i += 2; continue; }
                    if (text.at(i) == quote) { ++i; break; }
                    ++i;
                }
                continue;
            }

            if (c == QLatin1Char('{')) { ++depth; }
            else if (c == QLatin1Char('}')) { --depth; }
            ++i;
        }

        return depth;
    }
}

SMIslandEditor::SMIslandEditor(QWidget* parent /*= nullptr*/)
    : QFrame    (parent)
    , mHeader   (nullptr)
    , mScope    (nullptr)
    , mBody     (nullptr)
    , mWarn     (nullptr)
    , mNameBtn  (nullptr)
    , mMoveBtn  (nullptr)
    , mCloseBtn (nullptr)
    , mIsland   (-1)
{
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Raised);

    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(8, 6, 8, 6);
    outer->setSpacing(4);

    QHBoxLayout* header = new QHBoxLayout();
    mHeader = new QLabel(tr("{} lambda"), this);
    QFont bold = mHeader->font();
    bold.setBold(true);
    mHeader->setFont(bold);
    header->addWidget(mHeader);
    header->addStretch(1);
    QLabel* contract = new QLabel(tr("must return bool"), this);
    header->addWidget(contract);
    outer->addLayout(header);

    mScope = new QLabel(this);
    mScope->setWordWrap(true);
    outer->addWidget(mScope);

    mBody = new SMCodeEditor(this);
    mBody->setObjectName(QStringLiteral("smIslandBody"));
    mBody->setSignature(QString());
    mBody->setNote(tr("shallow checks only -- the body is stored verbatim and compiles in your project"));
    outer->addWidget(mBody);

    mWarn = new QLabel(this);
    mWarn->setStyleSheet(QStringLiteral("color: %1;")
                         .arg(NEGuardStyle::severityColor(NEGuardStyle::eSeverity::Warn).name()));
    mWarn->setVisible(false);
    outer->addWidget(mWarn);

    QHBoxLayout* footer = new QHBoxLayout();
    mNameBtn = new QPushButton(tr("Name it..."), this);
    mMoveBtn = new QPushButton(tr("Move to handler..."), this);
    mCloseBtn = new QPushButton(tr("Close"), this);
    footer->addWidget(mNameBtn);
    footer->addWidget(mMoveBtn);
    footer->addStretch(1);
    footer->addWidget(mCloseBtn);
    outer->addLayout(footer);

    connect(mBody->ctrlBody(), &QPlainTextEdit::textChanged, this, &SMIslandEditor::updateWarnings);
    connect(mNameBtn, &QPushButton::clicked, this, [this]()
    {
        emit nameRequested(mIsland, body());
    });
    connect(mMoveBtn, &QPushButton::clicked, this, [this]()
    {
        emit moveToHandlerRequested(mIsland, body());
    });
    connect(mCloseBtn, &QPushButton::clicked, this, [this]()
    {
        emit closeRequested(mIsland, body());
    });

    mBody->ctrlBody()->installEventFilter(this);
}

void SMIslandEditor::openFor(StateMachineModel& model, uint32_t transitionId, int islandIndex, const QString& body)
{
    mIsland = islandIndex;
    setObjectName(QStringLiteral("smIsland_%1").arg(islandIndex));

    const StateMachineData& data = model.getData();
    const QStringList names = SMGuardSymbols::paramNames(data, transitionId);
    const QStringList types = SMGuardSymbols::paramTypes(data, transitionId);
    QString stim;
    for (int i = 0; i < names.size(); ++i)
    {
        if (i > 0)
        {
            stim += QStringLiteral(", ");
        }

        stim += names.at(i) + QStringLiteral(" : ") + ((i < types.size()) ? types.at(i) : QString());
    }

    mScope->setText(stim.isEmpty()
                    ? tr("in scope: attributes . constants . handler conditions via handler().x()")
                    : tr("in scope: %1 (stimulus) . attributes . constants . handler conditions via handler().x()").arg(stim));

    mBody->setCompletions(SMSymbolIndex::completionWords(data, transitionId, true));
    mBody->ctrlBody()->setPlainText(body);
    updateWarnings();

    show();
    mBody->ctrlBody()->setFocus();
}

QString SMIslandEditor::body() const
{
    return mBody->ctrlBody()->toPlainText();
}

bool SMIslandEditor::eventFilter(QObject* watched, QEvent* event)
{
    if ((watched == mBody->ctrlBody()) && (event->type() == QEvent::KeyPress))
    {
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
        if (((key->key() == Qt::Key_Return) || (key->key() == Qt::Key_Enter))
            && (key->modifiers() & Qt::ControlModifier))
        {
            // Ctrl+Enter: commit the body, keep the island open (B16).
            emit bodyCommitted(mIsland, body());
            return true;
        }
    }

    return QFrame::eventFilter(watched, event);
}

void SMIslandEditor::updateWarnings()
{
    const QString text = body();
    QStringList warnings;
    const int balance = braceBalance(text);
    if (balance != 0)
    {
        warnings.append((balance > 0) ? tr("unbalanced braces: %1 unclosed '{'").arg(balance)
                                      : tr("unbalanced braces: %1 extra '}'").arg(-balance));
    }

    if (text.contains(QStringLiteral("return")) == false)
    {
        warnings.append(tr("no 'return' -- a lambda must return bool"));
    }

    mWarn->setText(warnings.isEmpty() ? QString() : QStringLiteral("(!) ") + warnings.join(QStringLiteral("; ")));
    mWarn->setVisible(warnings.isEmpty() == false);
}
