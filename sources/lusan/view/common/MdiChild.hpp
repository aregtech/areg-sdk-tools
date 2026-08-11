#ifndef LUSAN_VIEW_COMMON_MDICHILD_HPP
#define LUSAN_VIEW_COMMON_MDICHILD_HPP
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
 *  \file        lusan/view/common/MdiChild.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application Multi-document interface (MDI) child window.
 *
 ************************************************************************/

#include <QWidget>

#include "lusan/model/common/DocModelNotifier.hpp"

#include <QDateTime>

class IEDocumentModel;
class MdiMainWindow;
class QMdiSubWindow;
class QTabWidget;

/**
 * \brief   The MdiChild class represents a child window in the MDI interface.
 *          It provides functionalities for file operations and text editing.
 **/
class MdiChild  : public QWidget
{
    friend class MdiMainWindow;

    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:
    //! \brief   MDI Window type.
    enum eMdiWindow
    {
          MdiUnknown            = 0 //!< Unknown MDI Window type
        , MdiServiceInterface       //!< Service Interface MDI Window type
        , MdiStateMachine           //!< State Machine MDI Window type
        , MdiDataType               //!< Data Type document MDI Window type
        , MdiLogViewer              //!< Log Viewer MDI Window type
        , MdiOfflineLogViewer       //!< Offline Log Viewer MDI window type
        , MdiSourceViewer           //!< Read-only text / source file MDI window type
    };

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Constructor for MdiChild.
     **/
    MdiChild(MdiChild::eMdiWindow windowType, MdiMainWindow* wndMain, QWidget* parent = nullptr);

    virtual ~MdiChild();

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the MDI Window type.
     * \return  The MDI Window type.
     **/
    inline MdiChild::eMdiWindow getMdiWindowType() const;

    /**
     * \brief   Checks if the MDI child is a Service Interface window.
     * \return  True if it is a Service Interface window, false otherwise.
     **/
    inline bool isServiceInterfaceWindow() const;

    /**
     * \brief   Checks if the MDI child is a State Machine window.
     * \return  True if it is a State Machine window, false otherwise.
     **/
    inline bool isStateMachineWindow() const;

    /**
     * \brief   Checks if the MDI child is a Data Type document window.
     * \return  True if it is a Data Type document window, false otherwise.
     **/
    inline bool isDataTypeWindow() const;

    /**
     * \brief   Checks if the MDI child is a Log Viewer window.
     * \return  True if it is a Log Viewer window, false otherwise.
     **/
    inline bool isLogViewerWindow() const;

    /**
     * \brief   Checks if the MDI child is an Offline Log Viewer window.
     * \return  True if it is an Offline Log Viewer window, false otherwise.
     **/
    inline bool isOfflineLogViewerWindow() const;

    /**
     * \brief   Checks if the MDI child is a read-only text / source viewer window.
     * \return  True if it is a source viewer window, false otherwise.
     **/
    inline bool isSourceViewerWindow() const;

//////////////////////////////////////////////////////////////////////////
// Actions
//////////////////////////////////////////////////////////////////////////
public: 
    /**
     * \brief   Creates a new file.
     **/
    virtual void newFile();

    /**
     * \brief   Loads a file.
     * \param   fileName    The name of the file to load.
     * \return  True if the file was successfully loaded, false otherwise.
     **/
    virtual bool loadFile(const QString& fileName);

    /**
     * \brief   Saves the current file.
     * \return  True if the file was successfully saved, false otherwise.
     **/
    virtual bool save();

    /**
     * \brief   Saves the current file with a new name.
     * \return  True if the file was successfully saved, false otherwise.
     **/
    virtual bool saveAs();

    /**
     * \brief   Saves the file with the specified name.
     * \param   fileName    The name of the file to save.
     * \return  True if the file was successfully saved, false otherwise.
     **/
    virtual bool saveFile(const QString& fileName);

    /**
     * \brief   Hands the document the text that its open editors are still holding. Fields that
     *          apply on focus loss keep it while the caret is inside them, and a save started
     *          from the keyboard moves no focus. The base document has no such fields.
     **/
    virtual void commitPendingEdits(void);

    /**
     * \brief   Gets a user-friendly version of the current file name.
     * \return  The user-friendly file name.
     **/
    QString userFriendlyCurrentFile();

    /**
     * \brief   Gets the current file name.
     * \return  The current file name.
     **/
    inline const QString & currentFile() const;

    virtual void cut();
    virtual void copy();
    virtual void paste();

    virtual void undo();
    virtual void redo();

    /**
     * \brief   Document-wide search (Ctrl+F) and where-used (Shift+F12), forwarded from the
     *          main window's Edit menu to the active child. The empty defaults keep documents
     *          without a search/reference facility (SI, log viewers) inert; StateMachine
     *          routes them to the currently selected inner page.
     **/
    virtual void find();
    virtual void findUsages();

    /**
     * \brief   Go to Declaration (F12), forwarded from the main window's Edit menu to the active
     *          child. The empty default keeps documents without a reference facility inert;
     *          StateMachine routes it from a selected canvas element to its declaration page.
     **/
    virtual void gotoDefinition();

    /**
     * \brief   Whether the document's undo/redo history currently has a step to apply.
     *          The empty default (false) keeps documents without an undo framework (SI,
     *          log viewers) inert; StateMachine overrides these from its command stack.
     **/
    virtual bool canUndo() const;
    virtual bool canRedo() const;

    /**
     * \brief   Shows or hides the document's own command toolbar (if it has one), and
     *          reports whether it is currently visible. The empty default (always
     *          visible, no-op to hide) keeps documents without such a toolbar inert.
     **/
    virtual void setToolbarVisible(bool visible);
    virtual bool isToolbarVisible() const;

    /**
     * \brief   The document behind the window, for the surfaces that work on any editable
     *          document -- the validation results panel above all. A window that edits no
     *          document (a log viewer) keeps the nullptr default and is simply not listed.
     **/
    virtual IEDocumentModel* documentModel();

    /**
     * \brief   Brings the offending element of a validation finding into view: switches to the
     *          page that owns it and puts the selection, and where the check knows it the caret,
     *          on the thing that has to change.
     * \param   elementId   The unique ID of the element the finding blames.
     * \param   kind        The kind of that element, which decides the page.
     * \param   rule        The check that produced the finding, which often names the field.
     **/
    virtual void navigateToIssue(uint32_t elementId, eDocElementKind kind, int rule);

    inline bool isModified() const;
    virtual void setModified(bool modified);

    /**
     * \brief   True once the window has accepted its close and is only waiting to be deleted.
     *          The widget is still reachable from the MDI area during that window, so anything
     *          that enumerates the open documents must leave it out.
     **/
    inline bool isClosing() const;

    void clear();
    void selectAll();
    
    void zoomIn(int range = 1);
    void zoomOut(int range = 1);
    
    /**
     * \brief   Returns the document name.
     **/
    inline const QString& getDocumentName() const;

    /**
     * \brief   Compares the document's file with what it looked like when it was last read or
     *          written and, when another program has changed it, offers to reload or to keep the
     *          editor as it is. Called by the main window when the watched file reports a change.
     **/
    void checkFileChangedOnDisk();

    /**
     * \brief   Returns the MDI subwindow.
     **/
    inline QMdiSubWindow* getMdiSubwindow() const;

    /**
     * \brief   Sets the MDI subwindow.
     * \param   mdiSubwindow    The MDI subwindow.
     **/
    inline void setMdiSubwindow(QMdiSubWindow * mdiSubwindow);
    
//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    
    /**
     * \brief   Returns the file open operation success flag.
     **/
    virtual bool openSucceeded() const;
    
    /**
     * \brief   Sets the current file name.
     * \param   fileName    The name of the file.
     **/
    virtual void setCurrentFile(const QString& fileName);
    
signals:

/************************************************************************
 * Signals
 ************************************************************************/

    /**
     * \brief   The signal triggered when the MDI child window is closed.
     * \param   mdiChild    The MDI child window that is closed.
     **/
    void signalMdiChildClosed(MdiChild * mdiChild);

    /**
     * \brief   The signal triggered when the MDI child window is created.
     * \param   mdiChild    The MDI child window that is created.
     **/
    void signalMdiChildCreating(MdiChild * mdiChild);

    /**
     * \brief   Emitted when canUndo()/canRedo() changed, so MdiMainWindow can keep the
     *          global Edit menu's Undo/Redo actions in sync with the active child.
     **/
    void signalCanUndoChanged(bool canUndo);
    void signalCanRedoChanged(bool canRedo);

/************************************************************************
 * MdiChild overrides
 ************************************************************************/
protected:

    /**
     * \brief   Returns the default file name of new created document.
     **/
    virtual QString newDocumentName();

    /**
     * \brief   Returns the default name of new created document.
     **/
    virtual const QString& newDocument() const;

    /**
     * \brief   Returns the default extension of new created document.
     **/
    virtual const QString& newDocumentExt() const;

    /**
     * \brief   Returns the default file suffix.
     **/
    virtual const QString& fileSuffix() const;

    /**
     * \brief   Returns the default file filter.
     **/
    virtual const QString& fileFilter() const;

    /**
     * \brief   Returns the file name (or full path) to offer the user in the
     *          "Save As" dialog. The base implementation returns the current file
     *          name (the auto-generated `NewDocumentN.ext` for untitled documents).
     *          Derived classes may override this to suggest a name derived from the
     *          document content (for example, the state machine name).
     **/
    virtual QString suggestedSaveName() const;

    /**
     * \brief   Reads the document from the file.
     * \param   filePath    The path of the file to read.
     * \return  True if the document was successfully read, false otherwise.
     **/
    virtual bool writeToFile(const QString& filePath);

    /**
     * \brief   Names the document after the file it is saved into, but only while it still
     *          carries the generated placeholder. Once the author has named the document, the
     *          name is theirs and saving under any other file name leaves it alone.
     *
     *          The file name is spelled the way C++ can carry it: spaces drop out, an
     *          unspellable character becomes '_', and a leading digit becomes 'N'.
     * \param   filePath    The file the document is being saved into.
     **/
    void seedNameFromFile(const QString& filePath);

    /**
     * \brief   Writes the name into the document's overview and tells the open pages about it.
     *          Bypasses the undo history on purpose: naming a new document and naming a saved
     *          one are not edits the author made. An empty name is ignored.
     * \param   name    The name the document declares from now on.
     **/
    void setDocumentName(const QString& name);

    /**
     * \brief   Handles the close event.
     * \param   event    The close event.
     **/
    void closeEvent(QCloseEvent* event) override;

    /**
     * \brief   Called when the MDI child window is closing.
     *          This method can be overridden to handle window closing events.
     * \param   isActive    Indicates whether the window is active or not.
     **/
    virtual void onWindowClosing(bool isActive);

    /**
     * \brief   Called when the MDI child window is activated.
     *          This method can be overridden to handle window activation events.
     **/
    virtual void onWindowActivated();

    /**
     * \brief   Called when the MDI child window is created.
     *          This method can be overridden to handle window creation events.
     **/
    virtual void onWindowCreated();

    /**
     * \brief   Prompts the user to save changes if necessary.
     * \return  True if the user chose to save or discard changes, false if the user canceled.
     **/
    virtual bool maybeSave();

    /**
     * \brief   Returns the tab widget hosting this document's pages, or nullptr when the document
     *          has no page tabs. Backs the shared Ctrl+PageDown / Ctrl+PageUp page cycling
     *          installed in the constructor; only the paged editors (Service Interface, State
     *          Machine) override it -- log viewers keep the nullptr default and stay inert.
     **/
    virtual QTabWidget* pageTabWidget();

    /**
     * \brief   Switches to the page \p delta steps from the current one, wrapping around the ends.
     *          A no-op when the document has no page tabs or has only a single page.
     **/
    void switchToAdjacentPage(int delta);

protected:
    /**
     * \brief   Strips the path from the file name.
     * \param   fullFileName    The full file name with path.
     * \return  The file name without path.
     **/
    QString strippedName(const QString& fullFileName);

    /**
     * \brief   Records the timestamp and size the document's file has right now. Everything the
     *          editor itself reads or writes goes through here, so a later change of either value
     *          is a change somebody else made.
     **/
    void rememberFileState();

private slots:
    /**
     * \brief   Slot called when the document is modified.
     **/
    void onDocumentModified();

//////////////////////////////////////////////////////////////////////////
// Protected member variables
//////////////////////////////////////////////////////////////////////////
protected:
    const eMdiWindow    mMdiWindowType; //!< MDI Window type
    QString             mCurFile;       //!< The current file name.
    QString             mDocName;       //!< The document name.
    bool                mIsUntitled;    //!< Indicates whether the file is untitled.
    bool                mIsModified;    //!< Indicates the document modification flag.
    bool                mIsClosing;     //!< The close was accepted; the window is on its way out.
    QDateTime           mFileTime;      //!< File timestamp as of the last read or write by the editor.
    qint64              mFileSize;      //!< File size as of the last read or write by the editor.
    bool                mReloadAsked;   //!< A reload prompt for this document is on screen.
    QMdiSubWindow*      mMdiSubWindow;  //!< The MDI subwindow.
    MdiMainWindow*      mMainWindow;    //!< The MDI main window
};

//////////////////////////////////////////////////////////////////////////
// MdiChild class inline methods
//////////////////////////////////////////////////////////////////////////

inline MdiChild::eMdiWindow MdiChild::getMdiWindowType() const
{
    return mMdiWindowType;
}

inline bool MdiChild::isServiceInterfaceWindow() const
{
    return (mMdiWindowType == MdiServiceInterface);
}

inline bool MdiChild::isStateMachineWindow() const
{
    return (mMdiWindowType == MdiStateMachine);
}

inline bool MdiChild::isDataTypeWindow() const
{
    return (mMdiWindowType == MdiDataType);
}

inline bool MdiChild::isLogViewerWindow() const
{
    return (mMdiWindowType == MdiLogViewer);
}

inline bool MdiChild::isOfflineLogViewerWindow() const
{
    return (mMdiWindowType == MdiOfflineLogViewer);
}

inline bool MdiChild::isSourceViewerWindow() const
{
    return (mMdiWindowType == MdiSourceViewer);
}

inline const QString & MdiChild::currentFile() const
{
    return mCurFile;
}

inline bool MdiChild::isModified() const
{
    return mIsModified;
}

inline bool MdiChild::isClosing() const
{
    return mIsClosing;
}

inline const QString& MdiChild::getDocumentName() const
{
    return mDocName;
}

inline QMdiSubWindow* MdiChild::getMdiSubwindow() const
{
    return mMdiSubWindow;
}

inline void MdiChild::setMdiSubwindow(QMdiSubWindow * mdiSubwindow)
{
    mMdiSubWindow = mdiSubwindow;
}

#endif // LUSAN_VIEW_COMMON_MDICHILD_HPP
