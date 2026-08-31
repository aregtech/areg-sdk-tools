#ifndef LUSAN_VIEW_COMMON_OVERVIEWPAGE_HPP
#define LUSAN_VIEW_COMMON_OVERVIEWPAGE_HPP
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
 *  \file        lusan/view/common/OverviewPage.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Overview page shared by every document editor.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/common/DocIssue.hpp"
#include "lusan/view/common/IEditCommit.hpp"

#include <QIntValidator>
#include <QList>
#include <QScrollArea>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class OverviewModel;
class QCheckBox;
class QEvent;
class QFormLayout;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;

/**
 * \brief   One quick link of the Overview page: the page it opens, and what it says about it.
 **/
struct OverviewPageLink
{
    int         page;       //!< The page index the host editor switches to.
    QString     name;       //!< The object name of the button.
    QString     text;       //!< The button text.
    QString     tip;        //!< The tool tip of the button.
    QString     hint;       //!< The sentence shown beside the button.
};

/**
 * \brief   What tells one document's Overview page from another's, apart from the row the
 *          document adds itself.
 **/
struct OverviewPageConfig
{
    QString                     headline;           //!< The title above the two panels.
    QString                     versionTitle;       //!< The title of the version group.
    QString                     descriptionHint;    //!< The placeholder of the description box.
    bool                        nameEditable;       //!< False where the name follows the file name.
    QList<OverviewPageLink>     links;              //!< The quick links panel.
};

/**
 * \class   OverviewPage
 * \brief   The Overview page: what the document says about itself -- its name, the version its
 *          author owns, the description, and the deprecation mark with its hint.
 *
 *          Every edit is committed through the model's undo commands, one step per committed
 *          edit rather than one per keystroke, and the fields are refilled from the model on
 *          every Overview notification, so a field cannot drift from what was stored -- after an
 *          undo, a redo or a reload alike.
 *
 *          A document that declares more than the shared rows adds its own row to \ref
 *          getDetailsForm in its constructor and calls \ref refreshAll at the end of it: the
 *          widgets of this page are built before the subclass exists, so a refill started here
 *          would never reach the subclass.
 **/
class OverviewPage : public QScrollArea
                   , public IEditCommit
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Builds the page.
     * \param   model   The Overview model of the document being edited.
     * \param   config  What tells this document's page from the other's.
     * \param   parent  The parent widget.
     **/
    explicit OverviewPage(OverviewModel& model, const OverviewPageConfig& config, QWidget* parent = nullptr);

    virtual ~OverviewPage(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Puts the accent on the field a finding blames. The document is the page, so there
     *          is no element to select first; \a eIssueField::None leaves the page as it is.
     **/
    void revealField(eIssueField field);

    /**
     * \brief   Fills every field from the model. A subclass fills its own row too, and calls this
     *          at the end of its constructor.
     **/
    virtual void refreshAll(void);

    /**
     * \brief   Hands over the text the page is still holding. The fields apply their text when
     *          they lose the focus, which a save from the keyboard never causes.
     **/
    virtual void commitPendingEdits(void) override;

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:
    /**
     * \brief   Emitted when a quick link is clicked, to switch to the given page.
     * \param   page    The target page index, as the host editor numbers its pages.
     **/
    void signalPageLinkClicked(int page);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
protected slots:
    void onNameEdited(const QString& text);
    void onNameCommitted(void);
    void onVersionCommitted(void);
    void onDeprecatedToggled(bool checked);
    void onDeprecateHintCommitted(void);

    //!< Refills the page when the document reports the Overview changed by other means.
    void onOverviewChanged(void);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
protected:
    inline OverviewModel& getModel(void) const;

    //!< The form of the Details group, where a document adds the row it declares itself.
    inline QFormLayout* getDetailsForm(void) const;

    //!< The Details group box, the parent of every row of the form.
    inline QGroupBox* getDetailsGroup(void) const;

    /**
     * \brief   Marks the page as pushing its own edit, so the notification that edit causes does
     *          not refill the field the caret sits in. A subclass wraps its own commit in it.
     **/
    inline void setCommitting(bool committing);
    inline bool isCommitting(void) const;

private:
    void buildUi(const OverviewPageConfig& config);
    QWidget* buildLinksPanel(const QList<OverviewPageLink>& links);
    void setupSignals(void);
    void commitDescription(void);
    void showNameValid(bool valid);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    OverviewModel&      mModel;             //!< The Overview of the document being edited.
    QGroupBox*          mDetails;           //!< The Details group.
    QFormLayout*        mForm;              //!< The rows of the Details group.
    QLineEdit*          mName;
    QLabel*             mNameError;
    QLineEdit*          mMajor;
    QLineEdit*          mMinor;
    QLineEdit*          mPatch;
    QPlainTextEdit*     mDescription;
    QCheckBox*          mDeprecated;
    QLineEdit*          mDeprecateHint;
    QIntValidator       mVersionValidator;
    bool                mCommitting;        //!< True while a page edit is being pushed.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    OverviewPage(const OverviewPage& /*src*/) = delete;
    OverviewPage& operator = (const OverviewPage& /*src*/) = delete;
};

//////////////////////////////////////////////////////////////////////////
// OverviewPage inline methods
//////////////////////////////////////////////////////////////////////////

inline OverviewModel& OverviewPage::getModel(void) const
{
    return mModel;
}

inline QFormLayout* OverviewPage::getDetailsForm(void) const
{
    return mForm;
}

inline QGroupBox* OverviewPage::getDetailsGroup(void) const
{
    return mDetails;
}

inline void OverviewPage::setCommitting(bool committing)
{
    mCommitting = committing;
}

inline bool OverviewPage::isCommitting(void) const
{
    return mCommitting;
}

#endif  // LUSAN_VIEW_COMMON_OVERVIEWPAGE_HPP
