#ifndef LUSAN_VIEW_SM_SMACCORDION_HPP
#define LUSAN_VIEW_SM_SMACCORDION_HPP
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
 *  \file        lusan/view/sm/SMAccordion.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, collapsible section stack with a compact (exclusive) mode.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include <QIcon>
#include <QList>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QAbstractButton;
class QVBoxLayout;

/**
 * \class   SMAccordion
 * \brief   A vertical stack of titled, collapsible sections. It replaces QToolBox because the
 *          Conditions tab needs BOTH behaviours: a compact mode where opening one section closes
 *          the others (the default -- the Properties dock is narrow), and a free mode where the
 *          developer keeps several sections open and closes them by hand.
 *          Each header is painted by the platform style with the very same tool-box tab primitive
 *          QToolBox uses, so the stepped tab outline is identical to what the tab looked like
 *          before; a pointing-hand cursor keeps it visibly clickable.
 **/
class SMAccordion : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMAccordion(QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Appends a section and returns its index. The accordion takes ownership of
     *          \p content, which becomes the collapsible body under the header.
     **/
    int addSection(const QIcon& icon, const QString& title, QWidget* content);

    //!< The number of sections.
    int count() const;

    //!< True when \p index is expanded.
    bool isSectionOpen(int index) const;

    /**
     * \brief   Expands or collapses \p index. In compact mode expanding one section collapses
     *          every other; collapsing the last open section is refused there, because a
     *          fully closed accordion is a dead end the user cannot tell from a broken panel.
     **/
    void setSectionOpen(int index, bool open);

    //!< Expands \p index (compat with the QToolBox API this replaced).
    void setCurrentIndex(int index);

    //!< The first expanded section, or -1 when all are collapsed.
    int currentIndex() const;

    //!< True when opening a section auto-closes the others.
    bool isCompact() const;

    //!< Sets compact mode; switching it ON collapses everything but the current section.
    void setCompact(bool compact);

    //!< The header button of \p index (styling / tests), or nullptr.
    QAbstractButton* header(int index) const;

signals:
    //!< A section expanded or collapsed (by the user or programmatically).
    void sectionToggled(int index, bool open);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \struct  Section
     * \brief   One header + body pair.
     **/
    struct Section
    {
        QAbstractButton* header { nullptr };
        QWidget*        content { nullptr };
    };

    //!< Applies the open/closed state of \p index to its header and body.
    void applyState(int index, bool open);

    //!< Re-tags every header as first / middle / last, so the style steps the ends correctly.
    void updateTabPositions();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QVBoxLayout*    mLayout;    //!< The section stack.
    QList<Section>  mSections;  //!< The sections in display order.
    bool            mCompact;   //!< True while opening one section closes the others.
};

#endif  // LUSAN_VIEW_SM_SMACCORDION_HPP
