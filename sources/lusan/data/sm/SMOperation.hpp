#ifndef LUSAN_DATA_SM_SMOPERATION_HPP
#define LUSAN_DATA_SM_SMOPERATION_HPP
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
 *  \file        lusan/data/sm/SMOperation.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM operations: typed instructions executed
 *               on entry, exit, or during a transition, plus their argument mappings.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/DocumentElem.hpp"

#include <QList>
#include <QString>

/**
 * \class   SMArgumentEntry
 * \brief   One parameter mapping : binds a declared parameter to a value
 *          source. Shared by ActionCall, EventSend, condition-method calls, and the
 *          AttributeSet value.
 **/
class SMArgumentEntry : public DocumentElem
{
public:
    /**
     * \enum    eValueSource
     * \brief   The kind of value supplied for a mapping
     **/
    enum class eValueSource
    {
          Value         //!< A typed literal (the `Value` text).
        , Param         //!< A stimulus parameter (transition scope only).
        , Attribute     //!< A machine attribute.
        , Constant      //!< A declared constant.
        , Condition     //!< A parameterless Condition-method call.
        , Expression    //!< Verbatim C++ (the `Expression` child).
    };

    static constexpr const char* const  STR_SRC_VALUE       { "Value"      };
    static constexpr const char* const  STR_SRC_PARAM       { "Param"      };
    static constexpr const char* const  STR_SRC_ATTRIBUTE   { "Attribute"  };
    static constexpr const char* const  STR_SRC_CONSTANT    { "Constant"   };
    static constexpr const char* const  STR_SRC_CONDITION   { "Condition"  };
    static constexpr const char* const  STR_SRC_EXPRESSION  { "Expression" };

    static SMArgumentEntry::eValueSource fromSourceString(const QString& source);
    static const char* toString(SMArgumentEntry::eValueSource source);

public:
    SMArgumentEntry(ElementBase* parent = nullptr);
    SMArgumentEntry(  uint32_t id
                    , const QString& name
                    , eValueSource source  = eValueSource::Value
                    , const QString& value = QString()
                    , ElementBase* parent  = nullptr);
    SMArgumentEntry(const SMArgumentEntry& src);
    SMArgumentEntry(SMArgumentEntry&& src) noexcept;

    SMArgumentEntry& operator = (const SMArgumentEntry& other);
    SMArgumentEntry& operator = (SMArgumentEntry&& other) noexcept;

    inline const QString& getName(void) const;
    inline void setName(const QString& name);
    inline eValueSource getSource(void) const;
    inline void setSource(eValueSource source);
    inline const QString& getValue(void) const;
    inline void setValue(const QString& value);
    inline const QString& getExpression(void) const;
    inline void setExpression(const QString& expression);

    virtual bool isValid(void) const override;
    virtual bool readFromXml(QXmlStreamReader& xml) override;
    virtual void writeToXml(QXmlStreamWriter& xml) const override;

private:
    QString         mName;          //!< The mapped parameter name.
    eValueSource    mSource;        //!< The value source kind.
    QString         mValue;         //!< The literal / referenced-name text (empty for Expression).
    QString         mExpression;    //!< The verbatim C++ (for Source=Expression only).
};

//////////////////////////////////////////////////////////////////////////
// SMOperationBase and typed operation subclasses
//////////////////////////////////////////////////////////////////////////

/**
 * \class   SMOperationBase
 * \brief   Base of the typed operation hierarchy. Operations are typed
 *          elements (not a generic property bag) so the validator, canvas rendering,
 *          interpreter and generator can switch on kind without parsing.
 **/
class SMOperationBase : public DocumentElem
{
public:
    /**
     * \enum    eOperation
     * \brief   The operation kind.
     **/
    enum class eOperation
    {
          ActionCall    //!< Invoke a declared action.
        , AttributeSet  //!< Assign a value to a machine attribute.
        , TimerStart    //!< Start a declared timer.
        , TimerStop     //!< Stop a declared timer.
        , EventSend     //!< Queue an event for asynchronous dispatch.
        , InlineCode    //!< Execute a verbatim user code block.
    };

    static constexpr const char* const  STR_ACTION_CALL     { "ActionCall"   };
    static constexpr const char* const  STR_ATTRIBUTE_SET   { "AttributeSet" };
    static constexpr const char* const  STR_TIMER_START     { "TimerStart"   };
    static constexpr const char* const  STR_TIMER_STOP      { "TimerStop"    };
    static constexpr const char* const  STR_EVENT_SEND      { "EventSend"    };
    static constexpr const char* const  STR_INLINE_CODE     { "InlineCode"   };

protected:
    SMOperationBase(eOperation kind, ElementBase* parent = nullptr);
    SMOperationBase(eOperation kind, uint32_t id, ElementBase* parent);
    SMOperationBase(const SMOperationBase& src);
    SMOperationBase(SMOperationBase&& src) noexcept;
    SMOperationBase& operator = (const SMOperationBase& other);
    SMOperationBase& operator = (SMOperationBase&& other) noexcept;

public:
    virtual ~SMOperationBase(void) = default;

    /**
     * \brief   Returns the operation kind.
     **/
    inline eOperation getOperationType(void) const;

    /**
     * \brief   The XML element name for this operation kind.
     **/
    const char* getElementName(void) const;

    /**
     * \brief   Deep-copies this operation, preserving its concrete type.
     **/
    virtual SMOperationBase* clone(void) const = 0;

    /**
     * \brief   The primary referenced name (action/attribute/timer/event); used as the
     *          element display name. Empty for InlineCode.
     **/
    virtual QString getName(void) const = 0;

    virtual bool isValid(void) const override;
    virtual bool readFromXml(QXmlStreamReader& xml) override;
    virtual void writeToXml(QXmlStreamWriter& xml) const override;

protected:
    eOperation  mKind;      //!< The operation kind.
};

/**
 * \class   SMActionCall
 * \brief   `ActionCall`: invoke a declared action, mapping its parameters.
 **/
class SMActionCall : public SMOperationBase
{
public:
    SMActionCall(ElementBase* parent = nullptr);
    SMActionCall(uint32_t id, const QString& action, ElementBase* parent = nullptr);
    SMActionCall(const SMActionCall& src);

    inline const QString& getAction(void) const;
    inline void setAction(const QString& action);
    inline const QList<SMArgumentEntry>& getArguments(void) const;
    inline QList<SMArgumentEntry>& getArguments(void);

    /**
     * \brief   Appends an argument mapping, allocating its ID from the document counter.
     **/
    SMArgumentEntry* addArgument(const QString& name, SMArgumentEntry::eValueSource source, const QString& value);

    virtual SMOperationBase* clone(void) const override;
    virtual QString getName(void) const override;

private:
    QString                     mAction;        //!< The action method name.
    QList<SMArgumentEntry>      mArguments;     //!< The argument list.
};

/**
 * \class   SMAttributeSet
 * \brief   `AttributeSet`: assign a value to a machine attribute.
 **/
class SMAttributeSet : public SMOperationBase
{
public:
    SMAttributeSet(ElementBase* parent = nullptr);
    SMAttributeSet(uint32_t id, const QString& attribute, ElementBase* parent = nullptr);
    SMAttributeSet(const SMAttributeSet& src);

    inline const QString& getAttribute(void) const;
    inline void setAttribute(const QString& attribute);
    inline SMArgumentEntry::eValueSource getSource(void) const;
    inline void setSource(SMArgumentEntry::eValueSource source);
    inline const QString& getValue(void) const;
    inline void setValue(const QString& value);
    inline const QString& getExpression(void) const;
    inline void setExpression(const QString& expression);

    virtual SMOperationBase* clone(void) const override;
    virtual QString getName(void) const override;

private:
    QString                         mAttribute;     //!< The target attribute name.
    SMArgumentEntry::eValueSource   mSource;        //!< The value source kind.
    QString                         mValue;         //!< The literal / referenced name.
    QString                         mExpression;    //!< Verbatim C++ (Source=Expression).
};

/**
 * \class   SMTimerStart
 * \brief   `TimerStart`: start a declared timer with optional overrides.
 **/
class SMTimerStart : public SMOperationBase
{
public:
    SMTimerStart(ElementBase* parent = nullptr);
    SMTimerStart(uint32_t id, const QString& timer, ElementBase* parent = nullptr);
    SMTimerStart(const SMTimerStart& src);

    inline const QString& getTimer(void) const;
    inline void setTimer(const QString& timer);
    inline bool hasTimeoutOverride(void) const;
    inline uint32_t getTimeout(void) const;
    void setTimeout(uint32_t timeout);
    void clearTimeout(void);
    inline bool hasRepeatOverride(void) const;
    inline uint32_t getRepeat(void) const;
    void setRepeat(uint32_t repeat);
    void clearRepeat(void);

    virtual SMOperationBase* clone(void) const override;
    virtual QString getName(void) const override;

private:
    QString     mTimer;         //!< The timer name.
    bool        mHasTimeout;    //!< Whether a timeout override is present.
    uint32_t    mTimeout;       //!< The timeout override (ms).
    bool        mHasRepeat;     //!< Whether a repeat override is present.
    uint32_t    mRepeat;        //!< The repeat override.
};

/**
 * \class   SMTimerStop
 * \brief   `TimerStop`: stop a declared timer.
 **/
class SMTimerStop : public SMOperationBase
{
public:
    SMTimerStop(ElementBase* parent = nullptr);
    SMTimerStop(uint32_t id, const QString& timer, ElementBase* parent = nullptr);
    SMTimerStop(const SMTimerStop& src);

    inline const QString& getTimer(void) const;
    inline void setTimer(const QString& timer);

    virtual SMOperationBase* clone(void) const override;
    virtual QString getName(void) const override;

private:
    QString     mTimer;     //!< The timer name.
};

/**
 * \class   SMEventSend
 * \brief   `EventSend`: queue an event for asynchronous dispatch.
 **/
class SMEventSend : public SMOperationBase
{
public:
    SMEventSend(ElementBase* parent = nullptr);
    SMEventSend(uint32_t id, const QString& event, ElementBase* parent = nullptr);
    SMEventSend(const SMEventSend& src);

    inline const QString& getEvent(void) const;
    inline void setEvent(const QString& event);
    inline const QList<SMArgumentEntry>& getArguments(void) const;
    inline QList<SMArgumentEntry>& getArguments(void);

    SMArgumentEntry* addArgument(const QString& name, SMArgumentEntry::eValueSource source, const QString& value);

    virtual SMOperationBase* clone(void) const override;
    virtual QString getName(void) const override;

private:
    QString                     mEvent;         //!< The event name.
    QList<SMArgumentEntry>      mArguments;     //!< The argument list.
};

/**
 * \class   SMInlineCode
 * \brief   `InlineCode`: execute a verbatim user code block (never parsed).
 **/
class SMInlineCode : public SMOperationBase
{
public:
    SMInlineCode(ElementBase* parent = nullptr);
    SMInlineCode(uint32_t id, const QString& body, ElementBase* parent = nullptr);
    SMInlineCode(const SMInlineCode& src);

    inline const QString& getBody(void) const;
    inline void setBody(const QString& body);

    virtual SMOperationBase* clone(void) const override;
    virtual QString getName(void) const override;

private:
    QString     mBody;      //!< The verbatim C++ body (CDATA on write).
};

//////////////////////////////////////////////////////////////////////////
// SMOperationList
//////////////////////////////////////////////////////////////////////////

/**
 * \class   SMOperationList
 * \brief   An ordered, owning list of operations, backing a state's `EntryList` /
 *          `ExitList` and a transition's `OperationList`. A dedicated owner (rather than
 *          TEDataContainer) is used because operations are polymorphic and owned by
 *          pointer — the same reason SIDataTypeData manages its pointer entries by hand.
 *          Order is document order (operations run in order).
 **/
class SMOperationList
{
public:
    SMOperationList(ElementBase* parent = nullptr);
    SMOperationList(const SMOperationList& src);
    SMOperationList(SMOperationList&& src) noexcept;
    ~SMOperationList(void);

    SMOperationList& operator = (const SMOperationList& other);
    SMOperationList& operator = (SMOperationList&& other) noexcept;

    /**
     * \brief   Sets the parent element and re-parents every owned operation, so ID
     *          allocation delegates to the document counter.
     **/
    void setParent(ElementBase* parent);
    inline ElementBase* getParent(void) const;

    /**
     * \brief   Appends an operation, taking ownership and allocating its ID from the
     *          document counter.
     * \return  The same pointer that was passed in.
     **/
    SMOperationBase* addOperation(SMOperationBase* operation);

    inline const QList<SMOperationBase*>& getOperations(void) const;
    inline int getCount(void) const;
    inline bool isEmpty(void) const;
    inline SMOperationBase* at(int index) const;

    /**
     * \brief   Deletes and removes every owned operation.
     **/
    void clear(void);

private:
    ElementBase*                mParent;        //!< The owning element (for ID delegation).
    QList<SMOperationBase*>     mOperations;    //!< The owned operations, in document order.
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline const QString& SMArgumentEntry::getName(void) const
{
    return mName;
}

inline void SMArgumentEntry::setName(const QString& name)
{
    mName = name;
}

inline SMArgumentEntry::eValueSource SMArgumentEntry::getSource(void) const
{
    return mSource;
}

inline void SMArgumentEntry::setSource(SMArgumentEntry::eValueSource source)
{
    mSource = source;
}

inline const QString& SMArgumentEntry::getValue(void) const
{
    return mValue;
}

inline void SMArgumentEntry::setValue(const QString& value)
{
    mValue = value;
}

inline const QString& SMArgumentEntry::getExpression(void) const
{
    return mExpression;
}

inline void SMArgumentEntry::setExpression(const QString& expression)
{
    mExpression = expression;
}

inline SMOperationBase::eOperation SMOperationBase::getOperationType(void) const
{
    return mKind;
}

inline const QString& SMActionCall::getAction(void) const
{
    return mAction;
}

inline void SMActionCall::setAction(const QString& action)
{
    mAction = action;
}

inline const QList<SMArgumentEntry>& SMActionCall::getArguments(void) const
{
    return mArguments;
}

inline QList<SMArgumentEntry>& SMActionCall::getArguments(void)
{
    return mArguments;
}

inline const QString& SMAttributeSet::getAttribute(void) const
{
    return mAttribute;
}

inline void SMAttributeSet::setAttribute(const QString& attribute)
{
    mAttribute = attribute;
}

inline SMArgumentEntry::eValueSource SMAttributeSet::getSource(void) const
{
    return mSource;
}

inline void SMAttributeSet::setSource(SMArgumentEntry::eValueSource source)
{
    mSource = source;
}

inline const QString& SMAttributeSet::getValue(void) const
{
    return mValue;
}

inline void SMAttributeSet::setValue(const QString& value)
{
    mValue = value;
}

inline const QString& SMAttributeSet::getExpression(void) const
{
    return mExpression;
}

inline void SMAttributeSet::setExpression(const QString& expression)
{
    mExpression = expression;
}

inline const QString& SMTimerStart::getTimer(void) const
{
    return mTimer;
}

inline void SMTimerStart::setTimer(const QString& timer)
{
    mTimer = timer;
}

inline bool SMTimerStart::hasTimeoutOverride(void) const
{
    return mHasTimeout;
}

inline uint32_t SMTimerStart::getTimeout(void) const
{
    return mTimeout;
}

inline bool SMTimerStart::hasRepeatOverride(void) const
{
    return mHasRepeat;
}

inline uint32_t SMTimerStart::getRepeat(void) const
{
    return mRepeat;
}

inline const QString& SMTimerStop::getTimer(void) const
{
    return mTimer;
}

inline void SMTimerStop::setTimer(const QString& timer)
{
    mTimer = timer;
}

inline const QString& SMEventSend::getEvent(void) const
{
    return mEvent;
}

inline void SMEventSend::setEvent(const QString& event)
{
    mEvent = event;
}

inline const QList<SMArgumentEntry>& SMEventSend::getArguments(void) const
{
    return mArguments;
}

inline QList<SMArgumentEntry>& SMEventSend::getArguments(void)
{
    return mArguments;
}

inline const QString& SMInlineCode::getBody(void) const
{
    return mBody;
}

inline void SMInlineCode::setBody(const QString& body)
{
    mBody = body;
}

inline ElementBase* SMOperationList::getParent(void) const
{
    return mParent;
}

inline const QList<SMOperationBase*>& SMOperationList::getOperations(void) const
{
    return mOperations;
}

inline int SMOperationList::getCount(void) const
{
    return static_cast<int>(mOperations.size());
}

inline bool SMOperationList::isEmpty(void) const
{
    return mOperations.isEmpty();
}

inline SMOperationBase* SMOperationList::at(int index) const
{
    return ((index >= 0) && (index < mOperations.size())) ? mOperations.at(index) : nullptr;
}

#endif  // LUSAN_DATA_SM_SMOPERATION_HPP
