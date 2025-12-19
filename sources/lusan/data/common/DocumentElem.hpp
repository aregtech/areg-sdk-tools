#ifndef LUSAN_DATA_COMMON_DOCUMENTELEM_HPP
#define LUSAN_DATA_COMMON_DOCUMENTELEM_HPP
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
 *  \file        lusan/data/common/DocumentElem.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Document element object to save and read data from document.
 *
 ************************************************************************/

 /************************************************************************
  * Include files
  ************************************************************************/
#include "lusan/common/ElementBase.hpp"
#include <QSize>

class QXmlStreamReader;
class QXmlStreamWriter;

class DocumentElem : public ElementBase
{
//////////////////////////////////////////////////////////////////////////
// Constants and types
//////////////////////////////////////////////////////////////////////////
public:
    static constexpr const QSize  IconSize  {16, 16};
    
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief Constructor with optional parent element.
     * \param parent Pointer to the parent element.
     */
    DocumentElem(ElementBase* parent = nullptr);

    /**
     * \brief Constructor with ID and optional parent element.
     * \param id The ID of the element.
     * \param parent Pointer to the parent element.
     */
    DocumentElem(unsigned int id, ElementBase* parent = nullptr);

    /**
     * \brief   Parameterized constructor.
     * \param   src The source element to copy from.
     **/
    DocumentElem(const DocumentElem & src);

    /**
     * \brief   Move constructor.
     * \param   src The source element to move from.
     **/
    DocumentElem(DocumentElem&& src) noexcept;

    /**
     * \brief   Destructor.
     **/
    virtual ~DocumentElem(void) = default;

public:
    /**
     * \brief   Copy assignment operator.
     * \param   src The source element to copy from.
     * \return  Reference to this element.
     **/
    DocumentElem& operator = (const DocumentElem& src);

    /**
     * \brief   Move assignment operator.
     * \param   src The source element to move from.
     * \return  Reference to this element.
     **/
    DocumentElem& operator = (DocumentElem&& src) noexcept;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////

    /**
     * \brief   Checks if the parameter is valid.
     * \return  True if the parameter is valid, false otherwise.
     **/
    virtual bool isValid() const = 0;

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

protected:

    /**
     * \brief   Writes the text element to XML stream.
     * \param   xml             The XML stream writer.
     * \param   elemName        The name of the element to write.
     * \param   elemValue       The value of the element to write.
     * \param   skipIfEmpty     If true, the element is skipped if the value is empty.
     **/
    void writeTextElem(QXmlStreamWriter& xml, const char* elemName, const QString elemValue, bool skipIfEmpty) const;
};

#endif  // LUSAN_DATA_COMMON_DOCUMENTELEM_HPP
