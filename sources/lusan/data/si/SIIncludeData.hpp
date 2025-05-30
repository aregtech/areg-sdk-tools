﻿#ifndef LUSAN_DATA_SI_SIINCLUDEDATA_HPP
#define LUSAN_DATA_SI_SIINCLUDEDATA_HPP
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
 *  \file        lusan/data/si/SIIncludeData.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Include Data.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/DocumentElem.hpp"
#include "lusan/data/common/TEDataContainer.hpp"

#include "lusan/data/common/IncludeEntry.hpp"
#include <QList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class SIDataTypeData;

 /**
  * \class   SIIncludeData
  * \brief   Manages include data for service interfaces.
  **/
class SIIncludeData : public TEDataContainer< IncludeEntry, DocumentElem >
{
//////////////////////////////////////////////////////////////////////////
// public methods
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    SIIncludeData(ElementBase * parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   entries    The list of include entries.
     **/
    SIIncludeData(const QList<IncludeEntry>& entries, ElementBase* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Checks if the parameter is valid.
     * \return  True if the parameter is valid, false otherwise.
     **/
    virtual bool isValid() const override;

    /**
     * \brief   Reads include data from an XML stream.
     * \param   xml         The XML stream reader.
     * \return  True if the include data was successfully read, false otherwise.
     **/
    virtual bool readFromXml(QXmlStreamReader& xml) override;

    /**
     * \brief   Writes include data to an XML stream.
     * \param   xml         The XML stream writer.
     **/
    virtual void writeToXml(QXmlStreamWriter& xml) const override;

    /**
     * \brief   Validates the include data.
     * \param   dataTypes   The data type data to validate the includes.
     **/
    void validate(const SIDataTypeData& dataTypes);

    /**
     * \brief   Creates an include entry.
     * \param   location    The file path included in service interface.
     * \return  Returns the created include entry.
     **/
    IncludeEntry* createInclude(const QString location);

    /**
     * \brief   Inserts new IncludeEntry at the given position in SIIncludeData.
     * \param   location    The file path included in service interface.
     * \return  Valid pointer to the new created include element. Otherwise, returns nullptr.
     **/
    IncludeEntry* insertInclude(int position, const QString& location);
};

#endif  // LUSAN_DATA_SI_SIINCLUDEDATA_HPP
