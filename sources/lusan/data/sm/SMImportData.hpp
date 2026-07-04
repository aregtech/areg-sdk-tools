#ifndef LUSAN_DATA_SM_SMIMPORTDATA_HPP
#define LUSAN_DATA_SM_SMIMPORTDATA_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/sm/SMImportData.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM submachine imports registry
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/DocumentElem.hpp"
#include "lusan/data/common/TEDataContainer.hpp"
#include "lusan/common/VersionNumber.hpp"

#include <QString>

/**
 * \class   SMImportEntry
 * \brief   One `MachineImport`: a reference to another `.fsml` document,
 *          registered under an alias `Name`, with a `Location` and the imported
 *          document's `Version` pinned at import/update time. Resolution state
 *          (found/parsed/version-diff) is runtime-only and never serialized.
 **/
class SMImportEntry : public DocumentElem
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    SMImportEntry(ElementBase* parent = nullptr);

    SMImportEntry(  uint32_t id
                  , const QString& name
                  , const QString& location = QString()
                  , const VersionNumber& version = VersionNumber()
                  , ElementBase* parent = nullptr);

    SMImportEntry(const SMImportEntry& src);
    SMImportEntry(SMImportEntry&& src) noexcept;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:
    SMImportEntry& operator = (const SMImportEntry& other);
    SMImportEntry& operator = (SMImportEntry&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    inline const QString& getName(void) const;
    inline void setName(const QString& name);

    inline const QString& getLocation(void) const;
    inline void setLocation(const QString& location);

    inline const VersionNumber& getVersion(void) const;
    inline void setVersion(const VersionNumber& version);

    inline const QString& getDescription(void) const;
    inline void setDescription(const QString& description);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    virtual bool isValid(void) const override;
    virtual bool readFromXml(QXmlStreamReader& xml) override;
    virtual void writeToXml(QXmlStreamWriter& xml) const override;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QString         mName;          //!< The import alias name.
    QString         mLocation;      //!< The path to the imported `.fsml` document.
    VersionNumber   mVersion;       //!< The pinned imported-document version.
    QString         mDescription;   //!< The description text.
};

//////////////////////////////////////////////////////////////////////////
// SMImportData class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \class   SMImportData
 * \brief   The `ImportList` registry: an ordered container of SMImportEntry.
 **/
class SMImportData : public TEDataContainer<SMImportEntry, DocumentElem>
{
public:
    SMImportData(ElementBase* parent = nullptr);

    virtual bool isValid(void) const override;
    virtual bool readFromXml(QXmlStreamReader& xml) override;
    virtual void writeToXml(QXmlStreamWriter& xml) const override;

    /**
     * \brief   Creates a new import appended at the end of the list.
     * \param   name    The unique alias of the new import.
     * \return  Pointer to the created import, or nullptr if the alias already exists.
     **/
    SMImportEntry* createImport(const QString& name);
};

//////////////////////////////////////////////////////////////////////////
// SMImportEntry inline methods
//////////////////////////////////////////////////////////////////////////

inline const QString& SMImportEntry::getName(void) const
{
    return mName;
}

inline void SMImportEntry::setName(const QString& name)
{
    mName = name;
}

inline const QString& SMImportEntry::getLocation(void) const
{
    return mLocation;
}

inline void SMImportEntry::setLocation(const QString& location)
{
    mLocation = location;
}

inline const VersionNumber& SMImportEntry::getVersion(void) const
{
    return mVersion;
}

inline void SMImportEntry::setVersion(const VersionNumber& version)
{
    mVersion = version;
}

inline const QString& SMImportEntry::getDescription(void) const
{
    return mDescription;
}

inline void SMImportEntry::setDescription(const QString& description)
{
    mDescription = description;
}

#endif  // LUSAN_DATA_SM_SMIMPORTDATA_HPP
