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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/common/SearchLineEdit.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, QLineEdit control with tool-buttons for search.
 *
 ************************************************************************/

#include "lusan/view/common/SearchLineEdit.hpp"

#include "lusan/common/NELusanCommon.hpp"

#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QResizeEvent>
#include <QToolButton>

SearchLineEdit::SearchLineEdit(const QList<SearchLineEdit::eToolButton>& addButtons, QWidget* parent /*= nullptr*/)
    : SearchLineEdit(parent)
{
    initialize(addButtons);
}

SearchLineEdit::SearchLineEdit(QWidget* parent /*= nullptr*/)
    : QLineEdit     (parent)

    , mIsInitialized(false)
    , mMark         (nullptr)
    , mTrailing     (nullptr)
    , mCounter      (nullptr)
    , mBtnClear     (nullptr)
    , mBtnMatchCase (nullptr)
    , mBtnMatchWord (nullptr)
    , mBtnWildCard  (nullptr)
    , mBtnBackward  (nullptr)
    , mTrailWidth   (-1)
{
}

void SearchLineEdit::initialize(const QList<SearchLineEdit::eToolButton>& addButtons)
{
    if (mIsInitialized)
        return;

    mIsInitialized = true;

    mMark = new QLabel(this);
    mMark->setObjectName(QStringLiteral("searchMark"));
    mMark->setPixmap(NELusanCommon::iconSearch(NELusanCommon::SizeBig).pixmap(SearchLineEdit::SEARCH_ICON, SearchLineEdit::SEARCH_ICON));
    mMark->setAttribute(Qt::WidgetAttribute::WA_TransparentForMouseEvents, true);
    mMark->setEnabled(false);

    mTrailing = new QWidget(this);
    QHBoxLayout* trail = new QHBoxLayout(mTrailing);
    trail->setContentsMargins(0, 0, 0, 0);
    trail->setSpacing(1);

    mCounter = new QLabel(mTrailing);
    mCounter->setObjectName(QStringLiteral("searchCounter"));
    mCounter->setEnabled(false);
    mCounter->setVisible(false);
    trail->addWidget(mCounter);

    for (auto entry : addButtons)
    {
        switch (entry)
        {
        case SearchLineEdit::eToolButton::ToolButtonMatchCase:
            mBtnMatchCase = _addOption( NELusanCommon::iconSearchMatchCase(NELusanCommon::SizeBig)
                                      , tr("Match case")
                                      , QStringLiteral("buttonMatchCase"));
            connect(mBtnMatchCase, &QToolButton::toggled, this, [this](bool checked) {
                    emit signalButtonSearchMatchCaseClicked(checked);
                    _emitFilter();
                });
            break;

        case SearchLineEdit::eToolButton::ToolButtonMatchWord:
            mBtnMatchWord = _addOption( NELusanCommon::iconSearchMatchWord(NELusanCommon::SizeBig)
                                      , tr("Match whole word")
                                      , QStringLiteral("buttonMatchWord"));
            connect(mBtnMatchWord, &QToolButton::toggled, this, [this](bool checked) {
                    emit signalButtonSearchMatchWordClicked(checked);
                    _emitFilter();
                });
            break;

        case SearchLineEdit::eToolButton::ToolButtonWildCard:
            mBtnWildCard = _addOption( NELusanCommon::iconSearchWildCard(NELusanCommon::SizeBig)
                                     , tr("Read the text as a pattern")
                                     , QStringLiteral("buttonWildCard"));
            connect(mBtnWildCard, &QToolButton::toggled, this, [this](bool checked) {
                    emit signalButtonSearchWildCardClicked(checked);
                    _emitFilter();
                });
            break;

        case SearchLineEdit::eToolButton::ToolButtonBackward:
            mBtnBackward = _addOption( NELusanCommon::iconGoUp(NELusanCommon::SizeBig)
                                     , tr("Walk the matches backwards (Shift+F3)")
                                     , QStringLiteral("buttonSearchBackward"));
            connect(mBtnBackward, &QToolButton::toggled, this, [this](bool checked) {
                    emit signalButtonSearchBackwardClicked(checked);
                    _emitFilter();
                });
            break;

        default:
            break;
        }
    }

    mBtnClear = _addOption(NELusanCommon::iconClose(NELusanCommon::SizeBig), tr("Clear the search text"), QStringLiteral("buttonSearchClear"));
    mBtnClear->setCheckable(false);
    mBtnClear->setVisible(false);
    connect(mBtnClear, &QToolButton::clicked, this, [this]() { clear(); setFocus(); });

    // The field is at least as tall as its natural height and never shorter than the marks it
    // holds. Clamping it to the button height alone cuts the descenders off the typed text.
    setMinimumHeight(qMax(QLineEdit::sizeHint().height(), SearchLineEdit::SEARCH_BUTTON + (SearchLineEdit::SEARCH_AIR / 2) * 2));

    connect(this, &QLineEdit::textChanged, this, [this](const QString& newText) {
            mBtnClear->setVisible(newText.isEmpty() == false);
            _placeMarks();
            emit signalSearchTextChanged(newText);
            _emitFilter();
        });

    _placeMarks();
    mTrailing->show();
    mMark->show();
}

QToolButton* SearchLineEdit::_addOption(const QIcon& icon, const QString& toolTip, const QString& name)
{
    QToolButton* button = new QToolButton(mTrailing);
    button->setObjectName(name);
    button->setIcon(icon);
    button->setIconSize(QSize(SearchLineEdit::SEARCH_ICON, SearchLineEdit::SEARCH_ICON));
    button->setFixedSize(SearchLineEdit::SEARCH_BUTTON, SearchLineEdit::SEARCH_BUTTON);
    button->setCheckable(true);
    button->setChecked(false);
    button->setAutoRaise(true);
    button->setToolTip(toolTip);
    button->setStatusTip(toolTip);
    button->setAccessibleName(toolTip);
    button->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
    button->setStyleSheet(NELusanCommon::getStyleToolbutton());
    static_cast<QHBoxLayout*>(mTrailing->layout())->addWidget(button);
    return button;
}

void SearchLineEdit::setCounter(const QString& text)
{
    if (mCounter == nullptr)
        return;

    mCounter->setText(text);
    mCounter->setVisible(text.isEmpty() == false);
    _placeMarks();
}

void SearchLineEdit::_emitFilter(void)
{
    emit signalFilterText(text(), isMatchCaseChecked(), isMatchWordChecked(), isWildCardChecked(), isBackwardChecked());
}

void SearchLineEdit::_placeMarks(void)
{
    if (mTrailing == nullptr)
        return;

    const int box{ height() };
    const QSize markHint{ mMark->sizeHint() };
    mMark->setGeometry(SearchLineEdit::SEARCH_AIR, (box - markHint.height()) / 2, markHint.width(), markHint.height());

    const int trailWidth{ mTrailing->sizeHint().width() };
    mTrailing->setGeometry(width() - trailWidth - SearchLineEdit::SEARCH_AIR, 0, trailWidth, box);

    // Setting the margins lays the field out again, so it is done only when the trailing group
    // actually changed width -- a counter appearing, or the clear button coming and going.
    if (trailWidth != mTrailWidth)
    {
        mTrailWidth = trailWidth;
        const int left{ SearchLineEdit::SEARCH_AIR + markHint.width() + SearchLineEdit::SEARCH_AIR };
        setTextMargins(left, 1, trailWidth + (SearchLineEdit::SEARCH_AIR * 2), 1);
    }
}

void SearchLineEdit::resizeEvent(QResizeEvent* event)
{
    QLineEdit::resizeEvent(event);
    _placeMarks();
}

void SearchLineEdit::keyPressEvent(QKeyEvent* event)
{
    if ((event->key() == Qt::Key_F3) || (event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Enter))
    {
        emit signalButtonSearchClicked(true);
        emit signalSearchText(text(), isMatchCaseChecked(), isMatchWordChecked(), isWildCardChecked(), isBackwardChecked());
        event->accept();
        return;
    }
    else if (event->key() == Qt::Key_Escape)
    {
        clear();
        event->accept();
        return;
    }

    QLineEdit::keyPressEvent(event);
}
