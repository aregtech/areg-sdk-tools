#ifndef LUSAN_DATA_SI_SERVICEINTERFACEDATA_HPP
#define LUSAN_DATA_SI_SERVICEINTERFACEDATA_HPP
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
 *  \file        lusan/data/si/ServiceInterfaceData.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Data.
 *
 ************************************************************************/

#include "lusan/common/ElementBase.hpp"

#include "lusan/data/si/SIOverviewData.hpp"
#include "lusan/data/si/SIDataTypeData.hpp"
#include "lusan/data/si/SIAttributeData.hpp"
#include "lusan/data/si/SIMethodData.hpp"
#include "lusan/data/si/SIConstantData.hpp"
#include "lusan/data/si/SIIncludeData.hpp"

#include "lusan/common/VersionNumber.hpp"
#include "lusan/common/XmlSI.hpp"

/**
 * \class   ServiceInterfaceData
 * \brief   Represents the service interface data in the Lusan application.
 **/
class ServiceInterfaceData  : public ElementBase
{
//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
    static constexpr const char* const XML_FORMAT_VERSION   { "1.0.0" }; //!< The XML format version.

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     **/
    ServiceInterfaceData(void);

    ServiceInterfaceData(const QString& filePath);

    /**
     * \brief   Destructor.
     **/
    virtual ~ServiceInterfaceData(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Reads data from a file.
     * \param   filePath    The file path to read the data.
     * \return  Returns true if the data was successfully read, false otherwise.
     **/
    bool readFromFile(const QString& filePath);

    /**
     * \brief   Writes data to a file.
     * \param   filePath    The file path to write the data.
     * \return  Returns true if the data was successfully written, false otherwise.
     **/
    bool writeToFile(const QString& filePath = "") const;

    /**
     * \brief   Reads data from an XML stream.
     * \param   xml     The XML stream reader.
     * \return  True if the data was successfully read, false otherwise.
     **/
    bool readFromXml(QXmlStreamReader& xml);

    /**
     * \brief   Writes data to an XML stream.
     * \param   xml     The XML stream writer.
     **/
    void writeToXml(QXmlStreamWriter& xml) const;

    /**
     * \brief   Returns the file path of the service interface data.
     * \return  The file path of the service interface data.
     **/
    inline const QString& getFilePath(void) const;

    /**
     * \brief   Gets the overview data.
     * \return  The overview data.
     **/
    inline const SIOverviewData& getOverviewData(void) const;
    inline SIOverviewData& getOverviewData(void);

    /**
     * \brief   Gets the data type data.
     * \return  The data type data.
     **/
    inline const SIDataTypeData& getDataTypeData(void) const;
    inline SIDataTypeData& getDataTypeData(void);

    /**
     * \brief   Gets the attribute data.
     * \return  The attribute data.
     **/
    inline const SIAttributeData& getAttributeData(void) const;
    inline SIAttributeData& getAttributeData(void);

    /**
     * \brief   Gets the method data.
     * \return  The method data.
     **/
    inline const SIMethodData& getMethodData(void) const;
    inline SIMethodData& getMethodData(void);

    /**
     * \brief   Gets the constant data.
     * \return  The constant data.
     **/
    inline const SIConstantData& getConstantData(void) const;
    inline SIConstantData& getConstantData(void);

    /**
     * \brief   Gets the include data.
     * \return  The include data.
     **/
    inline const SIIncludeData& getIncludeData(void) const;
    inline SIIncludeData& getIncludeData(void);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QString          mFilePath;       //!< The file path of the service interface data.
    SIOverviewData   mOverviewData;   //!< The overview data.
    SIDataTypeData   mDataTypeData;   //!< The data type data.
    SIAttributeData  mAttributeData;  //!< The attribute data.
    SIMethodData     mMethodData;     //!< The method data.
    SIConstantData   mConstantData;   //!< The constant data.
    SIIncludeData    mIncludeData;    //!< The include data.
};

//////////////////////////////////////////////////////////////////////////
// ServiceInterfaceData inline methods
//////////////////////////////////////////////////////////////////////////

inline const QString& ServiceInterfaceData::getFilePath(void) const
{
    return mFilePath;
}

inline const SIOverviewData& ServiceInterfaceData::getOverviewData(void) const
{
    return mOverviewData;
}

inline SIOverviewData& ServiceInterfaceData::getOverviewData(void)
{
    return mOverviewData;
}

inline const SIDataTypeData& ServiceInterfaceData::getDataTypeData(void) const
{
    return mDataTypeData;
}

inline SIDataTypeData& ServiceInterfaceData::getDataTypeData(void)
{
    return mDataTypeData;
}

inline const SIAttributeData& ServiceInterfaceData::getAttributeData(void) const
{
    return mAttributeData;
}

inline SIAttributeData& ServiceInterfaceData::getAttributeData(void)
{
    return mAttributeData;
}

inline const SIMethodData& ServiceInterfaceData::getMethodData(void) const
{
    return mMethodData;
}

inline SIMethodData& ServiceInterfaceData::getMethodData(void)
{
    return mMethodData;
}

inline const SIConstantData& ServiceInterfaceData::getConstantData(void) const
{
    return mConstantData;
}

inline SIConstantData& ServiceInterfaceData::getConstantData(void)
{
    return mConstantData;
}

inline const SIIncludeData& ServiceInterfaceData::getIncludeData(void) const
{
    return mIncludeData;
}

inline SIIncludeData& ServiceInterfaceData::getIncludeData(void)
{
    return mIncludeData;
}

#endif // LUSAN_DATA_SI_SERVICEINTERFACEDATA_HPP
