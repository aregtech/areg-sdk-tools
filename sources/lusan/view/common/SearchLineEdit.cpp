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

#include "lusan/common/NELusanCommon.hpp"
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QWidget>

SearchLineEdit::SearchLineEdit(const QList<SearchLineEdit::eToolButton>& addButtons, QSize buttonSize, QWidget* parent)
    : QLineEdit     (parent)
    
    , mIsInitialized(false)
    , mToolButtons  (nullptr)
    , mBtnSearch    (nullptr)
    , mBtnMatchCase (nullptr)
    , mBtnMatchWord (nullptr)
    , mBtnWildCard  (nullptr)
    , mBtnBackward  (nullptr)
    , mButtonFlags  (0)
    , mButtons      ( )
{
    initialize(addButtons, buttonSize);
}

SearchLineEdit::SearchLineEdit(QWidget* parent)
    : QLineEdit     (parent)
    
    , mIsInitialized(false)
    , mToolButtons  (nullptr)
    , mBtnSearch    (nullptr)
    , mBtnMatchCase (nullptr)
    , mBtnMatchWord (nullptr)
    , mBtnWildCard  (nullptr)
    , mBtnBackward  (nullptr)
    , mButtonFlags  (0)
    , mButtons      ( )
{
}

void SearchLineEdit::initialize(const QList<SearchLineEdit::eToolButton> & addButtons, QSize buttonSize /*= QSize(16, 16)*/)
{
    if (mIsInitialized)
        return;
    
    mIsInitialized = true;
    // Layout for buttons
    mToolButtons = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(mToolButtons);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);
    if ((addButtons.isEmpty() == false) && (addButtons[0] != SearchLineEdit::eToolButton::ToolButtonNothing))
    {
        for (auto entry : addButtons)
        {
            switch (entry)
            {
            case SearchLineEdit::eToolButton::ToolButtonSearch:
            {
                
                mButtonFlags |= static_cast<uint32_t>(entry);
                mButtons.push_back(entry);
                mBtnSearch = new QToolButton(this);
                mBtnSearch->setObjectName("buttonSearch");
                mBtnSearch->setEnabled(true);
                mBtnSearch->setCheckable(false);
                mBtnSearch->setMinimumSize(buttonSize);
                mBtnSearch->setMaximumSize(buttonSize);
                mBtnSearch->setAutoFillBackground(true);
                mBtnSearch->setCheckable(true);
                mBtnSearch->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::EditFind));
                mBtnSearch->setShortcut(QString::fromUtf8("Ctrl+F, F3, Alt+F"));
                mBtnSearch->setToolTip(tr("Find text (Ctrl + F, F3, Alt + F)"));
                mBtnSearch->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
                connect(mBtnSearch, &QToolButton::toggled, [this](bool checked) {
                        emit signalButtonSearchClicked(checked);
                        emit signalSearchText(text(), isMatchCaseChecked(), isMatchWordChecked(), isWildCardChecked(), isBackwardChecked());
                        setFocus();
                    });
                layout->addWidget(mBtnSearch, 0, Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
            }
            break;
                
            case SearchLineEdit::eToolButton::ToolButtonMatchCase:
            {
                mButtonFlags |= static_cast<uint32_t>(entry);
                mButtons.push_back(entry);
                mBtnMatchCase = new QToolButton(this);
                mBtnMatchCase->setObjectName("buttonMatchCase");
                mBtnMatchCase->setEnabled(true);
                mBtnMatchCase->setCheckable(true);
                mBtnMatchCase->setChecked(false);                
                mBtnMatchCase->setMinimumSize(buttonSize);
                mBtnMatchCase->setMaximumSize(buttonSize);
                mBtnMatchCase->setAutoFillBackground(true);
                mBtnMatchCase->setIcon(QIcon(QString::fromUtf8(":/icons/search-match-case")));
                mBtnMatchCase->setShortcut(QString::fromUtf8("Ctrl+C"));
                mBtnMatchCase->setToolTip(tr("Find text exact match (Ctrl + C)"));
                mBtnMatchCase->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
                mBtnMatchCase->setStyleSheet(NELusanCommon::getStyleToolbutton());
                layout->addWidget(mBtnMatchCase, 0, Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
                connect(mBtnMatchCase, &QToolButton::toggled, [this](bool checked) {
                        emit signalButtonSearchMatchWordClicked(checked);
                        setFocus();
                    });
            }
            break;
                
            case SearchLineEdit::eToolButton::ToolButtonMatchWord:
            {
                mButtonFlags |= static_cast<uint32_t>(entry);
                mButtons.push_back(entry);
                mBtnMatchWord = new QToolButton(this);
                mBtnMatchWord->setObjectName("buttonMatchWord");
                mBtnMatchWord->setEnabled(true);
                mBtnMatchWord->setCheckable(true);
                mBtnMatchWord->setChecked(false);                
                mBtnMatchWord->setMinimumSize(buttonSize);
                mBtnMatchWord->setMaximumSize(buttonSize);
                mBtnMatchWord->setAutoFillBackground(true);
                mBtnMatchWord->setIcon(QIcon(QString::fromUtf8(":/icons/search-match-word")));
                mBtnMatchWord->setShortcut(QString::fromUtf8("Ctrl+W"));
                mBtnMatchWord->setToolTip(tr("Find text exact match (Ctrl + W)"));
                mBtnMatchWord->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
                mBtnMatchWord->setStyleSheet(NELusanCommon::getStyleToolbutton());
                layout->addWidget(mBtnMatchWord, 0, Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
                connect(mBtnMatchWord, &QToolButton::toggled, [this](bool checked) {
                        emit signalButtonSearchMatchCaseClicked(checked);
                        setFocus();
                    });
            }
            break;
                
            case SearchLineEdit::eToolButton::ToolButtonWildCard:
            {
                mButtonFlags |= static_cast<uint32_t>(entry);
                mButtons.push_back(entry);
                mBtnWildCard = new QToolButton(this);
                mBtnWildCard->setObjectName("buttonWildCard");
                mBtnWildCard->setEnabled(true);
                mBtnWildCard->setCheckable(true);
                mBtnWildCard->setChecked(false);                
                mBtnWildCard->setMinimumSize(buttonSize);
                mBtnWildCard->setMaximumSize(buttonSize);
                mBtnWildCard->setAutoFillBackground(true);
                mBtnWildCard->setIcon(QIcon(QString::fromUtf8(":/icons/search-wild-card")));
                mBtnWildCard->setShortcut(QString::fromUtf8("Alt+R"));
                mBtnWildCard->setToolTip(tr("Search with wild-card (Ctrl + R)"));
                mBtnWildCard->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
                mBtnWildCard->setStyleSheet(NELusanCommon::getStyleToolbutton());
                layout->addWidget(mBtnWildCard, 0, Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
                connect(mBtnWildCard, &QToolButton::toggled, [this](bool checked) {
                        emit signalButtonSearchWildCardClicked(checked);
                        setFocus();
                    });
            }
            break;
                
            case SearchLineEdit::eToolButton::ToolButtonBackward:
            {
                mButtonFlags |= static_cast<uint32_t>(entry);
                mButtons.push_back(entry);
                mBtnBackward = new QToolButton(this);
                mBtnBackward->setObjectName("buttonSearchBackward");
                mBtnBackward->setEnabled(true);
                mBtnBackward->setCheckable(true);
                mBtnBackward->setChecked(false);                
                mBtnBackward->setMinimumSize(buttonSize);
                mBtnBackward->setMaximumSize(buttonSize);
                mBtnBackward->setAutoFillBackground(true);
                mBtnBackward->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoUp));
                mBtnBackward->setShortcut(QString::fromUtf8("Shift+F3"));
                mBtnBackward->setToolTip(tr("Search text backward (Schift+F3)"));
                mBtnBackward->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
                mBtnBackward->setStyleSheet(NELusanCommon::getStyleToolbutton());
                layout->addWidget(mBtnBackward, 0, Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
                connect(mBtnBackward, &QToolButton::toggled, [this](bool checked) {
                        emit signalButtonSearchMatchCaseClicked(checked);
                        setFocus();
                    });
            }
            break;
            
            default:
                break;
            }
        }
    }
    
    this->setMinimumHeight(buttonSize.height() + 3);
    this->setMaximumHeight(buttonSize.height() + 3);
    // Place the widget inside the line edit
    setTextMargins(1, 1, mToolButtons->sizeHint().width() + 1, 1);
    mToolButtons->setFixedHeight(this->sizeHint().height());
    mToolButtons->move(rect().right() - mToolButtons->width(), 0);
    mToolButtons->show();
    
    // Update position on resize
    connect(this, &QLineEdit::textChanged, [this](const QString & newText) {
            mToolButtons->move(rect().right() - mToolButtons->width(), 0);
            emit signalSearchTextChanged(newText);
            emit signalFilterText(text(), isMatchCaseChecked(), isMatchWordChecked(), isWildCardChecked(), isBackwardChecked());
        });
}
    
void SearchLineEdit::resizeEvent(QResizeEvent *event)
{
    QLineEdit::resizeEvent(event);
    if (mToolButtons != nullptr)
    {
        mToolButtons->move(rect().right() - mToolButtons->width(), 0);
    }
}

void SearchLineEdit::keyPressEvent(QKeyEvent* event)
{
    // Handle keyboard shortcuts for search functionality
    if (event->key() == Qt::Key_F && (event->modifiers() & Qt::ControlModifier))
    {
        // Ctrl+F: Focus on search field
        setFocus();
        selectAll();
        event->accept();
        return;
    }
    else if ((event->key() == Qt::Key_F3) || (event->key() == Qt::Key_Return))
    {
        // F3: Find next (same as clicking search button)
        emit signalButtonSearchClicked(true);
        emit signalSearchText(text(), isMatchCaseChecked(), isMatchWordChecked(), isWildCardChecked(), isBackwardChecked());
        setFocus();
        event->accept();
        return;
    }
    else if (event->key() == Qt::Key_Escape)
    {
        // Escape: Clear search field and focus table
        clear();
        event->accept();
    }

    // Pass through to parent class
    QLineEdit::keyPressEvent(event);
}
