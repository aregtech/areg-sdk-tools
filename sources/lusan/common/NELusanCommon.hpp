#ifndef LUSAN_COMMON_NELUSANCOMMON_HPP
#define LUSAN_COMMON_NELUSANCOMMON_HPP
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
 *  \file        lusan/common/NELusanCommon.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Dialog to select folder.
 *
 ************************************************************************/

#include <QStringList>
#include <QString>

/**
 * \namespace NELusanCommon
 * \brief     Contains common definitions and utility functions for the Lusan application.
 **/
namespace NELusanCommon
{
    /**
     * \brief   The list of file filters.
     **/
    extern const QStringList FILTERS;

    /**
     * \brief   The application name.
     **/
    extern const QString    APPLICATION;

    /**
     * \brief   The organization name.
     **/
    extern const QString    ORGANIZATION;

    /**
     * \brief   The application version.
     **/
    extern const QString    VERSION;

    /**
     * \brief   The options file name.
     **/
    extern const QString    OPTIONS;

    /**
     * \brief   Gets the options file path.
     * \return  The options file path.
     **/
    QString getOptionsFile(void);

    /**
     * \brief   Gets the user profile file path.
     * \param   fileName    The name of the file.
     * \return  The user profile file path.
     **/
    QString getUserProfileFile(const QString& fileName);

    /**
     * \brief   Generates a unique ID.
     * \return  A unique ID.
     **/
    uint32_t getId(void);

    /**
     * \brief   Gets the current timestamp.
     * \return  The current timestamp.
     **/
    uint64_t getTimestamp(void);
    
    /**
     * \brief   XML workspace version.
     **/
    const QString xmlWorkspaceVersion                       {"1.0.0"};

    /**
     * \brief   XML element names and attributes.
     **/
    constexpr const char * const xmlElementOptionList       { "OptionList" };
    constexpr const char * const xmlElementOption           { "Option" };
    constexpr const char * const xmlElementWorkspaceList    { "WorspaceList" };
    constexpr const char * const xmlElementWorkspace        { "Workspace" };
    constexpr const char * const xmlElementSettings         { "Settings" };
    constexpr const char * const xmlElementDirectories      { "Directories" };

    constexpr const char * const xmlElementWorspaceRoot     { "WorkspaceRoot" };
    constexpr const char * const xmlElementDescription      { "Description" };
    constexpr const char * const xmlElementSources          { "Sources" };
    constexpr const char * const xmlElementIncludes         { "Includes" };
    constexpr const char * const xmlElementDelivery         { "Delivery" };

    constexpr const char * const xmlElementProject          { "Project" };

    constexpr const char * const xmlAttributeLastAccessed   { "Accessed" };
    constexpr const char * const xmlAttributeId             { "id" };
    constexpr const char * const xmlAttributeName           { "Name" };
    constexpr const char * const xmlAttributeVersion        { "Version" };

    constexpr const char * const xmlElementRecentFiles      { "RecentFiles" };
    constexpr const char * const xmlElementFile             { "File" };
}

#endif  // LUSAN_COMMON_NELUSANCOMMON_HPP
