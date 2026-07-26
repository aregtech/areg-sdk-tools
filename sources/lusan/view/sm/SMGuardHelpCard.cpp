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
 *  \brief       Lusan application, FSM guard help card.
 *
 ************************************************************************/

#include "lusan/view/sm/SMGuardHelpCard.hpp"

#include "lusan/view/sm/NEGuardStyle.hpp"

#include <QGuiApplication>
#include <QGridLayout>
#include <QLabel>
#include <QScreen>
#include <QScrollArea>
#include <QVBoxLayout>

namespace
{
    constexpr int PopupGap { 2 };

    //!< Word-wraps \p label AND tells the layout its height depends on its width. QLabel implements
    //!< heightForWidth but does not advertise it, so without this every enclosing layout reports
    //!< hasHeightForWidth() == false and the card sizes itself for text that has not wrapped yet.
    void setWrapped(QLabel* label)
    {
        label->setWordWrap(true);
        QSizePolicy policy = label->sizePolicy();
        policy.setHeightForWidth(true);
        label->setSizePolicy(policy);
    }

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

        QLabel* whatLabel = new QLabel(what);
        setWrapped(whatLabel);                  // long descriptions wrap instead of forcing the card wide
        grid->addWidget(glyphLabel, row, 0);
        grid->addWidget(whatLabel, row, 1);
        grid->addWidget(new QLabel(example), row, 2);
    }

    //!< A `gesture -- what it does` legend row (the gesture shown monospace).
    void addGestureRow(QGridLayout* grid, int row, const QString& gesture, const QString& what)
    {
        QLabel* key = new QLabel(gesture);
        key->setStyleSheet(QStringLiteral("font-family: monospace;"));
        QLabel* whatLabel = new QLabel(what);
        setWrapped(whatLabel);                  // long descriptions wrap instead of forcing the card wide
        grid->addWidget(key, row, 0);
        grid->addWidget(whatLabel, row, 1);
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

    QRect screenBoundsFor(const QWidget& anchor)
    {
        QScreen* screen = QGuiApplication::screenAt(anchor.mapToGlobal(anchor.rect().center()));
        if ((screen == nullptr) && (anchor.window() != nullptr))
        {
            screen = anchor.window()->screen();
        }
        if (screen == nullptr)
        {
            screen = QGuiApplication::primaryScreen();
        }

        return (screen != nullptr) ? screen->availableGeometry() : QRect();
    }

    bool fitsHorizontally(int x, int width, const QRect& bounds)
    {
        return bounds.isValid() && (x >= bounds.left()) && ((x + width) <= (bounds.x() + bounds.width()));
    }

    bool fitsVertically(int y, int height, const QRect& bounds)
    {
        return bounds.isValid() && (y >= bounds.top()) && ((y + height) <= (bounds.y() + bounds.height()));
    }
}

SMGuardHelpCard::SMGuardHelpCard(QWidget* parent /*= nullptr*/)
    : QFrame(parent)
    , mContent(nullptr)
{
    setObjectName(QStringLiteral("smGuardHelpCard"));
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Raised);
    buildUi();
}

void SMGuardHelpCard::buildUi()
{
    // The content lives inside a scroll area so the card stays fully readable even when the screen is
    // smaller than the card's natural size: popupAt() caps the card to the screen, and anything that
    // no longer fits becomes scrollable rather than clipped or pushed off-screen.
    QScrollArea* scroll = new QScrollArea(this);
    scroll->setObjectName(QStringLiteral("smGuardHelpScroll"));
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QVBoxLayout* frameLayout = new QVBoxLayout(this);
    frameLayout->setContentsMargins(0, 0, 0, 0);
    frameLayout->setSpacing(0);
    frameLayout->addWidget(scroll);

    QWidget* content = new QWidget(scroll);
    mContent = content;
    scroll->setWidget(content);

    QVBoxLayout* outer = new QVBoxLayout(content);
    outer->setContentsMargins(12, 12, 12, 12);
    outer->setSpacing(8);

    // This card is the ONLY place the editor explains itself: hovering answers what a symbol is and
    // nothing else, so everything a first-time reader needs -- what a condition decides, what it may
    // be written from, what the generator emits, the gestures, the operators -- has to be here.
    QLabel* lead = new QLabel(tr("A condition decides whether a transition may fire. When the stimulus arrives "
                                 "the machine evaluates the condition, and takes the transition only if the "
                                 "result is true. An empty condition always fires."), content);
    setWrapped(lead);
    outer->addWidget(lead);

    QLabel* title = new QLabel(tr("What can a guard use?"), content);
    QFont titleFont = title->font();
    titleFont.setBold(true);
    title->setFont(titleFont);
    outer->addWidget(title);

    // The letters are the ones a committed guard really draws on its chips (NEGuardStyle::ownerGlyph)
    // -- an attribute and a constant genuinely share `f`, and the description is what tells them
    // apart. The card used to list `#` and `K` here, which are the catalog's letters: a legend for
    // badges the user never sees is worse than no legend.
    QGridLayout* uses = new QGridLayout();
    uses->setHorizontalSpacing(16);
    addUseRow(uses, 0, NEGuardStyle::eOwner::Stimulus, NEGuardStyle::ownerGlyph(NEGuardStyle::eOwner::Stimulus)
             , tr("stimulus argument"), QStringLiteral("count"));
    addUseRow(uses, 1, NEGuardStyle::eOwner::Fsm     , NEGuardStyle::ownerGlyph(NEGuardStyle::eOwner::Fsm)
             , tr("FSM attribute"), QStringLiteral("IsNightMode"));
    addUseRow(uses, 2, NEGuardStyle::eOwner::Fsm     , NEGuardStyle::ownerGlyph(NEGuardStyle::eOwner::Fsm)
             , tr("FSM constant"), QStringLiteral("MIN_WAITING"));
    addUseRow(uses, 3, NEGuardStyle::eOwner::Handler , NEGuardStyle::ownerGlyph(NEGuardStyle::eOwner::Handler)
             , tr("handler condition"), QStringLiteral("HasWaiting()"));
    addUseRow(uses, 4, NEGuardStyle::eOwner::Raw     , NEGuardStyle::ownerGlyph(NEGuardStyle::eOwner::Raw)
             , tr("verbatim C++, never checked"), QStringLiteral("mCounter > 3"));
    addUseRow(uses, 5, NEGuardStyle::eOwner::Fsm     , QStringLiteral("{}")
             , tr("lambda"), QStringLiteral("{ ... }"));
    outer->addLayout(uses);

    // The Data catalog keeps its own two letters where a chip can only show one: `f` says "the
    // machine declares it", `#` and `K` say which. Naming the difference costs one line; leaving
    // the reader to notice it costs a hunt for a letter that is not in this table.
    QLabel* catalogNote = new QLabel(tr("The Data catalog narrows f to # attribute / K constant."), content);
    setWrapped(catalogNote);
    QFont noteFont = catalogNote->font();
    noteFont.setItalic(true);
    catalogNote->setFont(noteFont);
    outer->addWidget(catalogNote);

    QLabel* mapTitle = new QLabel(tr("you write        ->  it runs"), content);
    mapTitle->setStyleSheet(QStringLiteral("font-family: monospace; font-weight: bold;"));
    outer->addWidget(mapTitle);

    QGridLayout* maps = new QGridLayout();
    maps->setHorizontalSpacing(12);
    addMapRow(maps, 0, QStringLiteral("my_attribute"),    QStringLiteral("my_attribute()"));
    addMapRow(maps, 1, QStringLiteral("my_condition(x)"), QStringLiteral("handler().my_condition(x)"));
    addMapRow(maps, 2, QStringLiteral("{ return ...; }"), QStringLiteral("inline lambda"));
    addMapRow(maps, 3, QStringLiteral("IsCalmHours(x)"),  QStringLiteral("mIsCalmHours(x)"));
    outer->addLayout(maps);

    // The gesture legend: keys and mouse moves that are real features but were discoverable only by
    // accident (Shift+Enter, the chip icon hot-zone, double-click-to-edit, the Alt section jumps).
    QLabel* gestureTitle = new QLabel(tr("Gestures"), content);
    QFont gestureFont = gestureTitle->font();
    gestureFont.setBold(true);
    gestureTitle->setFont(gestureFont);
    outer->addWidget(gestureTitle);

    QGridLayout* gestures = new QGridLayout();
    gestures->setHorizontalSpacing(16);
    addGestureRow(gestures, 0, QStringLiteral("#"),           tr("pick a symbol (parameter, attribute, constant, condition)"));
    addGestureRow(gestures, 1, QStringLiteral("#kind:"),      tr("filter the picker to one kind"));
    addGestureRow(gestures, 2, QStringLiteral("Enter"),       tr("commit the guard"));
    addGestureRow(gestures, 3, QStringLiteral("Esc"),         tr("revert to the committed guard"));
    addGestureRow(gestures, 4, QStringLiteral("Shift+Enter"), tr("insert a line break while writing"));
    addGestureRow(gestures, 5, tr("hover a chip"),           tr("what it is, and whether it is sound"));
    addGestureRow(gestures, 6, tr("click a chip icon"),      tr("the same, as a card you can click: where used, map args"));
    addGestureRow(gestures, 7, tr("double-click a chip"),    tr("edit it as plain text"));
    addGestureRow(gestures, 8, QStringLiteral("Alt+1..4"),    tr("jump to a section"));
    outer->addLayout(gestures);

    QLabel* opTitle = new QLabel(tr("Operators"), content);
    opTitle->setFont(gestureFont);
    outer->addWidget(opTitle);

    QGridLayout* operators = new QGridLayout();
    operators->setHorizontalSpacing(16);
    addGestureRow(operators, 0, QStringLiteral("==  !=  <  <=  >  >="), tr("compare two values"));
    addGestureRow(operators, 1, QStringLiteral("&&  ||  !"),            tr("combine and negate"));
    addGestureRow(operators, 2, QStringLiteral("(  )"),                 tr("group, when the order matters"));
    outer->addLayout(operators);

    QLabel* feedback = new QLabel(tr("The line under the editor says whether the condition is sound, and stays "
                                     "there while you type. Point at a symbol and a tooltip explains what it is; "
                                     "the Hints box beside that line turns those tooltips off once you no longer "
                                     "need them. This card is always available, from (?) or F1."), content);
    setWrapped(feedback);
    outer->addWidget(feedback);
}

QSize SMGuardHelpCard::sizeHint() const
{
    // QScrollArea caps its own hint at 24 text lines, so asking the layout would open the card
    // part-way through its own content on every screen. The scroll area is here so a SMALL screen
    // can still reach everything, not so a large one has to scroll: popupAt caps to the screen
    // afterwards, which is the only place shrinking belongs.
    if (mContent == nullptr)
    {
        return QFrame::sizeHint();
    }

    // heightForWidth, not the raw hint: several rows carry word-wrapped descriptions, and a wrapped
    // label only knows how tall it is once the width is fixed. Asking for the hint alone opens the
    // card one wrapped line short of its own last section.
    QSize hint = mContent->sizeHint();
    const QLayout* layout = mContent->layout();
    if ((layout != nullptr) && layout->hasHeightForWidth())
    {
        hint.setHeight(qMax(hint.height(), layout->heightForWidth(hint.width())));
    }

    const int frame = 2 * frameWidth();
    return hint + QSize(frame, frame);
}

void SMGuardHelpCard::popupAt(const QWidget& anchor)
{
    // resize(sizeHint()), not adjustSize(): adjustSize clamps a WINDOW to two thirds of the screen,
    // which on a tall card silently cuts off its last section even though the screen has room. The
    // real cap is the one below, against the screen's available geometry.
    resize(sizeHint());
    const QRect anchorRect(anchor.mapToGlobal(QPoint(0, 0)), anchor.size());
    const QRect hostRect = (anchor.window() != nullptr) ? anchor.window()->frameGeometry() : QRect();
    const QRect screenBounds = screenBoundsFor(anchor);

    // Never let the card exceed the visible screen: a card larger than the screen cannot be brought
    // fully on-screen by repositioning alone, so cap it (the scroll area keeps the content reachable).
    // This guarantees the clamped position below leaves the whole popup inside screenBounds.
    if (screenBounds.isValid())
    {
        const int maxW = qMax(0, screenBounds.width() - 2 * PopupGap);
        const int maxH = qMax(0, screenBounds.height() - 2 * PopupGap);
        if ((width() > maxW) || (height() > maxH))
        {
            resize(qMin(width(), maxW), qMin(height(), maxH));
        }
    }

    const QSize cardSize = size();

    const int openRightX = anchorRect.left();
    const int openLeftX  = anchorRect.right() - cardSize.width() + 1;
    int x = (hostRect.isValid() && (anchorRect.center().x() > hostRect.center().x())) ? openLeftX : openRightX;

    if (screenBounds.isValid())
    {
        const int alternateX = (x == openRightX) ? openLeftX : openRightX;
        if (fitsHorizontally(x, cardSize.width(), screenBounds) == false
            && fitsHorizontally(alternateX, cardSize.width(), screenBounds))
        {
            x = alternateX;
        }

        const int minX = screenBounds.left();
        const int maxX = qMax(minX, screenBounds.x() + screenBounds.width() - cardSize.width());
        x = qBound(minX, x, maxX);
    }

    const int belowY = anchorRect.bottom() + PopupGap + 1;
    const int aboveY = anchorRect.top() - cardSize.height() - PopupGap;
    int y = belowY;

    if (screenBounds.isValid())
    {
        if (fitsVertically(y, cardSize.height(), screenBounds) == false
            && fitsVertically(aboveY, cardSize.height(), screenBounds))
        {
            y = aboveY;
        }

        const int minY = screenBounds.top();
        const int maxY = qMax(minY, screenBounds.y() + screenBounds.height() - cardSize.height());
        y = qBound(minY, y, maxY);
    }

    move(x, y);
    show();
    raise();
}
