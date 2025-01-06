#ifndef LUSAN_DATA_COMMON_METHODBASE_HPP
#define LUSAN_DATA_COMMON_METHODBASE_HPP
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
 *  \file        lusan/data/common/MethodBase.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Method Base.
 *
 ************************************************************************/
#include "lusan/common/ElementBase.hpp"

#include <QList>
#include <QString>
#include "lusan/data/common/MethodParameter.hpp"

 /**
  * \class   MethodBase
  * \brief   Represents a method base in the Lusan application.
  **/
class MethodBase    : public ElementBase
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    MethodBase(ElementBase * parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   id              The ID of the method.
     * \param   name            The name of the method.
     * \param   description     The description of the method.
     **/
    MethodBase(uint32_t id, const QString& name, const QString& description, ElementBase* parent = nullptr);

    /**
     * \brief   Copy constructor.
     * \param   src     The source object to copy from.
     **/
    MethodBase(const MethodBase& src);

    /**
     * \brief   Move constructor.
     * \param   src     The source object to move from.
     **/
    MethodBase(MethodBase&& src) noexcept;

    /**
     * \brief   Destructor.
     **/
    virtual ~MethodBase(void);

//////////////////////////////////////////////////////////////////////////
// Operators
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Copy assignment operator.
     * \param   other   The other object to copy from.
     * \return  Reference to this object.
     **/
    MethodBase& operator = (const MethodBase& other);

    /**
     * \brief   Move assignment operator.
     * \param   other   The other object to move from.
     * \return  Reference to this object.
     **/
    MethodBase& operator = (MethodBase&& other) noexcept;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Adds a method parameter to the list.
     * \param   parameter   The method parameter to add.
     * \return  True if the parameter was added, false otherwise.
     **/
    bool addParameter(MethodParameter&& parameter);

    /**
     * \brief   Removes a method parameter from the list by name.
     * \param   name    The name of the parameter to remove.
     * \return  True if the parameter was removed, false otherwise.
     **/
    bool removeParameter(const QString& name);

    /**
     * \brief   Inserts a method parameter into the list at the specified position.
     * \param   index       The position to insert the parameter.
     * \param   parameter   The method parameter to insert.
     * \return  True if the parameter was inserted, false otherwise.
     **/
    bool insertParameter(int index, MethodParameter&& parameter);

    /**
     * \brief   Searches for a method parameter by name.
     * \param   name    The name of the parameter to search for.
     * \return  Pointer to the method parameter if found, nullptr otherwise.
     **/
    MethodParameter* findParameter(const QString& name) const;

    /**
     * \brief   Gets the name of the method.
     * \return  The name of the method.
     **/
    const QString& getName() const;

    /**
     * \brief   Sets the name of the method.
     * \param   name    The name of the method.
     **/
    void setName(const QString& name);

    /**
     * \brief   Gets the description of the method.
     * \return  The description of the method.
     **/
    const QString& getDescription() const;

    /**
     * \brief   Sets the description of the method.
     * \param   description     The description of the method.
     **/
    void setDescription(const QString& description);

    /**
     * \brief   remove all entries and frees resources.
     **/
    void removeAll(void);

    /**
     * \brief   Reads data from an XML stream.
     * \param   xml     The XML stream reader.
     * \return  True if the data was successfully read, false otherwise.
     **/
    virtual bool readFromXml(QXmlStreamReader& xml) = 0;

    /**
     * \brief   Writes data to an XML stream.
     * \param   xml     The XML stream writer.
     **/
    virtual void writeToXml(QXmlStreamWriter& xml) const = 0;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
protected:
    QString mName;          //!< The method name.
    QString mDescription;   //!< The method description.
    QList<MethodParameter> mParameters; //!< The list of method parameters.
};

#endif // LUSAN_DATA_COMMON_METHODBASE_HPP
