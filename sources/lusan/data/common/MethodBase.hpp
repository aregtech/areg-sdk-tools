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

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/TEDataContainer.hpp"
#include "lusan/data/common/DocumentElem.hpp"

#include <QList>
#include <QString>
#include "lusan/data/common/MethodParameter.hpp"

 /**
  * \class   MethodBase
  * \brief   Represents a method base in the Lusan application.
  **/
class MethodBase    : public TEDataContainer<MethodParameter, DocumentElem >
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Default constructor.
     * \param   parent  The parent element.
     **/
    MethodBase(ElementBase * parent = nullptr);

    /**
     * \brief   Constructor with initialization.
     * \param   id              The ID of the method.
     * \param   name            The name of the method.
     * \param   description     The description of the method.
     * \param   parent          The parent element.
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
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    
    /**
     * \brief   Checks if the parameter is valid.
     * \return  True if the parameter is valid, false otherwise.
     **/
    virtual bool isValid() const override;
    
//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

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
     * \brief   Adds a new parameter to the method.
     * \param   name    The unique name of the parameter.
     * \return  Returns the pointer to the added parameter.
     **/
    MethodParameter* addParam(const QString& name);

    /**
     * \brief   Inserts a new parameter to the method at the specified position.
     * \param   position    The position to insert the parameter.
     * \param   name        The unique name of the parameter.
     * \return  Returns the pointer to the added parameter.
     **/
    MethodParameter* insertParam(int position, const QString& name);

    /**
     * \brief   Removes the parameter from the method by name.
     * \param   name    The name of the parameter to remove.
     **/
    void removeParam(const QString& name);

    /**
     * \brief   Removes the parameter from the method by ID.
     * \param   id  The ID of the parameter to remove.
     **/
    void removeParam(uint32_t id);

    /**
     * \brief   Returns the parameter by name.
     * \param   name    The name of the parameter to return.
     * \return  Returns the pointer to the parameter by name.
     **/
    DataTypeBase* getParamType(const QString& name) const;

    /**
     * \brief   Returns the parameter by ID.
     * \param   id  The ID of the parameter to return.
     * \return  Returns the pointer to the parameter by ID.
     **/
    DataTypeBase* getParamType(uint32_t id) const;

    /**
     * \brief   Validates the method.
     * \param   customTypes The list of custom data types to validate.
     * \return  Returns true if the method is valid, false otherwise.
     **/
    bool validate(const QList<DataTypeCustom*>& customTypes);

    /**
     * \brief   Invalidates the method.
     **/
    void invalidate(void);

    /**
     * \brief   Checks if the parameter with the given ID has a default value.
     * \param   id  The ID of the parameter.
     * \return  True if the parameter has a default value, false otherwise.
     **/
    bool hasDefaultValue(uint32_t id) const;

    /**
     * \brief   Checks if the parameter with the given name has a default value.
     * \param   name    The name of the parameter.
     * \return  True if the parameter has a default value, false otherwise.
     **/
    bool hasDefaultValue(const QString& name) const;

    /**
     * \brief   Checks if the parameter with the given ID can have a default value.
     * \param   id  The ID of the parameter.
     * \return  True if the parameter can have a default value, false otherwise.
     **/
    bool canHaveDefaultValue(uint32_t id) const;

    /**
     * \brief   Checks if the parameter with the given name can have a default value.
     * \param   name    The name of the parameter.
     * \return  True if the parameter can have a default value, false otherwise.
     **/
    bool canHaveDefaultValue(const QString& name) const;

    /**
     * \brief   Gets the first position with a default value.
     * \return  The first position with a default value.
     **/
    int firsPositionWithDefault(void) const;

    /**
     * \brief   Checks if the parameter with the given ID can switch its default value.
     * \param   id  The ID of the parameter.
     * \return  True if the parameter can switch its default value, false otherwise.
     **/
    bool canSwitchDefaultValue(uint32_t id) const;

    /**
     * \brief   Checks if the parameter with the given name can switch its default value.
     * \param   name    The name of the parameter.
     * \return  True if the parameter can switch its default value, false otherwise.
     **/
    bool canSwitchDefaultValue(const QString& name) const;

    /**
     * \brief   Checks if the parameter with the given ID is in the last position with a default value.
     * \param   id  The ID of the parameter.
     * \return  True if the parameter is in the last position with a default value, false otherwise.
     **/
    bool isLastPositionWithDefault(uint32_t id) const;

    /**
     * \brief   Checks if the parameter with the given name is in the last position with a default value.
     * \param   name    The name of the parameter.
     * \return  True if the parameter is in the last position with a default value, false otherwise.
     **/
    bool isLastPositionWithDefault(const QString& name) const;

    /**
     * \brief   Sets the default value for the parameter with the given ID.
     * \param   id          The ID of the parameter.
     * \param   newValue    The new default value to set.
     * \return  Pointer to the updated parameter.
     **/
    MethodParameter* setDefaultValue(uint32_t id, const QString & newValue);

    /**
     * \brief   Sets the default value for the parameter with the given name.
     * \param   name        The name of the parameter.
     * \param   newValue    The new default value to set.
     * \return  Pointer to the updated parameter.
     **/
    MethodParameter* setDefaultValue(const QString& name, const QString& newValue);

    /**
     * \brief   Makes the specified value the default for the parameter with the given ID.
     * \param   id          The ID of the parameter.
     * \param   makeDefault Flag indicating whether to make the value default.
     * \param   value       The value to set as default.
     * \return  Pointer to the updated parameter.
     **/
    MethodParameter* makeValueDefault(uint32_t id, bool makeDefault, const QString& value);

    /**
     * \brief   Makes the specified value the default for the parameter with the given name.
     * \param   name        The name of the parameter.
     * \param   makeDefault Flag indicating whether to make the value default.
     * \param   value       The value to set as default.
     * \return  Pointer to the updated parameter.
     **/
    MethodParameter* makeValueDefault(const QString& name, bool makeDefault, const QString& value);

    /**
     * \brief   Determines whether the parameter at the specified index can be swapped with the previous parameter.
     *
     * \details A parameter can be swapped with the one to its left only if the following conditions are met:
     *          - The parameter is not in the first position.
     *          - Swapping must not violate default value constraints. Specifically:
     *              - If the given parameter has a default value but the previous parameter does not,
     *                the swap is not allowed.
     *
     * \param   id  The unique identifier of the parameter to check.
     * \return  True if the parameter can be swapped with the previous one; otherwise, false.
     */
    bool canSwapParamLeft(uint32_t id) const;

    /**
     * \brief   Determines whether the parameter at the specified index can be swapped with the next parameter.
     *
     * \details A parameter can be swapped with the one to its right only if the following conditions are met:
     *          - The parameter is not in the last position.
     *          - Swapping must not violate default value constraints. Specifically:
     *              - If the given parameter has a default value but the next parameter does not,
     *                the swap is not allowed.
     *
     * \param   id  The unique identifier of the parameter to check.
     * \return  True if the parameter can be swapped with the next one; otherwise, false.
     */
    bool canSwapParamRight(uint32_t id) const;

    /**
     * \brief   Determines whether the parameter with the specified name can be swapped with the previous parameter.
     *
     * \details A parameter can be swapped with the one to its left only if the following conditions are met:
     *          - The parameter is not in the first position.
     *          - Swapping must not violate default value constraints. Specifically:
     *              - If the given parameter has a default value but the previous parameter does not,
     *                the swap is not allowed.
     *
     * \param   name    The name of the parameter to check.
     * \return  True if the parameter can be swapped with the previous one; otherwise, false.
     */
    bool canSwapParamLeft(const QString& name) const;

    /**
     * \brief   Determines whether the parameter with the specified name can be swapped with the next parameter.
     *
     * \details A parameter can be swapped with the one to its right only if the following conditions are met:
     *          - The parameter is not in the last position.
     *          - Swapping must not violate default value constraints. Specifically:
     *              - If the given parameter has a default value but the next parameter does not,
     *                the swap is not allowed.
     *
     * \param   name    The name of the parameter to check.
     * \return  True if the parameter can be swapped with the next one; otherwise, false.
     */
    bool canSwapParamRight(const QString& name) const;

    /**
     * \brief   Swaps the parameter at the specified position in the parameter list with the previous parameter.
     *
     * \details The swap is performed only if the following conditions are met:
     *          - The parameter is not in the first position.
     *          - Swapping does not violate default value constraints. Specifically:
     *              - If the given parameter has a default value but the previous parameter does not,
     *                the swap is not allowed.
     *
     * \param   id  The unique identifier of the parameter to swap.
     * \return  True if the parameter was successfully swapped with the previous one; otherwise, false.
     */
    bool canSwapParamLeft(int position) const;

    /**
     * \brief   Swaps the parameter at the specified position in the parameter list with the next parameter.
     *
     * \details The swap is performed only if the following conditions are met:
     *          - The parameter is not in the last position.
     *          - Swapping does not violate default value constraints. Specifically:
     *              - If the given parameter has a default value but the next parameter does not,
     *                the swap is not allowed.
     *
     * \param   id  The unique identifier of the parameter to swap.
     * \return  True if the parameter was successfully swapped with the next one; otherwise, false.
     */
    bool canSwapParamRight(int position) const;

protected:

    /**
     * \brief   Checks if the parameter at the given index can have a default value.
     * \param   index   The index of the parameter.
     * \return  True if the parameter can have a default value, false otherwise.
     **/
    bool canHaveDefaultValue(int index) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
protected:
    QString mName;          //!< The method name.
    QString mDescription;   //!< The method description.
};

#endif // LUSAN_DATA_COMMON_METHODBASE_HPP
