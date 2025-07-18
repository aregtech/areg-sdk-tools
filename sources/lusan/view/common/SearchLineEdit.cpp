/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]aregtech.com.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/common/SearchLineEdit.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, QLineEdit control with tool-buttons for search.
 *
 ************************************************************************/

#include "lusan/view/common/SearchLineEdit.hpp"

#include <QHBoxLayout>
#include <QIcon>
#include <QWidget>

SearchLineEdit::SearchLineEdit(const QList<SearchLineEdit::eToolButton>& addButtons, QSize buttonSize, QWidget* parent)
    : QLineEdit     (parent)

    , mBtnSearch    (nullptr)
    , mBtnMatchCase (nullptr)
    , mBtnMatchWord (nullptr)
    , mBtnBackward  (nullptr)
    , mButtonFlags  (0)
    , mButtons      ( )
{
    if ((addButtons.isEmpty() == false) && (addButtons[0] != SearchLineEdit::eToolButton::ToolButtonNothing)
    {
        for (auto entry : addButtons)
        {
            switch (entry)
            {
            case SearchLineEdit::eToolButton::ToolButtonSearch:
                mButtonFlags |= static_cast<uint32_t>(entry);
                mButtons.push_back(entry);
                mBtnSearch = new QToolButton(this);
                mBtnSearch->setObjectName("buttonSearch");
                mBtnSearch->setMinimumSize(buttonSize);
                mBtnSearch->setMaximumSize(buttonSize);
                mBtnSearch->setAutoFillBackground(true);
                mBtnSearch->setCheckable(true);
                mBtnSearch->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::EditFind));
                mBtnSearch->setShortcut(QString::fromUtf8("Ctrl+F"));
                connect(mBtnSearch, &QToolButton::toggled, [](bool checked) { emit signalButtonSearchClicked(checked); });
                break;
                
            case SearchLineEdit::eToolButton::ToolButtonMatchCase:
                mButtonFlags |= static_cast<uint32_t>(entry);
                mButtons.push_back(entry);
                mBtnMatchCase = new QToolButton(this);
                mBtnMatchCase->setObjectName("buttonSearch");
                mBtnMatchCase->setMinimumSize(buttonSize);
                mBtnMatchCase->setMaximumSize(buttonSize);
                mBtnMatchCase->setAutoFillBackground(true);
                mBtnMatchCase->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::EditFind));
                mBtnMatchCase->setShortcut(QString::fromUtf8("Ctrl+F"));
                connect(mBtnMatchCase, &QToolButton::toggled, [](bool checked) { emit signalButtonSearchMatchCaseClicked(checked); });
                break;
                
            }
                )
        }

    // Create tool buttons
    mBtnCase = new QToolButton(this);
    mBtnCase->setCheckable(true);
    mBtnCase->setIcon(QIcon(":/icons/case.png")); // Use your icon
    mBtnCase->setToolTip("Match Case");

    mBtnWhole = new QToolButton(this);
    mBtnWhole->setCheckable(true);
    mBtnWhole->setIcon(QIcon(":/icons/whole.png")); // Use your icon
    mBtnWhole->setToolTip("Match Whole Word");

    mBtnBackward = new QToolButton(this);
    mBtnBackward->setCheckable(true);
    mBtnBackward->setIcon(QIcon(":/icons/backward.png")); // Use your icon
    mBtnBackward->setToolTip("Search Backward");

    // Layout for buttons
    QWidget* buttonWidget = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(buttonWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(mBtnCase);
    layout->addWidget(mBtnWhole);
    layout->addWidget(mBtnBackward);

    // Place the widget inside the line edit
    setTextMargins(0, 0, buttonWidget->sizeHint().width(), 0);
    buttonWidget->setFixedHeight(this->sizeHint().height());
    buttonWidget->move(rect().right() - buttonWidget->width(), 0);
    buttonWidget->show();

    // Update position on resize
    connect(this, &QLineEdit::textChanged, this, [this, buttonWidget]() {
        buttonWidget->move(rect().right() - buttonWidget->width(), 0);
        });
    connect(this, &QLineEdit::resizeEvent, this, [this, buttonWidget](QResizeEvent*) {
        buttonWidget->move(rect().right() - buttonWidget->width(), 0);
        });
}
