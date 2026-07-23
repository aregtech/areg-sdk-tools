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
 *  \file        lusan/view/sm/SMAccordion.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, collapsible section stack with a compact (exclusive) mode.
 *
 ************************************************************************/

#include "lusan/view/sm/SMAccordion.hpp"

#include <QAbstractButton>
#include <QIcon>
#include <QStyleOptionToolBox>
#include <QStylePainter>
#include <QVBoxLayout>

namespace
{
    /**
     * \class   SectionHeader
     * \brief   One accordion header, painted by the PLATFORM STYLE as a tool-box tab
     *          (`CE_ToolBoxTabShape` + `CE_ToolBoxTabLabel`) rather than as a rectangle of our own
     *          drawing. That primitive is what gives the header its stepped tab outline -- the rule
     *          running along the top of the label and sloping away to the right -- which is the look
     *          the Conditions tab had while it was a real QToolBox, and which a stylesheet on a
     *          QToolButton cannot reproduce.
     *          A button, not a QToolBox tab, because the accordion must be able to keep several
     *          sections open at once, which QToolBox structurally cannot.
     **/
    class SectionHeader : public QAbstractButton
    {
    public:
        explicit SectionHeader(QWidget* parent)
            : QAbstractButton   (parent)
            , mPosition         (QStyleOptionToolBox::OnlyOneTab)
        {
            setCheckable(true);
            setCursor(Qt::PointingHandCursor);      // visibly clickable
            setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            setAttribute(Qt::WA_Hover, true);       // so the style paints the hover state
        }

        //!< Where this header sits in the stack; the style rounds/steps the ends differently.
        void setTabPosition(QStyleOptionToolBox::TabPosition position)
        {
            mPosition = position;
            update();
        }

        virtual QSize sizeHint() const override
        {
            const QFontMetrics metrics(font());
            int height = metrics.height();
            if (icon().isNull() == false)
            {
                height = qMax(height, iconSize().height());
            }

            return QSize(metrics.horizontalAdvance(text()) + iconSize().width() + 24, height + 10);
        }

    protected:
        virtual void paintEvent(QPaintEvent*) override
        {
            QStylePainter painter(this);
            QStyleOptionToolBox option;
            option.initFrom(this);
            option.text             = text();
            option.icon             = icon();
            option.position         = mPosition;
            option.selectedPosition = QStyleOptionToolBox::NotAdjacent;
            if (isChecked())
            {
                option.state |= QStyle::State_Selected;
            }

            painter.drawControl(QStyle::CE_ToolBoxTabShape, option);
            painter.drawControl(QStyle::CE_ToolBoxTabLabel, option);
        }

    private:
        QStyleOptionToolBox::TabPosition    mPosition;
    };
}

//////////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////////

SMAccordion::SMAccordion(QWidget* parent /*= nullptr*/)
    : QWidget   (parent)
    , mLayout   (nullptr)
    , mSections ( )
    , mCompact  (true)
{
    mLayout = new QVBoxLayout(this);
    mLayout->setContentsMargins(0, 0, 0, 0);
    // No gap between the bars: consecutive headers stack into one ruled column, as QToolBox tabs do.
    mLayout->setSpacing(0);
}

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////

int SMAccordion::addSection(const QIcon& icon, const QString& title, QWidget* content)
{
    const int index = mSections.size();

    SectionHeader* header = new SectionHeader(this);
    header->setObjectName(QStringLiteral("smAccordionHeader%1").arg(index));
    header->setText(title);
    header->setIcon(icon);

    content->setParent(this);

    // A section's content must never drive the accordion's WIDTH: an Ignored horizontal policy
    // pins its minimum width to 0, so opening or closing a section changes the height only. Without
    // this the hosting dock grew when a section expanded (its form's minimum width became visible)
    // and shrank back when it collapsed, so every toggle resized the whole Properties panel.
    QSizePolicy policy = content->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Ignored);
    content->setSizePolicy(policy);

    mLayout->addWidget(header);
    mLayout->addWidget(content);

    Section section;
    section.header  = header;
    section.content = content;
    mSections.append(section);

    connect(header, &QAbstractButton::clicked, this, [this, index](bool checked)
    {
        setSectionOpen(index, checked);
    });

    // Sections start collapsed; the owner opens the one it wants after building them all.
    applyState(index, false);
    updateTabPositions();
    return index;
}

void SMAccordion::updateTabPositions()
{
    // The style steps the first and last tabs differently from the ones between them.
    const int count = mSections.size();
    for (int i = 0; i < count; ++i)
    {
        QStyleOptionToolBox::TabPosition position = QStyleOptionToolBox::Middle;
        if (count == 1)         { position = QStyleOptionToolBox::OnlyOneTab; }
        else if (i == 0)        { position = QStyleOptionToolBox::Beginning; }
        else if (i == count - 1){ position = QStyleOptionToolBox::End; }

        static_cast<SectionHeader*>(mSections.at(i).header)->setTabPosition(position);
    }
}

int SMAccordion::count() const
{
    return mSections.size();
}

bool SMAccordion::isSectionOpen(int index) const
{
    return ((index >= 0) && (index < mSections.size())) ? mSections.at(index).content->isVisible() : false;
}

void SMAccordion::setSectionOpen(int index, bool open)
{
    if ((index < 0) || (index >= mSections.size()))
    {
        return;
    }

    if (mCompact)
    {
        if (open == false)
        {
            // Refuse to leave the accordion fully closed in compact mode: re-check the header so
            // the button state never contradicts what is on screen.
            mSections.at(index).header->setChecked(isSectionOpen(index));
            return;
        }

        for (int i = 0; i < mSections.size(); ++i)
        {
            if (i != index)
            {
                applyState(i, false);
            }
        }
    }

    applyState(index, open);
}

void SMAccordion::setCurrentIndex(int index)
{
    setSectionOpen(index, true);
}

int SMAccordion::currentIndex() const
{
    for (int i = 0; i < mSections.size(); ++i)
    {
        if (isSectionOpen(i))
        {
            return i;
        }
    }

    return -1;
}

bool SMAccordion::isCompact() const
{
    return mCompact;
}

void SMAccordion::setCompact(bool compact)
{
    if (mCompact == compact)
    {
        return;
    }

    mCompact = compact;
    if (compact == false)
    {
        return;     // nothing closes: the developer now manages the sections by hand
    }

    // Collapsing back to compact keeps the first open section and closes the rest.
    const int keep = currentIndex();
    for (int i = 0; i < mSections.size(); ++i)
    {
        if (i != keep)
        {
            applyState(i, false);
        }
    }
}

QAbstractButton* SMAccordion::header(int index) const
{
    return ((index >= 0) && (index < mSections.size())) ? mSections.at(index).header : nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////

void SMAccordion::applyState(int index, bool open)
{
    const Section& section = mSections.at(index);
    const bool changed = (section.content->isVisible() != open);

    section.header->setChecked(open);
    section.content->setVisible(open);

    if (changed)
    {
        emit sectionToggled(index, open);
    }
}
