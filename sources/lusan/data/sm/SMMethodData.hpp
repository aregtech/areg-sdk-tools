#ifndef LUSAN_DATA_SM_SMMETHODDATA_HPP
#define LUSAN_DATA_SM_SMMETHODDATA_HPP
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
 *  \file        lusan/data/sm/SMMethodData.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM methods registry: triggers, actions, conditions
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/MethodBase.hpp"
#include "lusan/data/common/TEDataContainer.hpp"

/**
 * \class   SMMethodEntry
 * \brief   One declared method: a Trigger, an Action or a Condition. One
 *          class with a kind enum keeps the shared stimulus name space queryable and
 *          the three kinds' shared structure (name + parameters) in one place.
 *          Conditions additionally declare a `Return` type and an `Implement` mode; an
 *          Embedded condition carries a verbatim `Body`.
 **/
class SMMethodEntry : public MethodBase
{
//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \enum    eMethodType
     * \brief   The method kind.
     **/
    enum class eMethodType
    {
          Trigger       //!< An externally callable stimulus (a public method of the machine).
        , Action        //!< A void callback implemented by the user (pure virtual).
        , Condition     //!< Always returns a value (default bool); Handler or Embedded.
    };

    /**
     * \enum    eImplement
     * \brief   How a Condition method is implemented.
     **/
    enum class eImplement
    {
          Handler       //!< Pure virtual, the user implements it at runtime.
        , Embedded      //!< Body written in the designer, generated verbatim.
    };

    static constexpr const char* const  STR_TYPE_TRIGGER    { "Trigger"   };
    static constexpr const char* const  STR_TYPE_ACTION     { "Action"    };
    static constexpr const char* const  STR_TYPE_CONDITION  { "Condition" };
    static constexpr const char* const  STR_IMPL_HANDLER    { "Handler"   };
    static constexpr const char* const  STR_IMPL_EMBEDDED   { "Embedded"  };
    static constexpr const char* const  DEFAULT_RETURN      { "bool"      };

    static SMMethodEntry::eMethodType fromTypeString(const QString& type);
    static const char* toString(SMMethodEntry::eMethodType type);
    static SMMethodEntry::eImplement fromImplementString(const QString& implement);
    static const char* toString(SMMethodEntry::eImplement implement);

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    SMMethodEntry(ElementBase* parent = nullptr);
    SMMethodEntry(uint32_t id, const QString& name, eMethodType type, ElementBase* parent = nullptr);
    SMMethodEntry(const SMMethodEntry& src);
    SMMethodEntry(SMMethodEntry&& src) noexcept;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:
    SMMethodEntry& operator = (const SMMethodEntry& other);
    SMMethodEntry& operator = (SMMethodEntry&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    inline eMethodType getMethodType() const;
    inline void setMethodType(eMethodType type);

    inline bool isTrigger() const;
    inline bool isAction() const;
    inline bool isCondition() const;

    inline const QString& getReturn() const;
    inline void setReturn(const QString& type);

    inline eImplement getImplement() const;
    inline void setImplement(eImplement implement);

    inline const QString& getBody() const;
    inline void setBody(const QString& body);

    //!< Returns true if the method is marked deprecated.
    inline bool getIsDeprecated() const;
    //!< Marks (or clears) the method as deprecated.
    inline void setIsDeprecated(bool isDeprecated);
    //!< Returns the hint explaining why the method is deprecated.
    inline const QString& getDeprecateHint() const;
    //!< Sets the hint explaining why the method is deprecated.
    inline void setDeprecateHint(const QString& hint);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    bool isValid() const override;
    bool readFromXml(QXmlStreamReader& xml) override;
    void writeToXml(QXmlStreamWriter& xml) const override;

    /**
     * \brief   Returns the icon to display for the given classification (kind-specific icon
     *          for Name, none otherwise).
     **/
    QIcon getIcon(ElementBase::eDisplay display) const override;

    /**
     * \brief   Returns the display string: the name for Name, the method kind for Type, and
     *          the parameter count (or the return type for conditions) for Value.
     **/
    QString getString(ElementBase::eDisplay display) const override;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    eMethodType mMethodType;    //!< The method kind.
    QString     mReturn;        //!< The condition return type (conditions only; default bool).
    eImplement  mImplement;     //!< The condition implementation mode (conditions only).
    QString     mBody;          //!< The verbatim embedded-condition body (CDATA on write).
    bool        mIsDeprecated;  //!< Flag, indicating whether the method is deprecated.
    QString     mDeprecateHint; //!< The hint, why the method is deprecated.
};

//////////////////////////////////////////////////////////////////////////
// SMMethodData class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \class   SMMethodData
 * \brief   The `MethodList` registry: an ordered, owning container of SMMethodEntry.
 *          Entries are held by pointer so their addresses (and thus their parameters'
 *          parent chain for ID allocation) stay stable across container growth.
 **/
class SMMethodData : public TEDataContainer<SMMethodEntry*, DocumentElem>
{
public:
    SMMethodData(ElementBase* parent = nullptr);
    virtual ~SMMethodData();

    bool isValid() const override;
    bool readFromXml(QXmlStreamReader& xml) override;
    void writeToXml(QXmlStreamWriter& xml) const override;

    /**
     * \brief   Creates a new method appended at the end of the list.
     * \param   name    The unique name of the new method.
     * \param   type    The method kind.
     * \return  Pointer to the created method, or nullptr if the name already exists.
     **/
    SMMethodEntry* createMethod(const QString& name, SMMethodEntry::eMethodType type);

    /**
     * \brief   Finds a method by name.
     **/
    SMMethodEntry* findMethod(const QString& name) const;

    /**
     * \brief   Finds a method by ID.
     **/
    SMMethodEntry* findMethod(uint32_t id) const;

    /**
     * \brief   Finds a method by name only if it is a Trigger (used by the shared
     *          stimulus name space.
     * \param   name    The trigger name to look up.
     * \return  Pointer to the trigger method, or nullptr if absent or not a trigger.
     **/
    SMMethodEntry* findTrigger(const QString& name) const;

    /**
     * \brief   Deletes and removes all methods.
     **/
    void removeAll();
};

//////////////////////////////////////////////////////////////////////////
// SMMethodEntry inline methods
//////////////////////////////////////////////////////////////////////////

inline SMMethodEntry::eMethodType SMMethodEntry::getMethodType() const
{
    return mMethodType;
}

inline void SMMethodEntry::setMethodType(SMMethodEntry::eMethodType type)
{
    mMethodType = type;
}

inline bool SMMethodEntry::isTrigger() const
{
    return (mMethodType == eMethodType::Trigger);
}

inline bool SMMethodEntry::isAction() const
{
    return (mMethodType == eMethodType::Action);
}

inline bool SMMethodEntry::isCondition() const
{
    return (mMethodType == eMethodType::Condition);
}

inline const QString& SMMethodEntry::getReturn() const
{
    return mReturn;
}

inline void SMMethodEntry::setReturn(const QString& type)
{
    mReturn = type;
}

inline SMMethodEntry::eImplement SMMethodEntry::getImplement() const
{
    return mImplement;
}

inline void SMMethodEntry::setImplement(SMMethodEntry::eImplement implement)
{
    mImplement = implement;
}

inline const QString& SMMethodEntry::getBody() const
{
    return mBody;
}

inline void SMMethodEntry::setBody(const QString& body)
{
    mBody = body;
}

inline bool SMMethodEntry::getIsDeprecated() const
{
    return mIsDeprecated;
}

inline void SMMethodEntry::setIsDeprecated(bool isDeprecated)
{
    mIsDeprecated = isDeprecated;
    if (isDeprecated == false)
    {
        mDeprecateHint.clear();
    }
}

inline const QString& SMMethodEntry::getDeprecateHint() const
{
    return mDeprecateHint;
}

inline void SMMethodEntry::setDeprecateHint(const QString& hint)
{
    mDeprecateHint = hint;
}

#endif  // LUSAN_DATA_SM_SMMETHODDATA_HPP
