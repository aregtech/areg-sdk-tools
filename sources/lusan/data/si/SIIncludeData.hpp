#ifndef LUSAN_DATA_SI_SIINCLUDEDATA_HPP
#define LUSAN_DATA_SI_SIINCLUDEDATA_HPP
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
 *  \file        lusan/data/si/SIIncludeData.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Include Data.
 *
 ************************************************************************/

#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "lusan/data/si/SIIncludeEntry.hpp"

 /**
  * \class   SIIncludeData
  * \brief   Manages include data for service interfaces.
  **/
class SIIncludeData
{
//////////////////////////////////////////////////////////////////////////
// Local types and static members
//////////////////////////////////////////////////////////////////////////
private:
    static const SIIncludeEntry InvalidInclude; //!< Invalid include object

//////////////////////////////////////////////////////////////////////////
// public methods
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    SIIncludeData(void);

    /**
     * \brief   Constructor with initialization.
     * \param   includes    The list of include entries.
     **/
    SIIncludeData(const QList<SIIncludeEntry>& includes);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Gets the list of include entries.
     * \return  The list of include entries.
     **/
    const QList<SIIncludeEntry>& getIncludes(void) const;

    /**
     * \brief   Sets the list of include entries.
     * \param   includes    The list of include entries.
     **/
    void setIncludes(const QList<SIIncludeEntry>& includes);

    /**
     * \brief   Searches for an include entry in the list.
     * \param   include     The include entry to search for.
     * \return  The index of the include entry, or -1 if not found.
     **/
    int findInclude(const SIIncludeEntry& include) const;

    /**
     * \brief   Adds an include entry to the list.
     * \param   include     The include entry to add.
     **/
    void addInclude(const SIIncludeEntry& include);

    /**
     * \brief   Removes an include entry from the list.
     * \param   include     The include entry to remove.
     * \return  True if the include entry was removed, false otherwise.
     **/
    bool removeInclude(const SIIncludeEntry& include);

    /**
     * \brief   Replaces an include entry in the list.
     * \param   oldInclude  The include entry to replace.
     * \param   newInclude  The new include entry.
     * \return  True if the include entry was replaced, false otherwise.
     **/
    bool replaceInclude(const SIIncludeEntry& oldInclude, const SIIncludeEntry& newInclude);

    /**
     * \brief   Reads include data from an XML stream.
     * \param   xml         The XML stream reader.
     * \return  True if the include data was successfully read, false otherwise.
     **/
    bool readFromXml(QXmlStreamReader& xml);

    /**
     * \brief   Writes include data to an XML stream.
     * \param   xml         The XML stream writer.
     **/
    void writeToXml(QXmlStreamWriter& xml) const;

    /**
     * \brief   Finds an include entry by location.
     * \param   location    The location to search for.
     * \return  The index of the include entry, or -1 if not found.
     **/
    int findInclude(const QString& location) const;

    /**
     * \brief   Checks if an include entry exists by location.
     * \param   location    The location to check.
     * \return  True if the include entry exists, false otherwise.
     **/
    bool exists(const QString& location) const;

    /**
     * \brief   Gets an include entry by location.
     * \param   location    The location to search for.
     * \return  The include entry if found, otherwise an invalid include entry.
     **/
    const SIIncludeEntry & getInclude(const QString& location) const;

    /**
     * \brief   Removes an include entry by location.
     * \param   location    The location to remove.
     * \return  True if the include entry was removed, false otherwise.
     **/
    bool removeInclude(const QString& location);

//////////////////////////////////////////////////////////////////////////
// Hidden member variables.
//////////////////////////////////////////////////////////////////////////
private:
    QList<SIIncludeEntry> mIncludes;  //!< The list of include entries.
};

#endif  // LUSAN_DATA_SI_SIINCLUDEDATA_HPP
