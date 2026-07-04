#ifndef LUSAN_DATA_SM_SMCONSTANTDATA_HPP
#define LUSAN_DATA_SM_SMCONSTANTDATA_HPP
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
 *  \file        lusan/data/sm/SMConstantData.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM constants registry. Reuses ConstantEntry.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/DocumentElem.hpp"
#include "lusan/data/common/TEDataContainer.hpp"
#include "lusan/data/common/ConstantEntry.hpp"

/**
 * \class   SMConstantData
 * \brief   The `ConstantList` registry: an ordered container of ConstantEntry. The FSM
 *          constant has exactly the same shape as the Service-Interface constant
 *          (name + type + value + description), so ConstantEntry is reused verbatim.
 **/
class SMConstantData : public TEDataContainer<ConstantEntry, DocumentElem>
{
public:
    SMConstantData(ElementBase* parent = nullptr);

    virtual bool isValid(void) const override;
    virtual bool readFromXml(QXmlStreamReader& xml) override;
    virtual void writeToXml(QXmlStreamWriter& xml) const override;

    /**
     * \brief   Creates a new constant appended at the end of the list.
     * \param   name    The unique name of the new constant.
     * \return  Pointer to the created constant, or nullptr if the name already exists.
     **/
    ConstantEntry* createConstant(const QString& name);
};

#endif  // LUSAN_DATA_SM_SMCONSTANTDATA_HPP
