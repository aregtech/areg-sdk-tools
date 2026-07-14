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
 *  \file        lusan/view/sm/SMGuardHelpCard.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard help card (v7 B11 / S11).
 *
 ************************************************************************/

#include "lusan/view/sm/SMGuardHelpCard.hpp"

#include "lusan/view/sm/NEGuardStyle.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace
{
    //!< A glyph + description + example row of the "what can a guard use" table.
    void addUseRow(QGridLayout* grid, int row, NEGuardStyle::eOwner owner, const QString& glyph, const QString& what, const QString& example)
    {
        QLabel* glyphLabel = new QLabel(glyph);
        QFont font = glyphLabel->font();
        font.setBold(true);
        glyphLabel->setFont(font);
        QPalette palette = glyphLabel->palette();
        palette.setColor(QPalette::WindowText, NEGuardStyle::ownerColor(owner));
        glyphLabel->setPalette(palette);

        grid->addWidget(glyphLabel, row, 0);
        grid->addWidget(new QLabel(what), row, 1);
        grid->addWidget(new QLabel(example), row, 2);
    }

    //!< A `you write -> it runs` mapping row (monospace).
    void addMapRow(QGridLayout* grid, int row, const QString& write, const QString& runs)
    {
        QLabel* left = new QLabel(write);
        QLabel* right = new QLabel(runs);
        left->setStyleSheet(QStringLiteral("font-family: monospace;"));
        right->setStyleSheet(QStringLiteral("font-family: monospace;"));
        grid->addWidget(left, row, 0);
        grid->addWidget(new QLabel(QStringLiteral("->")), row, 1);
        grid->addWidget(right, row, 2);
    }
}

SMGuardHelpCard::SMGuardHelpCard(QWidget* parent /*= nullptr*/)
    : QFrame(parent)
{
    setObjectName(QStringLiteral("smGuardHelpCard"));
    setWindowFlags(Qt::Popup);
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Raised);
    buildUi();
}

void SMGuardHelpCard::buildUi()
{
    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(12, 12, 12, 12);
    outer->setSpacing(8);

    QLabel* title = new QLabel(tr("What can a guard use?"), this);
    QFont titleFont = title->font();
    titleFont.setBold(true);
    title->setFont(titleFont);
    outer->addWidget(title);

    QGridLayout* uses = new QGridLayout();
    uses->setHorizontalSpacing(16);
    addUseRow(uses, 0, NEGuardStyle::eOwner::Stimulus, QStringLiteral("a"),  tr("stimulus argument"), QStringLiteral("count"));
    addUseRow(uses, 1, NEGuardStyle::eOwner::Fsm,      QStringLiteral("#"),  tr("FSM attribute"),     QStringLiteral("IsNightMode"));
    addUseRow(uses, 2, NEGuardStyle::eOwner::Fsm,      QStringLiteral("K"),  tr("FSM constant"),      QStringLiteral("MIN_WAITING"));
    addUseRow(uses, 3, NEGuardStyle::eOwner::Handler,  QStringLiteral("h"),  tr("handler condition"), QStringLiteral("HasWaiting()"));
    addUseRow(uses, 4, NEGuardStyle::eOwner::Fsm,      QStringLiteral("{}"), tr("lambda"),            QStringLiteral("{ ... }"));
    outer->addLayout(uses);

    QLabel* mapTitle = new QLabel(tr("you write        ->  it runs"), this);
    mapTitle->setStyleSheet(QStringLiteral("font-family: monospace; font-weight: bold;"));
    outer->addWidget(mapTitle);

    QGridLayout* maps = new QGridLayout();
    maps->setHorizontalSpacing(12);
    addMapRow(maps, 0, QStringLiteral("my_attribute"),    QStringLiteral("my_attribute()"));
    addMapRow(maps, 1, QStringLiteral("my_condition(x)"), QStringLiteral("handler().my_condition(x)"));
    addMapRow(maps, 2, QStringLiteral("{ return ...; }"), QStringLiteral("inline lambda"));
    addMapRow(maps, 3, QStringLiteral("IsCalmHours(x)"),  QStringLiteral("mIsCalmHours(x)"));
    outer->addLayout(maps);
}

void SMGuardHelpCard::popupAt(const QPoint& globalTopLeft)
{
    adjustSize();
    move(globalTopLeft);
    show();
}
