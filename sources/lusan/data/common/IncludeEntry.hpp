#ifndef LUSAN_DATA_COMMON_INCLUDEENTRY_HPP
#define LUSAN_DATA_COMMON_INCLUDEENTRY_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/data/common/IncludeEntry.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Include Entry.
 *
 ************************************************************************/

/************************************************************************
 * Include files
 ************************************************************************/
#include "lusan/data/common/DocumentElem.hpp"

#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

 /**
  * \class   IncludeEntry
  * \brief   Represents an include entry for service interfaces.
  **/
class IncludeEntry  : public DocumentElem
{
//////////////////////////////////////////////////////////////////////////
// Public methods
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     * \param   parent  The parent element.
     **/
    IncludeEntry(ElementBase * parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   id          The ID of the include entry.
     * \param   location    The file path of the include entry.
     * \param   parent      The parent element.
     **/
    IncludeEntry(uint32_t id, const QString & location, ElementBase * parent = nullptr);
    
    /**
     * \brief   Parameterized constructor.
     * \param   path            The file path.
     * \param   id              The unique ID.
     * \param   description     The description.
     * \param   deprecated      The deprecated flag.
     * \param   deprecationHint The deprecation hint.
     * \param   parent          The parent element.
     **/
    IncludeEntry(const QString& path, uint32_t id, const QString& description, bool deprecated, const QString& deprecationHint, ElementBase * parent = nullptr);

    /**
     * \brief   Copy constructor.
     * \param   other   The other IncludeEntry to copy from.
     **/
    IncludeEntry(const IncludeEntry& other);

    /**
     * \brief   Move constructor.
     * \param   other   The other IncludeEntry to move from.
     **/
    IncludeEntry(IncludeEntry&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Copy assignment operator.
     * \param   other   The other IncludeEntry to copy from.
     * \return  Reference to the assigned IncludeEntry.
     **/
    IncludeEntry& operator = (const IncludeEntry& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other IncludeEntry to move from.
     * \return  Reference to the assigned IncludeEntry.
     **/
    IncludeEntry& operator = (IncludeEntry&& other) noexcept;

     /**
      * \brief   Equality operator.
      * \param   other   The other IncludeEntry to compare with.
      * \return  True if both entries are equal, false otherwise.
      **/
     bool operator == (const IncludeEntry& other) const;

     /**
      * \brief   Inequality operator.
      * \param   other   The other IncludeEntry to compare with.
      * \return  True if both entries are not equal, false otherwise.
      **/
     bool operator != (const IncludeEntry& other) const;

    /**
     * \brief   Less than operator for sorting.
     * \param   other   The other object to compare with.
     * \return  True if this constant is less than the other, false otherwise.
     **/
    bool operator < (const IncludeEntry& other) const;

    /**
     * \brief   Less than operator for sorting.
     * \param   other   The other object to compare with.
     * \return  True if this constant is less than the other, false otherwise.
     **/
    bool operator > (const IncludeEntry& other) const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Reads include entry data from an XML stream.
     * \param   xml     The XML stream reader.
     * \return  True if the include entry data was successfully read, false otherwise.
     **/
    virtual bool readFromXml(QXmlStreamReader& xml) override;

    /**
     * \brief   Writes include entry data to an XML stream.
     * \param   xml     The XML stream writer.
     **/
    virtual void writeToXml(QXmlStreamWriter& xml) const override;

    /**
     * \brief   Returns true if the entry is valid. The entry is valid if location parameter is not empty.
     **/
    virtual bool isValid(void) const override;

    /**
     * \brief Returns the icon to display for specific display type.
     * \param display   The classification to display.
     */
    virtual QIcon getIcon(ElementBase::eDisplay display) const override;

    /**
     * \brief Returns the string to display for specific display type.
     * \param display   The classification to display.
     */
    virtual QString getString(ElementBase::eDisplay display) const override;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Gets the include file path location.
     * \return  The file path.
     **/
    const QString& getLocation(void) const;

    /**
     * \brief   Sets the include file path location.
     * \param   path    The file path.
     **/
    void setLocation(const QString& path);

    /**
     * \brief   Gets the name of the include entry, i.e. the location.
     **/
    inline const QString& getName(void) const;

    /**
     * \brief   Sets the name of the include entry, i.e. the location.
     **/
    inline void setName(const QString& path);

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
    bool getIsDeprecated(void) const;

    /**
     * \brief   Sets the deprecated flag.
     * \param   deprecated  The deprecated flag.
     **/
    void setIsDeprecated(bool deprecated);

    /**
     * \brief   Gets the deprecation hint.
     * \return  The deprecation hint.
     **/
    const QString& getDeprecateHint(void) const;

    /**
     * \brief   Sets the deprecation hint.
     * \param   deprecationHint The deprecation hint.
     **/
    void setDeprecateHint(const QString& deprecationHint);

    /**
     * \brief   Deprecates the entry. Sets deprecation flag and the comment
     * \param   deprecationHint     The comment to add for deprecation.
     */
    void deprecateEntry(const QString& deprecationHint);

//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:
    QString     mLocation;      //!< The file path.
    QString     mDescription;   //!< The description.
    bool        mDeprecated;    //!< The deprecated flag.
    QString     mDeprecateHint; //!< The deprecation hint.
};

//////////////////////////////////////////////////////////////////////////
// IncludeEntry inline methods
//////////////////////////////////////////////////////////////////////////

inline const QString& IncludeEntry::getName(void) const
{
    return getLocation();
}

inline void IncludeEntry::setName(const QString& path)
{
    setLocation(path);
}

#endif  // LUSAN_DATA_COMMON_INCLUDEENTRY_HPP
