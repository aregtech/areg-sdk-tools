#ifndef LUSAN_DATA_COMMON_OVERVIEWDATASECTION_HPP
#define LUSAN_DATA_COMMON_OVERVIEWDATASECTION_HPP
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
 *  \file        lusan/data/common/OverviewDataSection.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Overview section shared by every document.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/DocumentElem.hpp"
#include "lusan/common/VersionNumber.hpp"

#include <QString>
#include <QXmlStreamAttributes>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

/**
 * \class   OverviewDataSection
 * \brief   The `Overview` element every document opens with: the name it declares, the version
 *          its author owns, the description, and the deprecation mark with its hint.
 *
 *          What a document says about itself beyond that -- the service category of an interface,
 *          the threading mode of a state machine -- is its own, and is carried by the document's
 *          own section class through \ref readOwnAttributes and \ref writeOwnAttributes.
 **/
class OverviewDataSection : public DocumentElem
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Creates the section.
     * \param   omitEmptyDescription    True where the document leaves an empty description out
     *                                  of the file rather than writing an empty element.
     * \param   parent                  The parent element, which is the document root.
     **/
    OverviewDataSection(bool omitEmptyDescription, ElementBase* parent);

    /**
     * \brief   Creates the section with an ID and a name.
     **/
    OverviewDataSection(uint32_t id, const QString& name, bool omitEmptyDescription, ElementBase* parent);

    virtual ~OverviewDataSection(void) = default;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Checks that the document declares a name.
     **/
    virtual bool isValid(void) const override;

    /**
     * \brief   Reads the section from an XML stream, positioned at the `Overview` element.
     **/
    virtual bool readFromXml(QXmlStreamReader& xml) override;

    /**
     * \brief   Writes the section to an XML stream.
     **/
    virtual void writeToXml(QXmlStreamWriter& xml) const override;

protected:
    /**
     * \brief   Reads what only this document's overview carries. The shared attributes are
     *          already applied when this is called.
     **/
    virtual void readOwnAttributes(const QXmlStreamAttributes& attributes);

    /**
     * \brief   Writes what only this document's overview carries, into the open `Overview`
     *          element, right after the shared attributes.
     **/
    virtual void writeOwnAttributes(QXmlStreamWriter& xml) const;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    inline const QString& getName(void) const;
    inline void setName(const QString& name);

    inline const VersionNumber& getVersion(void) const;
    inline void setVersion(const VersionNumber& version);
    inline void setVersion(const QString& version);
    inline void setVersion(uint32_t major, uint32_t minor, uint32_t patch);

    inline const QString& getDescription(void) const;
    inline void setDescription(const QString& description);

    inline bool getIsDeprecated(void) const;
    inline void setIsDeprecated(bool isDeprecated);

    inline const QString& getDeprecateHint(void) const;
    inline void setDeprecateHint(const QString& hint);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QString         mName;                  //!< The name the document declares.
    VersionNumber   mVersion;               //!< The version its author owns.
    QString         mDescription;           //!< The description text.
    bool            mIsDeprecated;          //!< The flag marking the whole document deprecated.
    QString         mDeprecateHint;         //!< The hint shown when the document is deprecated.
    bool            mOmitEmptyDescription;  //!< Whether an empty description is left out of the file.
};

//////////////////////////////////////////////////////////////////////////
// OverviewDataSection inline methods
//////////////////////////////////////////////////////////////////////////

inline const QString& OverviewDataSection::getName(void) const
{
    return mName;
}

inline void OverviewDataSection::setName(const QString& name)
{
    mName = name;
}

inline const VersionNumber& OverviewDataSection::getVersion(void) const
{
    return mVersion;
}

inline void OverviewDataSection::setVersion(const VersionNumber& version)
{
    mVersion = version;
}

inline void OverviewDataSection::setVersion(const QString& version)
{
    mVersion.fromString(version);
}

inline void OverviewDataSection::setVersion(uint32_t major, uint32_t minor, uint32_t patch)
{
    mVersion = VersionNumber(major, minor, patch);
}

inline const QString& OverviewDataSection::getDescription(void) const
{
    return mDescription;
}

inline void OverviewDataSection::setDescription(const QString& description)
{
    mDescription = description;
}

inline bool OverviewDataSection::getIsDeprecated(void) const
{
    return mIsDeprecated;
}

inline void OverviewDataSection::setIsDeprecated(bool isDeprecated)
{
    mIsDeprecated = isDeprecated;
}

inline const QString& OverviewDataSection::getDeprecateHint(void) const
{
    return mDeprecateHint;
}

inline void OverviewDataSection::setDeprecateHint(const QString& hint)
{
    mDeprecateHint = hint;
}

#endif  // LUSAN_DATA_COMMON_OVERVIEWDATASECTION_HPP
