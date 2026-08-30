#ifndef LUSAN_DATA_COMMON_WORKSPACEENTRY_HPP
#define LUSAN_DATA_COMMON_WORKSPACEENTRY_HPP
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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/common/WorkspaceEntry.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Dialog to select folder.
 *
 ************************************************************************/

#include "lusan/common/NELusanCommon.hpp"
#include <string>

class QXmlStreamReader;
class QXmlStreamWriter;

/**
 * \class   WorkspaceEntry
 * \brief   Represents a workspace model in the Lusan application.
 **/
class WorkspaceEntry
{
////////////////////////////////////////////////////////////////////////
// Internal types and constants
////////////////////////////////////////////////////////////////////////
public:
    //!< Invalid workspace entry.
    static const WorkspaceEntry InvalidWorkspace;

    /**
     * \brief   Which log window a column record belongs to. Every offline window shares one
     *          record, because they all read the same kind of archive.
     **/
    enum class eLogMode : uint8_t
    {
          LogModeLive       = 0 //!< The window that follows a running collector
        , LogModeOffline    = 1 //!< Every window that reads an archive
    };

    /**
     * \brief   One column of the log table, as the workspace remembers it. The order of the
     *          list is the order of the columns.
     **/
    struct sLogColumn
    {
        QString key;    //!< The stored name of the column, from LoggingModelBase.
        int     width;  //!< The width in pixels, zero when the column keeps the default.
    };

    using ListLogColumns = QList<WorkspaceEntry::sLogColumn>;

////////////////////////////////////////////////////////////////////////
// Static methods
////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the name a workspace takes when none was given: the name of its root
     *          directory.
     * \param   root    The root directory of the workspace.
     **/
    static QString nameFromRoot(const QString& root);

////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    WorkspaceEntry();

    /**
     * \brief   Parameterized constructor.
     * \param   root        The root directory of the workspace.
     * \param   name        The short name shown in the workspace lists.
     * \param   description The description of the workspace.
     * \param   id          The ID of the workspace.
     **/
    WorkspaceEntry(const QString& root, const QString& name, const QString& description, uint32_t id = 0);

    /**
     * \brief   Constructor that initializes the workspace model from an XML stream.
     * \param   xml         The XML stream reader.
     **/
    WorkspaceEntry(QXmlStreamReader& xml);

    /**
     * \brief   Copy constructor.
     * \param   src         The source workspace model to copy from.
     **/
    WorkspaceEntry(const WorkspaceEntry& src);

    /**
     * \brief   Move constructor.
     * \param   src         The source workspace model to move from.
     **/
    WorkspaceEntry(WorkspaceEntry&& src) noexcept;

////////////////////////////////////////////////////////////////////////
// Operators
////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Copy assignment operator.
     * \param   src         The source workspace model to copy from.
     * \return  Reference to the assigned workspace model.
     **/
    WorkspaceEntry& operator = (const WorkspaceEntry& src);

    /**
     * \brief   Move assignment operator.
     * \param   src         The source workspace model to move from.
     * \return  Reference to the assigned workspace model.
     **/
    WorkspaceEntry& operator = (WorkspaceEntry&& src) noexcept;

    /**
     * \brief   Equality operator.
     * \param   other       The other workspace model to compare with.
     * \return  True if the models are equal, false otherwise.
     **/
    bool operator == (const WorkspaceEntry& other) const;

    /**
     * \brief   Greater than operator.
     * \param   other       The other workspace model to compare with.
     * \return  True if this workspace model is greater than the other, false otherwise.
     **/
    bool operator > (const WorkspaceEntry& other) const;

    /**
     * \brief   Less than operator.
     * \param   other       The other workspace model to compare with.
     * \return  True if this workspace model is less than the other, false otherwise.
     **/
    bool operator < (const WorkspaceEntry& other) const;

////////////////////////////////////////////////////////////////////////
// Attributes and operations
////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Reads the workspace data from an XML stream.
     * \param   xml         The XML stream reader.
     * \return  True if the workspace data was successfully read, false otherwise.
     **/
    bool readFromXml(QXmlStreamReader& xml);

    /**
     * \brief   Writes the workspace data to an XML stream.
     * \param   xml         The XML stream writer.
     * \return  True if the workspace data was successfully written, false otherwise.
     **/
    bool writeToXml(QXmlStreamWriter& xml) const;

    /**
     * \brief   Sets the root directory of the workspace.
     * \param   root        The root directory.
     **/
    inline void setWorkspaceRoot(const QString& root);

    /**
     * \brief   Gets the root directory of the workspace.
     * \return  The root directory.
     **/
    inline const QString& getWorkspaceRoot() const;

    /**
     * \brief   Sets the short name of the workspace.
     * \param   name        The name shown in the workspace lists.
     **/
    inline void setWorkspaceName(const QString& name);

    /**
     * \brief   Returns the short name of the workspace. An entry read without a name takes the
     *          name of its root directory, so a valid entry never returns an empty string.
     **/
    inline const QString& getWorkspaceName() const;

    /**
     * \brief   Sets the description of the workspace.
     * \param   description The description.
     **/
    inline void setWorkspaceDescription(const QString& description);

    /**
     * \brief   Gets the description of the workspace.
     * \return  The description.
     **/
    inline const QString& getWorkspaceDescription() const;

    /**
     * \brief   Sets the sources directory of the workspace.
     * \param   sources     The sources directory.
     **/
    inline void setDirSources(const QString& sources);

    /**
     * \brief   Gets the sources directory of the workspace.
     * \return  The sources directory.
     **/
    inline const QString& getDirSources() const;

    /**
     * \brief   Sets the includes directory of the workspace.
     * \param   includes    The includes directory.
     **/
    inline void setDirIncludes(const QString& includes);

    /**
     * \brief   Gets the includes directory of the workspace.
     * \return  The includes directory.
     **/
    inline const QString& getDirIncludes() const;

    /**
     * \brief   Sets the delivery directory of the workspace.
     * \param   delivery    The delivery directory.
     **/
    inline void setDirDelivery(const QString& delivery);

    /**
     * \brief   Gets the delivery directory of the workspace.
     * \return  The delivery directory.
     **/
    inline const QString& getDirDelivery() const;

    /**
     * \brief   Sets the directory to save logging files.
     * \param   logs    The path to the directory to save logs.
     **/
    inline void setDirLogs(const QString& logs);

    /**
     * \brief   Gets the directory to save logging files.
     **/
    inline const QString& getDirLogs() const;

    /**
     * \brief   Sets the columns of a log table, in the order they are drawn.
     * \param   mode    Which kind of log window the columns belong to.
     * \param   columns The columns to remember. An empty list restores the defaults.
     **/
    inline void setLogColumns(WorkspaceEntry::eLogMode mode, const WorkspaceEntry::ListLogColumns& columns);

    /**
     * \brief   Gets the columns of a log table. Empty when none was ever saved.
     * \param   mode    Which kind of log window to read.
     **/
    inline const WorkspaceEntry::ListLogColumns& getLogColumns(WorkspaceEntry::eLogMode mode) const;

    /**
     * \brief   Sets the log database the workspace opened last.
     * \param   path    The full path of the database file.
     **/
    inline void setLogDatabase(const QString& path);

    /**
     * \brief   Gets the log database the workspace opened last.
     **/
    inline const QString& getLogDatabase() const;

    /**
     * \brief   Gets the ID of the workspace.
     * \return  The ID of the workspace.
     **/
    inline uint32_t getId() const;

    /**
     * \brief   Sets the ID of the workspace.
     * \param   id          The ID of the workspace.
     **/
    inline void setId(uint32_t id);

    /**
     * \brief   Gets the last accessed timestamp of the workspace.
     * \return  The last accessed timestamp.
     **/
    inline uint64_t getLastAccessed() const;

    /**
     * \brief   Activates the workspace by updating the last accessed timestamp.
     * \return  The updated last accessed timestamp.
     **/
    inline uint64_t activate();

    /**
     * \brief   Gets the key of the workspace, which is the last accessed timestamp.
     * \return  The last accessed timestamp.
     **/
    inline uint64_t getKey() const;

    /**
     * \brief   Checks if the workspace entry is valid.
     * \return  True if the workspace entry is valid, false otherwise.
     **/
    inline bool isValid() const;

////////////////////////////////////////////////////////////////////////
// Hidden methods
////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   Reads the settings from an XML stream.
     * \param   xml         The XML stream reader.
     **/
    void _readSettings(QXmlStreamReader& xml);

    /**
     * \brief   Reads the directories from an XML stream.
     * \param   xml         The XML stream reader.
     **/
    void _readDirectories(QXmlStreamReader& xml);

    /**
     * \brief   Reads the log window settings: the columns and the database opened last.
     **/
    void _readLogView(QXmlStreamReader& xml);

    /**
     * \brief   Writes the directories to an XML stream.
     * \param   xml         The XML stream writer.
     **/
    void _writeDirectories(QXmlStreamWriter& xml);

    /**
     * \brief   Writes the settings to an XML stream.
     * \param   xml         The XML stream writer.
     **/
    void _writeSettings(QXmlStreamWriter& xml);

////////////////////////////////////////////////////////////////////////
// Member variables
////////////////////////////////////////////////////////////////////////
private:
    uint32_t    mId;                //!< The ID of the workspace.
    uint64_t    mLastAccessed;      //!< The last accessed timestamp of the workspace.
    QString     mWorkspaceRoot;     //!< The root directory of the workspace.
    QString     mName;              //!< The short name of the workspace.
    QString     mDescription;       //!< The description of the workspace.
    QString     mSources;           //!< The sources directory of the workspace.
    QString     mIncludes;          //!< The includes directory of the workspace.
    QString     mDelivery;          //!< The delivery directory of the workspace.
    QString     mLogFiles;          //!< The location of logging files.
    QString     mLogDatabase;       //!< The log database the workspace opened last.
    ListLogColumns mLogColumns;     //!< The columns of the live log table, in the order they are drawn.
    ListLogColumns mLogColumnsFile; //!< The columns every offline log table shares, in the order they are drawn.

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   Writes one column record.
     * \param   xml     The writer to write into.
     * \param   columns The columns to write, in the order they are drawn.
     * \param   mode    The value of the mode attribute. An empty string writes no attribute.
     **/
    void _writeLogColumns(QXmlStreamWriter& xml, const WorkspaceEntry::ListLogColumns& columns, const QString& mode) const;
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline void WorkspaceEntry::setWorkspaceRoot(const QString& root)
{
    mWorkspaceRoot = NELusanCommon::fixPath(root);
}

inline const QString& WorkspaceEntry::getWorkspaceRoot() const
{
    return mWorkspaceRoot;
}

inline void WorkspaceEntry::setWorkspaceName(const QString& name)
{
    mName = name;
}

inline const QString& WorkspaceEntry::getWorkspaceName() const
{
    return mName;
}

inline void WorkspaceEntry::setWorkspaceDescription(const QString& description)
{
    mDescription = description;
}

inline const QString& WorkspaceEntry::getWorkspaceDescription() const
{
    return mDescription;
}

inline void WorkspaceEntry::setDirSources(const QString& sources)
{
    mSources = NELusanCommon::fixPath(sources);
}

inline const QString& WorkspaceEntry::getDirSources() const
{
    return mSources;
}

inline void WorkspaceEntry::setDirIncludes(const QString& includes)
{
    mIncludes = NELusanCommon::fixPath(includes);
}

inline const QString& WorkspaceEntry::getDirIncludes() const
{
    return mIncludes;
}

inline void WorkspaceEntry::setDirDelivery(const QString& delivery)
{
    mDelivery = NELusanCommon::fixPath(delivery);
}

inline const QString& WorkspaceEntry::getDirDelivery() const
{
    return mDelivery;
}

inline void WorkspaceEntry::setDirLogs(const QString& logs)
{
    mLogFiles = NELusanCommon::fixPath(logs);
}

inline const QString& WorkspaceEntry::getDirLogs() const
{
    return mLogFiles;
}

inline void WorkspaceEntry::setLogColumns(WorkspaceEntry::eLogMode mode, const WorkspaceEntry::ListLogColumns& columns)
{
    if (mode == WorkspaceEntry::eLogMode::LogModeLive)
    {
        mLogColumns = columns;
    }
    else
    {
        mLogColumnsFile = columns;
    }
}

inline const WorkspaceEntry::ListLogColumns& WorkspaceEntry::getLogColumns(WorkspaceEntry::eLogMode mode) const
{
    return (mode == WorkspaceEntry::eLogMode::LogModeLive) ? mLogColumns : mLogColumnsFile;
}

inline void WorkspaceEntry::setLogDatabase(const QString& path)
{
    mLogDatabase = NELusanCommon::fixPath(path);
}

inline const QString& WorkspaceEntry::getLogDatabase() const
{
    return mLogDatabase;
}

inline uint32_t WorkspaceEntry::getId() const
{
    return mId;
}

inline void WorkspaceEntry::setId(uint32_t id)
{
    mId = id;
}

inline uint64_t WorkspaceEntry::getLastAccessed() const
{
    return mLastAccessed;
}

inline uint64_t WorkspaceEntry::activate()
{
    mLastAccessed = NELusanCommon::getTimestamp();
    return mLastAccessed;
}

inline uint64_t WorkspaceEntry::getKey() const
{
    return mLastAccessed;
}

inline bool WorkspaceEntry::isValid() const
{
    return (mId != 0) && (mLastAccessed != 0);
}

#endif  // LUSAN_DATA_COMMON_WORKSPACEENTRY_HPP
