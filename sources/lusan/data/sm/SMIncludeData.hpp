#ifndef LUSAN_DATA_SM_SMINCLUDEDATA_HPP
#define LUSAN_DATA_SM_SMINCLUDEDATA_HPP
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
 *  \file        lusan/data/sm/SMIncludeData.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM includes registry. Reuses IncludeEntry.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/DocumentElem.hpp"
#include "lusan/data/common/TEDataContainer.hpp"
#include "lusan/data/common/IncludeEntry.hpp"

/**
 * \class   SMIncludeData
 * \brief   The `IncludeList` registry: header files to include in generated sources.
 *          Same shape as the Service-Interface include list, so IncludeEntry is reused.
 **/
class SMIncludeData : public TEDataContainer<IncludeEntry, DocumentElem>
{
public:
    SMIncludeData(ElementBase* parent = nullptr);

    virtual bool isValid(void) const override;
    virtual bool readFromXml(QXmlStreamReader& xml) override;
    virtual void writeToXml(QXmlStreamWriter& xml) const override;

    /**
     * \brief   Creates a new include entry appended at the end of the list.
     * \param   location    The header location (its unique name).
     * \return  Pointer to the created entry, or nullptr if the location already exists.
     **/
    IncludeEntry* createInclude(const QString& location);
};

#endif  // LUSAN_DATA_SM_SMINCLUDEDATA_HPP
