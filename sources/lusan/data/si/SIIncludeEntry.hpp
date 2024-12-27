#ifndef LUSAN_DATA_SI_SIINCLUDEENTRY_HPP
#define LUSAN_DATA_SI_SIINCLUDEENTRY_HPP
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
 *  \copyright   � 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/data/si/SIIncludeEntry.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Include Entry.
 *
 ************************************************************************/

#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

 /**
  * \class   SIIncludeEntry
  * \brief   Represents an include entry for service interfaces.
  **/
class SIIncludeEntry
{
//////////////////////////////////////////////////////////////////////////
// Public methods
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    SIIncludeEntry(void);

    /**
     * \brief   Parameterized constructor.
     * \param   path            The file path.
     * \param   id              The unique ID.
     * \param   description     The description.
     * \param   deprecated      The deprecated flag.
     * \param   deprecationHint The deprecation hint.
     **/
    SIIncludeEntry(const QString& path, uint32_t id, const QString& description, bool deprecated, const QString& deprecationHint);

    /**
     * \brief   Copy constructor.
     * \param   other   The other SIIncludeEntry to copy from.
     **/
    SIIncludeEntry(const SIIncludeEntry& other);

    /**
     * \brief   Move constructor.
     * \param   other   The other SIIncludeEntry to move from.
     **/
    SIIncludeEntry(SIIncludeEntry&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Copy assignment operator.
     * \param   other   The other SIIncludeEntry to copy from.
     * \return  Reference to the assigned SIIncludeEntry.
     **/
    SIIncludeEntry& operator = (const SIIncludeEntry& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other SIIncludeEntry to move from.
     * \return  Reference to the assigned SIIncludeEntry.
     **/
    SIIncludeEntry& operator = (SIIncludeEntry&& other) noexcept;

     /**
      * \brief   Equality operator.
      * \param   other   The other SIIncludeEntry to compare with.
      * \return  True if both entries are equal, false otherwise.
      **/
     bool operator == (const SIIncludeEntry& other) const;

     /**
      * \brief   Inequality operator.
      * \param   other   The other SIIncludeEntry to compare with.
      * \return  True if both entries are not equal, false otherwise.
      **/
     bool operator != (const SIIncludeEntry& other) const;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns true if the entry is valid. The entry is valid if location parameter is not empty.
     **/
    bool isValid(void) const;

    /**
     * \brief   Gets the file path.
     * \return  The file path.
     **/
    const QString& getLocation(void) const;

    /**
     * \brief   Sets the file path.
     * \param   path    The file path.
     **/
    void setLocation(const QString& path);

    /**
     * \brief   Gets the unique ID.
     * \return  The unique ID.
     **/
    uint32_t getId(void) const;

    /**
     * \brief   Sets the unique ID.
     * \param   id      The unique ID.
     **/
    void setId(uint32_t id);

    /**
     * \brief   Gets the description.
     * \return  The description.
     **/
    const QString& getDescription(void) const;

    /**
     * \brief   Sets the description.
     * \param   description The description.
     **/
    void setDescription(const QString& description);

    /**
     * \brief   Gets the deprecated flag.
     * \return  The deprecated flag.
     **/
    bool isDeprecated(void) const;

    /**
     * \brief   Sets the deprecated flag.
     * \param   deprecated  The deprecated flag.
     **/
    void setDeprecated(bool deprecated);

    /**
     * \brief   Gets the deprecation hint.
     * \return  The deprecation hint.
     **/
    const QString& getDeprecationHint(void) const;

    /**
     * \brief   Sets the deprecation hint.
     * \param   deprecationHint The deprecation hint.
     **/
    void setDeprecationHint(const QString& deprecationHint);

    /**
     * \brief   Deprecates the entry. Sets deprecation flag and the comment
     * \param   deprecationHint     The comment to add for deprecation.
     */
    void deprecateEntry(const QString& deprecationHint);

    /**
     * \brief   Reads include entry data from an XML stream.
     * \param   xml     The XML stream reader.
     * \return  True if the include entry data was successfully read, false otherwise.
     **/
    bool readFromXml(QXmlStreamReader& xml);

    /**
     * \brief   Writes include entry data to an XML stream.
     * \param   xml     The XML stream writer.
     **/
    void writeToXml(QXmlStreamWriter& xml) const;

//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:
    uint32_t    mEntryId;       //!< The unique ID.
    QString     mLocation;      //!< The file path.
    QString     mDescription;   //!< The description.
    bool        mDeprecated;    //!< The deprecated flag.
    QString     mDeprecateHint; //!< The deprecation hint.
};

#endif  // LUSAN_DATA_SI_SIINCLUDEENTRY_HPP