#ifndef LUSAN_VIEW_COMMON_PENDINGEDITWATCHER_HPP
#define LUSAN_VIEW_COMMON_PENDINGEDITWATCHER_HPP
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
 *  \file        lusan/view/common/PendingEditWatcher.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, reports text typed into a field but not yet given to the document.
 *
 ************************************************************************/

/************************************************************************
 * Dependencies
 ************************************************************************/
#include <QObject>

#include <QPointer>
#include <QString>

class QWidget;

/**
 * \brief   Tells the document window that the field the caret sits in holds text the document has
 *          not received yet.
 *
 *          The editor fields hand their text over when they lose the focus, so between the first
 *          keystroke and the next click the document is already changed for the author while its
 *          model still says otherwise. The window shows a clean title in that window of time,
 *          which reads as "nothing to save".
 *
 *          Only the field that has the keyboard focus can hold such text, so this keeps the text
 *          that field had when it got the focus and compares. Typing something else reports a
 *          pending edit, typing the original text back withdraws it, and losing the focus ends it
 *          because the field commits at that moment. Programmatic refills settle themselves the
 *          same way, since the answer is a comparison and never a flag.
 *
 *          A field is watched only when it, or a form around it, is marked with \ref watchField.
 *          The mark carries the document the text belongs to, so a panel that lives outside the
 *          document window and rebinds to whichever document is active is still attributed right.
 **/
class PendingEditWatcher : public QObject
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Static methods
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Marks \p field as carrying text of \p document. Marking a form marks every field
     *          inside it, which is what the detail forms use -- a search or filter box outside the
     *          form stays unmarked and never touches the modification mark.
     * \param   field       The field or the form holding fields. Ignored when null.
     * \param   document    The object standing for the document, the same one the watcher of that
     *                      document was created with.
     **/
    static void watchField(QWidget* field, const QObject& document);

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Creates the watcher of one document.
     * \param   document    The object standing for the document. Fields marked with it are watched,
     *                      fields of every other document are left to their own watcher.
     * \param   parent      The owner object.
     **/
    explicit PendingEditWatcher(const QObject& document, QObject* parent = nullptr);

    virtual ~PendingEditWatcher(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Whether a watched field currently holds text the document has not received.
     **/
    inline bool hasPendingEdit(void) const;

    /**
     * \brief   Takes what the watched field holds as given to the document. Called after the
     *          document has collected the open fields itself, so a save started from the keyboard
     *          leaves the title clean while the caret stays where it is.
     **/
    void acceptPendingEdit(void);

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:

    /**
     * \brief   Emitted when the answer of \ref hasPendingEdit changes.
     **/
    void signalPendingEditChanged(bool pending);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:

    void onFocusChanged(QWidget* previous, QWidget* current);
    void onFieldTextChanged(void);

private:

    //!< Starts watching the given field, or stops watching when it is not a field of this document.
    void bindField(QWidget* field);

    //!< Publishes the new answer when it differs from the current one.
    void setPending(bool pending);

    //!< The text of a field, or an empty string for anything this does not know how to read.
    static QString fieldText(const QWidget* field);

    //!< Whether what the field holds was typed into it rather than put there by the page.
    static bool fieldEdited(const QWidget* field);

    //!< The document a field is marked with, walking up to the form that carries the mark.
    static const QObject* documentOf(const QWidget* field);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    const QObject*      mDocument;  //!< The document this watcher answers for.
    QPointer<QWidget>   mField;     //!< The watched field holding the focus, or null.
    QString             mBaseline;  //!< What that field held when it got the focus, or when a save took it.
    bool                mPending;   //!< The published answer.
};

//////////////////////////////////////////////////////////////////////////
// PendingEditWatcher class inline methods
//////////////////////////////////////////////////////////////////////////

inline bool PendingEditWatcher::hasPendingEdit(void) const
{
    return mPending;
}

#endif  // LUSAN_VIEW_COMMON_PENDINGEDITWATCHER_HPP
