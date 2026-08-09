#ifndef LUSAN_APPLICATION_SI_SICONSTANT_HPP
#define LUSAN_APPLICATION_SI_SICONSTANT_HPP
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
 *  \file        lusan/view/si/SIConstant.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Constants page.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/view/common/ConstantPage.hpp"
#include "lusan/view/common/IEDataTypeConsumer.hpp"

/**
 * \brief   The Service Interface Constants page. The editing itself is the shared
 *          \ref ConstantPage; what the service interface adds is the bridge from its data type
 *          page, which announces its changes by direct call rather than through the document
 *          notifier the shared page listens to.
 **/
class SIConstant : public ConstantPage
                 , public IEDataTypeConsumer
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SIConstant(ConstantModel& model, QWidget* parent = nullptr);

    virtual ~SIConstant(void) = default;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Triggered when a new data type is created.
     **/
    virtual void dataTypeCreated(DataTypeCustom* dataType) override;

    /**
     * \brief   Triggered when a data type is converted to another category.
     **/
    virtual void dataTypeConverted(DataTypeCustom* oldType, DataTypeCustom* newType) override;

    /**
     * \brief   Triggered when a data type is deleted and invalidated.
     **/
    virtual void dataTypeDeleted(DataTypeCustom* dataType) override;

    /**
     * \brief   Triggered when a data type is updated.
     **/
    virtual void dataTypeUpdated(DataTypeCustom* dataType) override;
};

#endif // LUSAN_APPLICATION_SI_SICONSTANT_HPP
