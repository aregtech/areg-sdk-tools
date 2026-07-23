#ifndef LUSAN_VIEW_SM_SMSECTIONCHROME_HPP
#define LUSAN_VIEW_SM_SMSECTIONCHROME_HPP
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
 *  \file        lusan/view/sm/SMSectionChrome.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the shared section chrome: a header strip of section
 *               jump buttons plus a compact toggle over an SMAccordion.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include <QList>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QFrame;
class QHBoxLayout;
class QIcon;
class QLabel;
class QToolButton;
class QVBoxLayout;
class SMAccordion;

/**
 * \class   SMSectionChrome
 * \brief   The shared "accordion chrome" every Properties tab wears: a header strip that
 *          carries one icon-only jump button per accordion section plus the compact toggle,
 *          sitting over an SMAccordion. The Conditions tab (SMGuardBar) grew this by hand;
 *          this control lifts it out verbatim so the State-Actions and General tabs paint the
 *          exact same header and behave identically, with the header construction, the
 *          button<->section sync, the Alt+N jump shortcuts and the compact wiring living in
 *          one place instead of being copied per tab.
 *
 *          Layout, top to bottom: the header row (title label, section jump buttons, compact
 *          toggle, a divider, a stretch, then any host action buttons), an optional body slot
 *          (a host editor + status line), the accordion, and an optional footer slot. A host
 *          only ever calls \ref addSection plus the slot helpers -- it never re-wires the
 *          buttons.
 **/
class SMSectionChrome : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMSectionChrome(QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Appends an accordion section and its matching header jump button, wires the two
     *          together (and an Alt+N shortcut), and returns the section index. The chrome takes
     *          ownership of \p content.
     * \param   icon     The section icon; the jump button carries the same glyph.
     * \param   title    The accordion header text.
     * \param   content  The collapsible body widget.
     * \param   jumpTip  The jump button tooltip; falls back to \p title when empty.
     **/
    int addSection(const QIcon& icon, const QString& title, QWidget* content, const QString& jumpTip = QString());

    //!< Sets the leading header label (empty hides it).
    void setTitle(const QString& text);

    //!< Appends a host action widget to the trailing end of the header (after the stretch).
    void addHeaderWidget(QWidget* widget);

    //!< Stacks a host widget between the header and the accordion (e.g. an editor + status line).
    void addBodyWidget(QWidget* widget);

    //!< Stacks a host widget below the accordion.
    void addFooterWidget(QWidget* widget);

    //!< Adds a trailing stretch below the accordion so the content stays top-aligned.
    void addFooterStretch();

    //!< The underlying accordion (styling / tests / setCurrentIndex).
    SMAccordion* accordion() const;

    //!< The jump button of \p index, or nullptr.
    QToolButton* sectionButton(int index) const;

    //!< The compact toggle button.
    QToolButton* compactButton() const;

    //!< True when the accordion keeps only one section open at a time.
    bool isCompact() const;

    //!< Sets compact mode on both the toggle and the accordion.
    void setCompact(bool compact);

    //!< Expands \p index.
    void setCurrentSection(int index);

    /**
     * \brief   Expands every section at once, so a tab opens ready to read and edit rather than as
     *          a column of closed bars. Only meaningful outside compact mode (which structurally
     *          keeps one section open); a compact chrome is left alone.
     **/
    void openAllSections();

    //!< The first expanded section, or -1.
    int currentSection() const;

    //!< The last section that was opened (remember-last, per tab).
    int lastSection() const;

signals:
    //!< A section expanded or collapsed; hosts hook this for section-specific reactions.
    void sectionActivated(int index, bool open);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    //!< Builds one icon-only, auto-raised, checkable-or-not tool button in the chrome's idiom.
    QToolButton* makeTool(const QString& name, const QIcon& icon, const QString& tip, bool checkable);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QVBoxLayout*        mOuter;       //!< The vertical stack: header, body, accordion, footer.
    QHBoxLayout*        mHeader;      //!< The header row.
    QHBoxLayout*        mSectionBox;  //!< Holds the section jump buttons.
    QHBoxLayout*        mTrailingBox; //!< Holds host action buttons (after the stretch).
    QVBoxLayout*        mBodyBox;     //!< Host widgets between the header and the accordion.
    QVBoxLayout*        mFooterBox;   //!< Host widgets below the accordion.
    QLabel*             mTitle;       //!< The leading header label.
    QToolButton*        mCompactBtn;  //!< The compact-mode toggle.
    QFrame*             mDivider;     //!< The rule between the section controls and host actions.
    SMAccordion*        mAccordion;   //!< The section stack.
    QList<QToolButton*> mSectionBtns; //!< One jump button per section, in display order.
    int                 mLastSection; //!< The last-open section (remember-last).
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline SMAccordion* SMSectionChrome::accordion() const
{
    return mAccordion;
}

inline QToolButton* SMSectionChrome::compactButton() const
{
    return mCompactBtn;
}

inline int SMSectionChrome::lastSection() const
{
    return mLastSection;
}

#endif  // LUSAN_VIEW_SM_SMSECTIONCHROME_HPP
