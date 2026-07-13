#ifndef LUSAN_DATA_SM_SMTIMERDATA_HPP
#define LUSAN_DATA_SM_SMTIMERDATA_HPP
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
 *  \file        lusan/data/sm/SMTimerData.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM timers registry.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/DocumentElem.hpp"
#include "lusan/data/common/TEDataContainer.hpp"

#include <QString>

/**
 * \class   SMTimerEntry
 * \brief   One timer: a named timeout with interval and repeat count.
 *          `Timeout` is in milliseconds (minimum 1); `Repeat` count 0 means continuous.
 **/
class SMTimerEntry : public DocumentElem
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    SMTimerEntry(ElementBase* parent = nullptr);

    SMTimerEntry(  uint32_t id
                 , const QString& name
                 , uint32_t timeout = 1
                 , uint32_t repeat  = 1
                 , ElementBase* parent = nullptr);

    SMTimerEntry(const SMTimerEntry& src);
    SMTimerEntry(SMTimerEntry&& src) noexcept;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:
    SMTimerEntry& operator = (const SMTimerEntry& other);
    SMTimerEntry& operator = (SMTimerEntry&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    inline const QString& getName() const;
    inline void setName(const QString& name);

    inline uint32_t getTimeout() const;
    inline void setTimeout(uint32_t timeout);

    inline uint32_t getRepeat() const;
    inline void setRepeat(uint32_t repeat);

    inline const QString& getDescription() const;
    inline void setDescription(const QString& description);

    //!< Returns true if the timer is marked deprecated.
    inline bool getIsDeprecated() const;
    //!< Marks (or clears) the timer as deprecated.
    inline void setIsDeprecated(bool isDeprecated);
    //!< Returns the hint explaining why the timer is deprecated.
    inline const QString& getDeprecateHint() const;
    //!< Sets the hint explaining why the timer is deprecated.
    inline void setDeprecateHint(const QString& hint);

    /**
     * \brief   True if the repeat count means "continuous" (0 or 0xFFFFFFFF).
     **/
    inline bool isContinuous() const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
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
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QString     mName;          //!< The timer name.
    uint32_t    mTimeout;       //!< The timeout in milliseconds (minimum 1).
    uint32_t    mRepeat;        //!< The repeat count (0 = continuous).
    QString     mDescription;   //!< The description text.
    bool        mIsDeprecated;  //!< Flag, indicating whether the timer is deprecated.
    QString     mDeprecateHint; //!< The hint, why the timer is deprecated.
};

//////////////////////////////////////////////////////////////////////////
// SMTimerData class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \class   SMTimerData
 * \brief   The `TimerList` registry: an ordered container of SMTimerEntry.
 **/
class SMTimerData : public TEDataContainer<SMTimerEntry, DocumentElem>
{
public:
    SMTimerData(ElementBase* parent = nullptr);
    SMTimerData(const QList<SMTimerEntry>& entries, ElementBase* parent = nullptr);

    bool isValid() const override;
    bool readFromXml(QXmlStreamReader& xml) override;
    void writeToXml(QXmlStreamWriter& xml) const override;

    /**
     * \brief   Creates a new timer appended at the end of the list.
     * \param   name    The unique name of the new timer.
     * \return  Pointer to the created timer, or nullptr if the name already exists.
     **/
    SMTimerEntry* createTimer(const QString& name);
};

//////////////////////////////////////////////////////////////////////////
// SMTimerEntry inline methods
//////////////////////////////////////////////////////////////////////////

inline const QString& SMTimerEntry::getName() const
{
    return mName;
}

inline void SMTimerEntry::setName(const QString& name)
{
    mName = name;
}

inline uint32_t SMTimerEntry::getTimeout() const
{
    return mTimeout;
}

inline void SMTimerEntry::setTimeout(uint32_t timeout)
{
    mTimeout = timeout;
}

inline uint32_t SMTimerEntry::getRepeat() const
{
    return mRepeat;
}

inline void SMTimerEntry::setRepeat(uint32_t repeat)
{
    mRepeat = repeat;
}

inline const QString& SMTimerEntry::getDescription() const
{
    return mDescription;
}

inline void SMTimerEntry::setDescription(const QString& description)
{
    mDescription = description;
}

inline bool SMTimerEntry::getIsDeprecated() const
{
    return mIsDeprecated;
}

inline void SMTimerEntry::setIsDeprecated(bool isDeprecated)
{
    mIsDeprecated = isDeprecated;
    if (isDeprecated == false)
    {
        mDeprecateHint.clear();
    }
}

inline const QString& SMTimerEntry::getDeprecateHint() const
{
    return mDeprecateHint;
}

inline void SMTimerEntry::setDeprecateHint(const QString& hint)
{
    mDeprecateHint = hint;
}

inline bool SMTimerEntry::isContinuous() const
{
    return (mRepeat == 0) || (mRepeat == 0xFFFFFFFFu);
}

#endif  // LUSAN_DATA_SM_SMTIMERDATA_HPP
