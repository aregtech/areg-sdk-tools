#ifndef LUSAN_DATA_COMMON_WORKSPACEENTRY_HPP
#define LUSAN_DATA_COMMON_WORKSPACEENTRY_HPP
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
 *  \file        lusan/data/common/WorkspaceEntry.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
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
public:
    /**
     * \brief   Default constructor.
     **/
    WorkspaceEntry(void);

    /**
     * \brief   Parameterized constructor.
     * \param   root        The root directory of the workspace.
     * \param   description The description of the workspace.
     * \param   id          The ID of the workspace.
     **/
    WorkspaceEntry(const QString& root, const QString& description, uint32_t id = 0);

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

public:
    /**
     * \brief   Copy assignment operator.
     * \param   src         The source workspace model to copy from.
     * \return  Reference to the assigned workspace model.
     **/
    WorkspaceEntry& operator=(const WorkspaceEntry& src);

    /**
     * \brief   Move assignment operator.
     * \param   src         The source workspace model to move from.
     * \return  Reference to the assigned workspace model.
     **/
    WorkspaceEntry& operator=(WorkspaceEntry&& src) noexcept;

    /**
     * \brief   Equality operator.
     * \param   other       The other workspace model to compare with.
     * \return  True if the models are equal, false otherwise.
     **/
    bool operator==(const WorkspaceEntry& other) const;

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

public:
    /**
     * \brief   Reads the workspace data from an XML stream.
     * \param   xml         The XML stream reader.
     * \return  True if the workspace data was successfully read, false otherwise.
     **/
    bool readWorkspace(QXmlStreamReader& xml);

    /**
     * \brief   Writes the workspace data to an XML stream.
     * \param   xml         The XML stream writer.
     * \return  True if the workspace data was successfully written, false otherwise.
     **/
    bool writeWorkspace(QXmlStreamWriter& xml) const;

    /**
     * \brief   Sets the root directory of the workspace.
     * \param   root        The root directory.
     **/
    inline void setWorkspaceRoot(const QString& root);

    /**
     * \brief   Gets the root directory of the workspace.
     * \return  The root directory.
     **/
    inline const QString& getWorkspaceRoot(void) const;

    /**
     * \brief   Sets the description of the workspace.
     * \param   description The description.
     **/
    inline void setWorkspaceDescription(const QString& description);

    /**
     * \brief   Gets the description of the workspace.
     * \return  The description.
     **/
    inline const QString& getWorkspaceDescription(void) const;

    /**
     * \brief   Sets the sources directory of the workspace.
     * \param   sources     The sources directory.
     **/
    inline void setDirSources(const QString& sources);

    /**
     * \brief   Gets the sources directory of the workspace.
     * \return  The sources directory.
     **/
    inline const QString& getDirSources(void) const;

    /**
     * \brief   Sets the includes directory of the workspace.
     * \param   includes    The includes directory.
     **/
    inline void setDirIncludes(const QString& includes);

    /**
     * \brief   Gets the includes directory of the workspace.
     * \return  The includes directory.
     **/
    inline const QString& getDirIncludes(void) const;

    /**
     * \brief   Sets the delivery directory of the workspace.
     * \param   delivery    The delivery directory.
     **/
    inline void setDirDelivery(const QString& delivery);

    /**
     * \brief   Gets the delivery directory of the workspace.
     * \return  The delivery directory.
     **/
    inline const QString& getDirDelivery(void) const;

    /**
     * \brief   Gets the ID of the workspace.
     * \return  The ID of the workspace.
     **/
    inline uint32_t getId(void) const;

    /**
     * \brief   Sets the ID of the workspace.
     * \param   id          The ID of the workspace.
     **/
    inline void setId(uint32_t id);

    /**
     * \brief   Gets the last accessed timestamp of the workspace.
     * \return  The last accessed timestamp.
     **/
    inline uint64_t getLastAccessed(void) const;

    /**
     * \brief   Activates the workspace by updating the last accessed timestamp.
     * \return  The updated last accessed timestamp.
     **/
    inline uint64_t activate(void);

    /**
     * \brief   Gets the key of the workspace, which is the last accessed timestamp.
     * \return  The last accessed timestamp.
     **/
    inline uint64_t getKey(void) const;

    /**
     * \brief   Checks if the workspace entry is valid.
     * \return  True if the workspace entry is valid, false otherwise.
     **/
    inline bool isValid(void) const;

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
     * \brief   Writes the directories to an XML stream.
     * \param   xml         The XML stream writer.
     **/
    void _writeDirectories(QXmlStreamWriter& xml);

    /**
     * \brief   Writes the settings to an XML stream.
     * \param   xml         The XML stream writer.
     **/
    void _writeSettings(QXmlStreamWriter& xml);

private:
    uint32_t    mId;                //!< The ID of the workspace.
    uint64_t    mLastAccessed;      //!< The last accessed timestamp of the workspace.
    QString     mWorkspaceRoot;     //!< The root directory of the workspace.
    QString     mDescription;       //!< The description of the workspace.
    QString     mSources;           //!< The sources directory of the workspace.
    QString     mIncludes;          //!< The includes directory of the workspace.
    QString     mDelivery;          //!< The delivery directory of the workspace.
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline void WorkspaceEntry::setWorkspaceRoot(const QString& root)
{
    mWorkspaceRoot = root;
}

inline const QString& WorkspaceEntry::getWorkspaceRoot(void) const
{
    return mWorkspaceRoot;
}

inline void WorkspaceEntry::setWorkspaceDescription(const QString& description)
{
    mDescription = description;
}

inline const QString& WorkspaceEntry::getWorkspaceDescription(void) const
{
    return mDescription;
}

inline void WorkspaceEntry::setDirSources(const QString& sources)
{
    mSources = sources;
}

inline const QString& WorkspaceEntry::getDirSources(void) const
{
    return mSources;
}

inline void WorkspaceEntry::setDirIncludes(const QString& includes)
{
    mIncludes = includes;
}

inline const QString& WorkspaceEntry::getDirIncludes(void) const
{
    return mIncludes;
}

inline void WorkspaceEntry::setDirDelivery(const QString& delivery)
{
    mDelivery = delivery;
}

inline const QString& WorkspaceEntry::getDirDelivery(void) const
{
    return mDelivery;
}

inline uint32_t WorkspaceEntry::getId(void) const
{
    return mId;
}

inline void WorkspaceEntry::setId(uint32_t id)
{
    mId = id;
}

inline uint64_t WorkspaceEntry::getLastAccessed(void) const
{
    return mLastAccessed;
}

inline uint64_t WorkspaceEntry::activate(void)
{
    mLastAccessed = NELusanCommon::getTimestamp();
    return mLastAccessed;
}

inline uint64_t WorkspaceEntry::getKey(void) const
{
    return mLastAccessed;
}

inline bool WorkspaceEntry::isValid(void) const
{
    return (mId != 0) && (mLastAccessed != 0);
}

#endif  // LUSAN_DATA_COMMON_WORKSPACEENTRY_HPP
