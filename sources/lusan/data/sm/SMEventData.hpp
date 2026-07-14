#ifndef LUSAN_DATA_SM_SMEVENTDATA_HPP
#define LUSAN_DATA_SM_SMEVENTDATA_HPP
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
 *  \file        lusan/data/sm/SMEventData.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM events registry.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/MethodBase.hpp"
#include "lusan/data/common/TEDataContainer.hpp"

/**
 * \class   SMEventEntry
 * \brief   One internal event: a machine-global asynchronous stimulus with
 *          an optional typed payload. Derives MethodBase so the payload reuses the
 *          MethodParameter children (name/type/default) verbatim.
 **/
class SMEventEntry : public MethodBase
{
public:
    SMEventEntry(ElementBase* parent = nullptr);
    SMEventEntry(uint32_t id, const QString& name, ElementBase* parent = nullptr);
    SMEventEntry(const SMEventEntry& src);
    SMEventEntry(SMEventEntry&& src) noexcept;

    SMEventEntry& operator = (const SMEventEntry& other);
    SMEventEntry& operator = (SMEventEntry&& other) noexcept;

    bool isValid() const override;
    bool readFromXml(QXmlStreamReader& xml) override;
    void writeToXml(QXmlStreamWriter& xml) const override;

    /**
     * \brief Returns the icon to display for specific display type.
     * \param display   The classification to display.
     */
    QIcon getIcon(ElementBase::eDisplay display) const override;

    /**
     * \brief Returns the string to display for specific display type.
     * \param display   The classification to display.
     */
    QString getString(ElementBase::eDisplay display) const override;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    //!< Returns true if the event is marked deprecated.
    inline bool getIsDeprecated() const;
    //!< Marks (or clears) the event as deprecated.
    inline void setIsDeprecated(bool isDeprecated);
    //!< Returns the hint explaining why the event is deprecated.
    inline const QString& getDeprecateHint() const;
    //!< Sets the hint explaining why the event is deprecated.
    inline void setDeprecateHint(const QString& hint);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    bool    mIsDeprecated;  //!< Flag, indicating whether the event is deprecated.
    QString mDeprecateHint; //!< The hint, why the event is deprecated.
};

//////////////////////////////////////////////////////////////////////////
// SMEventEntry inline methods
//////////////////////////////////////////////////////////////////////////

inline bool SMEventEntry::getIsDeprecated() const
{
    return mIsDeprecated;
}

inline void SMEventEntry::setIsDeprecated(bool isDeprecated)
{
    mIsDeprecated = isDeprecated;
    if (isDeprecated == false)
    {
        mDeprecateHint.clear();
    }
}

inline const QString& SMEventEntry::getDeprecateHint() const
{
    return mDeprecateHint;
}

inline void SMEventEntry::setDeprecateHint(const QString& hint)
{
    mDeprecateHint = hint;
}

//////////////////////////////////////////////////////////////////////////
// SMEventData class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \class   SMEventData
 * \brief   The `EventList` registry: an ordered, owning container of SMEventEntry. Entries
 *          are held by pointer so their addresses (and thus their payload parameters'
 *          parent chain for ID allocation) stay stable across container growth.
 **/
class SMEventData : public TEDataContainer<SMEventEntry*, DocumentElem>
{
public:
    SMEventData(ElementBase* parent = nullptr);
    virtual ~SMEventData();

    bool isValid() const override;
    bool readFromXml(QXmlStreamReader& xml) override;
    void writeToXml(QXmlStreamWriter& xml) const override;

    /**
     * \brief   Creates a new event appended at the end of the list.
     * \param   name    The unique name of the new event.
     * \return  Pointer to the created event, or nullptr if the name already exists.
     **/
    SMEventEntry* createEvent(const QString& name);

    /**
     * \brief   Finds an event by name.
     **/
    SMEventEntry* findEvent(const QString& name) const;

    /**
     * \brief   Finds an event by ID.
     **/
    SMEventEntry* findEvent(uint32_t id) const;

    /**
     * \brief   Deletes and removes all events.
     **/
    void removeAll();
};

#endif  // LUSAN_DATA_SM_SMEVENTDATA_HPP
