#ifndef LUSAN_DATA_COMMON_METHODENTRY_HPP
#define LUSAN_DATA_COMMON_METHODENTRY_HPP
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
 *  \file        lusan/data/common/MethodEntry.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, one declared method, shared by every document editor.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/MethodBase.hpp"

#include <QList>
#include <QString>

/**
 * \brief   One kind a method may be declared as. A service interface offers Request, Response
 *          and Broadcast; a state machine offers Trigger, Action and Condition. The kinds a
 *          document offers, in the order they are offered, are its configuration.
 *
 *          The flags say what a method of this kind carries beyond a name, parameters and a
 *          description, so the entry, the section and the page all read the same answer from
 *          one place.
 **/
struct MethodKind
{
    QString token;          //!< The `MethodType` value written to and read from the file.
    QString label;          //!< The caption of the type radio button and the Add menu entry.
    QString icon;           //!< The resource path of the row icon, empty for none.
    bool    hasReply;       //!< A method of this kind names the method that answers it.
    bool    isReply;        //!< A method of this kind may be named as an answer.
    bool    hasReturn;      //!< A method of this kind declares the type it returns.
    bool    hasImplement;   //!< A method of this kind says how it is implemented, and carries a
                            //!< verbatim body when it is implemented here.
};

/**
 * \brief   What a document's methods are: the kinds it offers, the return type a kind that has
 *          one starts with, and how a parameter's default value is spelled in its file.
 *
 *          Service interface: Request / Response / Broadcast, a request names its response, a
 *          default is a `<Value IsDefault="true">` child.
 *          State machine: Trigger / Action / Condition, a condition returns a value and says how
 *          it is implemented, a default is a `Default` attribute.
 **/
struct MethodConfig
{
    QList<MethodKind>   kinds;                      //!< The kinds, in display order.
    QString             defaultReturn;              //!< What a returning kind starts with.
    bool                paramDefaultAsAttribute;    //!< A parameter default is an attribute.
    bool                omitEmptyDescription;       //!< A method with no description writes none.
};

/**
 * \class   MethodEntry
 * \brief   One declared method: a name, an ordered parameter list, a description, a deprecation
 *          state, and the kind it was declared as. What else it carries follows its kind -- the
 *          method that answers it, or the type it returns plus how it is implemented -- so one
 *          class serves both editors and neither file grows a field it never had.
 *
 *          The entry keeps a copy of its section's configuration, so it writes the shape its
 *          document expects wherever it is serialized: the document, the clipboard, or a paste
 *          fragment.
 **/
class MethodEntry : public MethodBase
{
//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \enum    eImplement
     * \brief   How a method that has an implementation mode is implemented.
     **/
    enum class eImplement
    {
          Handler       //!< Declared here, implemented by the user at run time.
        , Embedded      //!< The body is written here and generated verbatim.
    };

    static constexpr const char* const  STR_IMPL_HANDLER    { "Handler"   };
    static constexpr const char* const  STR_IMPL_EMBEDDED   { "Embedded"  };
    static constexpr const char* const  DEFAULT_RETURN      { "bool"      };

    static eImplement fromImplementString(const QString& implement);
    static const char* toString(eImplement implement);

    /**
     * \brief   The configuration an entry carries until a section stamps it: no kinds and every
     *          field, so a stray entry never silently drops what it holds on the way to a file.
     **/
    static const MethodConfig& defaultConfig();

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    MethodEntry(ElementBase* parent = nullptr);
    MethodEntry(uint32_t id, const QString& name, int kind, const MethodConfig& config, ElementBase* parent = nullptr);
    MethodEntry(const MethodEntry& src);
    MethodEntry(MethodEntry&& src) noexcept;

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:
    MethodEntry& operator = (const MethodEntry& other);
    MethodEntry& operator = (MethodEntry&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    //!< The index of the kind this method was declared as, in its configuration's kind list.
    inline int getKind() const;
    void setKind(int kind);

    //!< The kind itself. A method whose index is out of range answers an empty kind, which
    //!< carries nothing -- that is what an entry with no configuration is.
    const MethodKind& kind() const;

    //!< The `MethodType` spelling of this method's kind.
    QString getType() const;

    //!< True when the method's kind carries the matching field.
    inline bool hasReply() const;
    inline bool isReplyKind() const;
    inline bool hasReturn() const;
    inline bool hasImplement() const;

    //!< The name of the method that answers this one, empty when none is named.
    inline const QString& getReply() const;
    inline void setReply(const QString& name);

    inline const QString& getReturn() const;
    inline void setReturn(const QString& type);

    inline eImplement getImplement() const;
    inline void setImplement(eImplement implement);

    //!< True when the method owns its body here rather than declaring it for a handler.
    inline bool isEmbedded() const;

    inline const QString& getBody() const;
    inline void setBody(const QString& body);

    inline bool getIsDeprecated() const;
    inline void setIsDeprecated(bool isDeprecated);
    inline const QString& getDeprecateHint() const;
    inline void setDeprecateHint(const QString& hint);

    //!< What this method's document declares. A section stamps every entry it creates or reads.
    inline const MethodConfig& getConfig() const;
    void setConfig(const MethodConfig& config);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   A method is identified by its name; a parameter list may legitimately be empty.
     **/
    virtual bool isValid() const override;

    /**
     * \brief   Reads one `Method` element. Accepts a parameter default in either spelling.
     **/
    virtual bool readFromXml(QXmlStreamReader& xml) override;

    /**
     * \brief   Writes one `Method` element in this entry's configured shape.
     **/
    virtual void writeToXml(QXmlStreamWriter& xml) const override;

    virtual QIcon getIcon(ElementBase::eDisplay display) const override;

    virtual QString getString(ElementBase::eDisplay display) const override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    void readParamList(QXmlStreamReader& xml);
    void writeParamList(QXmlStreamWriter& xml) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    MethodConfig    mConfig;        //!< What this method's document declares.
    int             mKind;          //!< The index of this method's kind.
    QString         mReply;         //!< The name of the method that answers this one.
    QString         mReturn;        //!< The type this method returns.
    eImplement      mImplement;     //!< How this method is implemented.
    QString         mBody;          //!< The verbatim body, written as CDATA.
    bool            mIsDeprecated;  //!< True while the method is marked deprecated.
    QString         mDeprecateHint; //!< Why the method is deprecated.
};

//////////////////////////////////////////////////////////////////////////
// MethodEntry inline methods
//////////////////////////////////////////////////////////////////////////

inline int MethodEntry::getKind() const
{
    return mKind;
}

inline bool MethodEntry::hasReply() const
{
    return kind().hasReply;
}

inline bool MethodEntry::isReplyKind() const
{
    return kind().isReply;
}

inline bool MethodEntry::hasReturn() const
{
    return kind().hasReturn;
}

inline bool MethodEntry::hasImplement() const
{
    return kind().hasImplement;
}

inline const QString& MethodEntry::getReply() const
{
    return mReply;
}

inline void MethodEntry::setReply(const QString& name)
{
    mReply = name;
}

inline const QString& MethodEntry::getReturn() const
{
    return mReturn;
}

inline void MethodEntry::setReturn(const QString& type)
{
    mReturn = type;
}

inline MethodEntry::eImplement MethodEntry::getImplement() const
{
    return mImplement;
}

inline void MethodEntry::setImplement(MethodEntry::eImplement implement)
{
    mImplement = implement;
}

inline bool MethodEntry::isEmbedded() const
{
    return hasImplement() && (mImplement == eImplement::Embedded);
}

inline const QString& MethodEntry::getBody() const
{
    return mBody;
}

inline void MethodEntry::setBody(const QString& body)
{
    mBody = body;
}

inline bool MethodEntry::getIsDeprecated() const
{
    return mIsDeprecated;
}

inline void MethodEntry::setIsDeprecated(bool isDeprecated)
{
    mIsDeprecated = isDeprecated;
    if (isDeprecated == false)
    {
        mDeprecateHint.clear();
    }
}

inline const QString& MethodEntry::getDeprecateHint() const
{
    return mDeprecateHint;
}

inline void MethodEntry::setDeprecateHint(const QString& hint)
{
    mDeprecateHint = hint;
}

inline const MethodConfig& MethodEntry::getConfig() const
{
    return mConfig;
}

#endif  // LUSAN_DATA_COMMON_METHODENTRY_HPP
