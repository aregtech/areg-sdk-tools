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

class MdiMainWindow;
class QMdiSubWindow;

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
        , MdiLogViewer              //!< Log Viewer MDI Window type
        , MdiOfflineLogViewer       //!< Offline Log Viewer MDI window type
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
     * \brief   Checks if the MDI child is a Log Viewer window.
     * \return  True if it is a Log Viewer window, false otherwise.
     **/
    inline bool isLogViewerWindow() const;

    /**
     * \brief   Checks if the MDI child is an Offline Log Viewer window.
     * \return  True if it is an Offline Log Viewer window, false otherwise.
     **/
    inline bool isOfflineLogViewerWindow() const;

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

    inline bool isModified() const;
    virtual void setModified(bool modified);

    void clear();
    void selectAll();
    
    void zoomIn(int range = 1);
    void zoomOut(int range = 1);
    
    /**
     * \brief   Returns the document name.
     **/
    inline const QString& getDocumentName() const;

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
     * \brief   Reads the document from the file.
     * \param   filePath    The path of the file to read.
     * \return  True if the document was successfully read, false otherwise.
     **/
    virtual bool writeToFile(const QString& filePath);

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
    
protected:
    /**
     * \brief   Strips the path from the file name.
     * \param   fullFileName    The full file name with path.
     * \return  The file name without path.
     **/
    QString strippedName(const QString& fullFileName);

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

inline bool MdiChild::isLogViewerWindow() const
{
    return (mMdiWindowType == MdiLogViewer);
}

inline bool MdiChild::isOfflineLogViewerWindow() const
{
    return (mMdiWindowType == MdiOfflineLogViewer);
}

inline const QString & MdiChild::currentFile() const
{
    return mCurFile;
}

inline bool MdiChild::isModified() const
{
    return mIsModified;
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
