#ifndef LUSAN_COMMON_MDICHILD_HPP
#define LUSAN_COMMON_MDICHILD_HPP
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
 *  \file        lusan/common/MdiChild.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application Multi-document interface (MDI) child window.
 *
 ************************************************************************/

#include <QAbstractScrollArea>

/**
 * \brief   The MdiChild class represents a child window in the MDI interface.
 *          It provides functionalities for file operations and text editing.
 **/
class MdiChild : public QAbstractScrollArea
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Constructor for MdiChild.
     **/
    MdiChild(QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// actions
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

protected:
    /**
     * \brief   Handles the close event.
     * \param   event    The close event.
     **/
    virtual void closeEvent(QCloseEvent* event) override;

private slots:
    /**
     * \brief   Slot called when the document is modified.
     **/
    void documentWasModified();

private:
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

//////////////////////////////////////////////////////////////////////////
// member variables
//////////////////////////////////////////////////////////////////////////
private:
    //!< The current file name.
    QString mCurFile;

    //!< Indicates whether the file is untitled.
    bool    mIsUntitled;
};

inline const QString & MdiChild::currentFile(void) const
{
    return mCurFile;
}

#endif // LUSAN_COMMON_MDICHILD_HPP
