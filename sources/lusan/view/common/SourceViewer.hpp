#ifndef LUSAN_VIEW_COMMON_SOURCEVIEWER_HPP
#define LUSAN_VIEW_COMMON_SOURCEVIEWER_HPP
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
 *  \file        lusan/view/common/SourceViewer.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, read-only viewer for plain text and source files.
 *
 ************************************************************************/

#include "lusan/view/common/MdiChild.hpp"

#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class CppSyntaxHighlighter;
class MdiMainWindow;
class QPlainTextEdit;

//////////////////////////////////////////////////////////////////////////
// SourceViewer class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   The document window for the files Lusan understands but does not edit: the C and
 *          C++ sources and headers the navigation tree lists next to the service interfaces,
 *          and any other text file the user opens. Without it, asking to open a header did
 *          nothing at all -- the request reached MdiMainWindow and fell off the end of the
 *          extension dispatch.
 *
 *          It is deliberately a viewer and not an editor. Lusan generates and consumes those
 *          sources, it does not own them, so there is nothing here to save and no risk of an
 *          accidental keystroke reaching a file that another tool is the author of. C++ syntax
 *          colouring is applied when the suffix says the file is C or C++; everything else is
 *          shown as monospaced plain text.
 **/
class SourceViewer : public MdiChild
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal constants
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   The largest file the viewer accepts, in bytes. QPlainTextEdit lays out the whole
     *          document up front, so a text window is simply the wrong instrument beyond this
     *          size: it would block the UI for as long as the layout takes. Refusing with a
     *          message beats appearing to hang.
     **/
    static constexpr qint64 MaxViewSize{ 8 * 1024 * 1024 };

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Creates the viewer and reads the given file into it.
     * \param   wndMain     The main window of the application.
     * \param   filePath    The file to show. An empty path creates an empty window.
     * \param   parent      The parent widget.
     **/
    SourceViewer(MdiMainWindow* wndMain, const QString& filePath, QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns why the file could not be shown, or an empty string when it could.
     *          The caller reports this instead of composing its own guess.
     **/
    inline const QString& errorText() const;

    /**
     * \brief   Returns the text control holding the file content.
     **/
    inline QPlainTextEdit* ctrlText() const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:

    //!< Returns true when the file was read and is on screen.
    bool openSucceeded() const override;

    //!< Reads the given file into the view. Replaces whatever was shown before.
    bool loadFile(const QString& fileName) override;

    /**
     * \brief   Saving is refused: this window never modifies what it shows, so there is
     *          nothing to write back and no "Save As" that would not be a plain file copy.
     **/
    bool save() override;
    bool saveAs() override;
    bool saveFile(const QString& fileName) override;

    /**
     * \brief   Only copy does anything -- cut and paste have no target in a read-only view.
     **/
    void cut() override;
    void copy() override;
    void paste() override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:

    //!< Builds the text control.
    void setupWidgets();

    /**
     * \brief   Reads the file and hands its text to the control, or fills mError and leaves the
     *          view empty. Refuses a directory, a file too large to lay out, and a file whose
     *          head holds a NUL byte -- the practical test for "this is not text".
     * \param   filePath    The file to read.
     * \return  True when the content is on screen.
     **/
    bool readFile(const QString& filePath);

    /**
     * \brief   Whether the given file suffix is one of the C / C++ suffixes, and so worth
     *          colouring. Everything else is shown as plain text rather than coloured by rules
     *          that do not apply to it.
     * \param   suffix  The file suffix, without the dot.
     **/
    static bool isCppSuffix(const QString& suffix);

//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private:
    QPlainTextEdit*         mText;          //!< The read-only monospaced text control.
    CppSyntaxHighlighter*   mHighlighter;   //!< C++ colouring, created only for C / C++ files.
    QString                 mError;         //!< Why the file was refused; empty when it was not.
};

//////////////////////////////////////////////////////////////////////////
// SourceViewer class inline methods
//////////////////////////////////////////////////////////////////////////

inline const QString& SourceViewer::errorText() const
{
    return mError;
}

inline QPlainTextEdit* SourceViewer::ctrlText() const
{
    return mText;
}

#endif  // LUSAN_VIEW_COMMON_SOURCEVIEWER_HPP
