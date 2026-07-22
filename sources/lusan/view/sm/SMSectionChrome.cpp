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
 *  \file        lusan/view/sm/SMSectionChrome.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the shared section chrome implementation.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/view/sm/SMSectionChrome.hpp"

#include "lusan/view/sm/SMAccordion.hpp"
#include "lusan/view/sm/SMToolIcons.hpp"

#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyCombination>
#include <QLabel>
#include <QShortcut>
#include <QToolButton>
#include <QVBoxLayout>

//////////////////////////////////////////////////////////////////////////
// SMSectionChrome class implementation
//////////////////////////////////////////////////////////////////////////

SMSectionChrome::SMSectionChrome(QWidget* parent /*= nullptr*/)
    : QWidget       (parent)
    , mOuter        (nullptr)
    , mHeader       (nullptr)
    , mSectionBox   (nullptr)
    , mTrailingBox  (nullptr)
    , mBodyBox      (nullptr)
    , mFooterBox    (nullptr)
    , mTitle        (nullptr)
    , mCompactBtn   (nullptr)
    , mDivider      (nullptr)
    , mAccordion    (nullptr)
    , mLastSection  (-1)
{
    mOuter = new QVBoxLayout(this);
    mOuter->setContentsMargins(8, 8, 8, 8);
    mOuter->setSpacing(6);

    // Header row: the title label, one jump button per accordion section, the compact toggle, a
    // rule, a stretch, then whatever action buttons the host appends. Every glyph is drawn by
    // SMToolIcons -- the same thin-stroke vector language as the canvas toolbar -- and icon-only
    // keeps the narrow Properties dock usable; each button carries a tooltip.
    mHeader = new QHBoxLayout();

    mTitle = new QLabel(this);
    mTitle->hide();                                 // shown only when the host sets a title.
    mHeader->addWidget(mTitle);

    mSectionBox = new QHBoxLayout();
    mSectionBox->setContentsMargins(0, 0, 0, 0);
    mHeader->addLayout(mSectionBox);

    mCompactBtn = makeTool(QStringLiteral("smSectionCompact"), SMToolIcons::icon(SMToolIcons::eIcon::GuardCompact)
                          , tr("Compact: keep only one section open at a time"), true);
    mHeader->addWidget(mCompactBtn);

    // A rule tells the section controls apart from the host action buttons that follow.
    mDivider = new QFrame(this);
    mDivider->setObjectName(QStringLiteral("smSectionSeparator"));
    mDivider->setFrameShape(QFrame::VLine);
    mDivider->setFrameShadow(QFrame::Sunken);
    mHeader->addWidget(mDivider);

    mHeader->addStretch(1);

    mTrailingBox = new QHBoxLayout();
    mTrailingBox->setContentsMargins(0, 0, 0, 0);
    mHeader->addLayout(mTrailingBox);

    mOuter->addLayout(mHeader);

    mBodyBox = new QVBoxLayout();
    mBodyBox->setContentsMargins(0, 0, 0, 0);
    mBodyBox->setSpacing(6);
    mOuter->addLayout(mBodyBox);

    mAccordion = new SMAccordion(this);
    mOuter->addWidget(mAccordion);

    mFooterBox = new QVBoxLayout();
    mFooterBox->setContentsMargins(0, 0, 0, 0);
    mFooterBox->setSpacing(6);
    mOuter->addLayout(mFooterBox);

    // The toolbar section buttons and the accordion headers are two views of one state: opening a
    // section presses its button, pressing a button opens the section.
    connect(mAccordion, &SMAccordion::sectionToggled, this, [this](int index, bool open)
    {
        if ((index >= 0) && (index < mSectionBtns.size()))
        {
            mSectionBtns[index]->setChecked(open);
        }

        if (open)
        {
            mLastSection = index;                   // Remember-last (per tab).
        }

        emit sectionActivated(index, open);
    });

    connect(mCompactBtn, &QToolButton::toggled, this, [this](bool checked)
    {
        mAccordion->setCompact(checked);
    });

    // The button starts in step with the accordion (compact by default); a host overrides it via
    // setCompact() when its tab wants several sections open at once.
    mCompactBtn->setChecked(mAccordion->isCompact());
}

QToolButton* SMSectionChrome::makeTool(const QString& name, const QIcon& icon, const QString& tip, bool checkable)
{
    QToolButton* button = new QToolButton(this);
    button->setObjectName(name);
    button->setIcon(icon);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setAutoRaise(true);
    button->setCheckable(checkable);
    button->setCursor(Qt::PointingHandCursor);
    button->setToolTip(tip);
    return button;
}

int SMSectionChrome::addSection(const QIcon& icon, const QString& title, QWidget* content, const QString& jumpTip /*= QString()*/)
{
    const int index = mAccordion->addSection(icon, title, content);

    QToolButton* jump = makeTool(QStringLiteral("smSectionJump%1").arg(index), icon
                               , jumpTip.isEmpty() ? title : jumpTip, true);
    mSectionBtns.append(jump);
    mSectionBox->addWidget(jump);

    connect(jump, &QToolButton::clicked, this, [this, index](bool checked)
    {
        mAccordion->setSectionOpen(index, checked);
        mSectionBtns[index]->setChecked(mAccordion->isSectionOpen(index));
    });

    // Alt+1 .. Alt+N reach the same section as the Nth button. The shortcut just presses that
    // button, so button, header and shortcut stay one code path. Scoped to the chrome and its
    // children so it never leaks app-wide.
    QShortcut* jumpKey = new QShortcut(QKeySequence(QKeyCombination(Qt::AltModifier, static_cast<Qt::Key>(Qt::Key_1 + index))), this);
    jumpKey->setContext(Qt::WidgetWithChildrenShortcut);
    connect(jumpKey, &QShortcut::activated, this, [this, index]() { mSectionBtns[index]->click(); });

    return index;
}

void SMSectionChrome::setTitle(const QString& text)
{
    mTitle->setText(text);
    mTitle->setVisible(text.isEmpty() == false);
}

void SMSectionChrome::addHeaderWidget(QWidget* widget)
{
    mTrailingBox->addWidget(widget);
}

void SMSectionChrome::addBodyWidget(QWidget* widget)
{
    mBodyBox->addWidget(widget);
}

void SMSectionChrome::addFooterWidget(QWidget* widget)
{
    mFooterBox->addWidget(widget);
}

void SMSectionChrome::addFooterStretch()
{
    mFooterBox->addStretch(1);
}

QToolButton* SMSectionChrome::sectionButton(int index) const
{
    return ((index >= 0) && (index < mSectionBtns.size())) ? mSectionBtns[index] : nullptr;
}

bool SMSectionChrome::isCompact() const
{
    return mAccordion->isCompact();
}

void SMSectionChrome::setCompact(bool compact)
{
    mCompactBtn->setChecked(compact);
    mAccordion->setCompact(compact);
}

void SMSectionChrome::setCurrentSection(int index)
{
    mAccordion->setCurrentIndex(index);
}

int SMSectionChrome::currentSection() const
{
    return mAccordion->currentIndex();
}
