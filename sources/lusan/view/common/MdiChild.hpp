#ifndef LUSAN_VIEW_COMMON_MDICHILD_HPP
#define LUSAN_VIEW_COMMON_MDICHILD_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]aregtech.com.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/common/MdiChild.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
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

    virtual ~MdiChild(void);

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the MDI Window type.
     * \return  The MDI Window type.
     **/
    inline MdiChild::eMdiWindow getMdiWindowType(void) const;

    /**
     * \brief   Checks if the MDI child is a Service Interface window.
     * \return  True if it is a Service Interface window, false otherwise.
     **/
    inline bool isServiceInterfaceWindow(void) const;

    /**
     * \brief   Checks if the MDI child is a Log Viewer window.
     * \return  True if it is a Log Viewer window, false otherwise.
     **/
    inline bool isLogViewerWindow(void) const;

    /**
     * \brief   Checks if the MDI child is an Offline Log Viewer window.
     * \return  True if it is an Offline Log Viewer window, false otherwise.
     **/
    inline bool isOfflineLogViewerWindow(void) const;

//////////////////////////////////////////////////////////////////////////
// Actions
//////////////////////////////////////////////////////////////////////////
public: 
    /**
     * \brief   Creates a new file.
     **/
    void newFile();

    /**
     * \brief   Loads a file.
     * \param   fileName    The name of the file to load.
     * \return  True if the file was successfully loaded, false otherwise.
     **/
    bool loadFile(const QString& fileName);

    /**
     * \brief   Saves the current file.
     * \return  True if the file was successfully saved, false otherwise.
     **/
    bool save();

    /**
     * \brief   Saves the current file with a new name.
     * \return  True if the file was successfully saved, false otherwise.
     **/
    bool saveAs();

    /**
     * \brief   Saves the file with the specified name.
     * \param   fileName    The name of the file to save.
     * \return  True if the file was successfully saved, false otherwise.
     **/
    bool saveFile(const QString& fileName);

    /**
     * \brief   Gets a user-friendly version of the current file name.
     * \return  The user-friendly file name.
     **/
    QString userFriendlyCurrentFile();

    /**
     * \brief   Gets the current file name.
     * \return  The current file name.
     **/
    inline const QString & currentFile(void) const;

    void cut();
    void copy();
    void paste();

    void undo();
    void redo();

    void clear();
    void selectAll();
    
    void zoomIn(int range = 1);
    void zoomOut(int range = 1);
    
    void copyAvailable(bool available);

    /**
     * \brief   Returns the document name.
     **/
    inline const QString& getDocumentName(void) const;

    /**
     * \brief   Returns the MDI subwindow.
     **/
    inline QMdiSubWindow* getMdiSubwindow(void) const;

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
    virtual bool openSucceeded(void) const;

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
    
/************************************************************************
 * MdiChild overrides
 ************************************************************************/
protected:

    /**
     * \brief   Returns the default file name of new created document.
     **/
    virtual QString newDocumentName(void);

    /**
     * \brief   Returns the default name of new created document.
     **/
    virtual const QString& newDocument(void) const;

    /**
     * \brief   Returns the default extension of new created document.
     **/
    virtual const QString& newDocumentExt(void) const;

    /**
     * \brief   Returns the default file suffix.
     **/
    virtual const QString& fileSuffix(void) const;

    /**
     * \brief   Returns the default file filter.
     **/
    virtual const QString& fileFilter(void) const;

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
    virtual void closeEvent(QCloseEvent* event) override;

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
    virtual void onWindowActivated(void);

    /**
     * \brief   Called when the MDI child window is created.
     *          This method can be overridden to handle window creation events.
     **/
    virtual void onWindowCreated(void);

protected:
    /**
     * \brief   Prompts the user to save changes if necessary.
     * \return  True if the user chose to save or discard changes, false if the user canceled.
     **/
    bool maybeSave();

    /**
     * \brief   Sets the current file name.
     * \param   fileName    The name of the file.
     **/
    void setCurrentFile(const QString& fileName);

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
    QMdiSubWindow*      mMdiSubWindow;  //!< The MDI subwindow.
    MdiMainWindow*      mMainWindow;    //!< The MDI main window
};

//////////////////////////////////////////////////////////////////////////
// MdiChild class inline methods
//////////////////////////////////////////////////////////////////////////

inline MdiChild::eMdiWindow MdiChild::getMdiWindowType(void) const
{
    return mMdiWindowType;
}

inline bool MdiChild::isServiceInterfaceWindow(void) const
{
    return (mMdiWindowType == MdiServiceInterface);
}

inline bool MdiChild::isLogViewerWindow(void) const
{
    return (mMdiWindowType == MdiLogViewer);
}

inline bool MdiChild::isOfflineLogViewerWindow(void) const
{
    return (mMdiWindowType == MdiOfflineLogViewer);
}

inline const QString & MdiChild::currentFile(void) const
{
    return mCurFile;
}

inline const QString& MdiChild::getDocumentName(void) const
{
    return mDocName;
}

inline QMdiSubWindow* MdiChild::getMdiSubwindow(void) const
{
    return mMdiSubWindow;
}

inline void MdiChild::setMdiSubwindow(QMdiSubWindow * mdiSubwindow)
{
    mMdiSubWindow = mdiSubwindow;
}

#endif // LUSAN_VIEW_COMMON_MDICHILD_HPP
