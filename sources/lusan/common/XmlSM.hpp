#ifndef LUSAN_COMMON_XMLSM_HPP
#define LUSAN_COMMON_XMLSM_HPP
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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/common/XmlSM.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, State Machine (`.fsml`) XML tags.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/common/XmlSI.hpp"

/**
 * \brief   The list of element and attribute names used in the State Machine
 *          (`.fsml`) XML document, the sibling of \ref XmlSI. Where `.fsml` reuses the
 *          `.siml` element vocabulary (the DataType sections, `ParamList`/`Parameter`,
 *          `Description`, `Location`, `ID`/`Name`/`DataType`), the constants reference the
 *          same \ref XmlSI string values so the two formats cannot drift apart.
 **/
namespace XmlSM
{
    //////////////////////////////////////////////////////////////////////////
    // Element names
    //////////////////////////////////////////////////////////////////////////

    constexpr const char* const xmlSMElementStateMachine    { "StateMachine" };
    constexpr const char* const xmlSMElementOverview        { XmlSI::xmlSIElementOverview };
    constexpr const char* const xmlSMElementDescription     { XmlSI::xmlSIElementDescription };
    constexpr const char* const xmlSMElementDeprecateHint   { XmlSI::xmlSIElementDeprecateHint };

    constexpr const char* const xmlSMElementDataTypeList    { XmlSI::xmlSIElementDataTypeList };
    constexpr const char* const xmlSMElementDataType        { XmlSI::xmlSIElementDataType };

    constexpr const char* const xmlSMElementAttributeList   { XmlSI::xmlSIElementAttributeList };
    constexpr const char* const xmlSMElementAttribute       { XmlSI::xmlSIElementAttribute };

    constexpr const char* const xmlSMElementEventList       { "EventList" };
    constexpr const char* const xmlSMElementEvent           { "Event" };

    constexpr const char* const xmlSMElementParamList       { XmlSI::xmlSIElementParamList };
    constexpr const char* const xmlSMElementParameter       { XmlSI::xmlSIElementParameter };

    constexpr const char* const xmlSMElementTimerList       { "TimerList" };
    constexpr const char* const xmlSMElementTimer           { "Timer" };

    constexpr const char* const xmlSMElementMethodList      { XmlSI::xmlSIElementMethodList };
    constexpr const char* const xmlSMElementMethod          { XmlSI::xmlSIElementMethod };
    constexpr const char* const xmlSMElementBody            { "Body" };

    constexpr const char* const xmlSMElementConstantList    { XmlSI::xmlSIElementConstantList };
    constexpr const char* const xmlSMElementConstant        { XmlSI::xmlSIElementConstant };

    constexpr const char* const xmlSMElementIncludeList     { XmlSI::xmlSIElementIncludeList };
    constexpr const char* const xmlSMElementLocation        { XmlSI::xmlSIElementLocation };

    constexpr const char* const xmlSMElementImportList      { "ImportList" };
    constexpr const char* const xmlSMElementMachineImport   { "MachineImport" };

    constexpr const char* const xmlSMElementStateList       { "StateList" };
    constexpr const char* const xmlSMElementState           { "State" };
    constexpr const char* const xmlSMElementEntryList       { "EntryList" };
    constexpr const char* const xmlSMElementExitList        { "ExitList" };
    constexpr const char* const xmlSMElementTransitionList  { "TransitionList" };
    constexpr const char* const xmlSMElementTransition      { "Transition" };
    constexpr const char* const xmlSMElementConditionList   { "ConditionList" };
    constexpr const char* const xmlSMElementConditionGroup  { "ConditionGroup" };
    constexpr const char* const xmlSMElementCondition       { "Condition" };
    constexpr const char* const xmlSMElementOperationList   { "OperationList" };
    constexpr const char* const xmlSMElementExpression      { "Expression" };
    constexpr const char* const xmlSMElementArgumentList    { "ArgumentList" };
    constexpr const char* const xmlSMElementArgument        { "Argument" };

    // Operation element names (mirror the SMOperationBase kind strings)
    constexpr const char* const xmlSMElementActionCall      { "ActionCall" };
    constexpr const char* const xmlSMElementAttributeSet    { "AttributeSet" };
    constexpr const char* const xmlSMElementTimerStart      { "TimerStart" };
    constexpr const char* const xmlSMElementTimerStop       { "TimerStop" };
    constexpr const char* const xmlSMElementEventSend       { "EventSend" };
    constexpr const char* const xmlSMElementInlineCode      { "InlineCode" };

    // Layout element names
    constexpr const char* const xmlSMElementLayout          { "Layout" };
    constexpr const char* const xmlSMElementViewList        { "ViewList" };
    constexpr const char* const xmlSMElementView            { "View" };
    constexpr const char* const xmlSMElementNodeList        { "NodeList" };
    constexpr const char* const xmlSMElementNode            { "Node" };
    constexpr const char* const xmlSMElementEdgeList        { "EdgeList" };
    constexpr const char* const xmlSMElementEdge            { "Edge" };
    constexpr const char* const xmlSMElementPoint           { "Point" };
    constexpr const char* const xmlSMElementLabel           { "Label" };
    constexpr const char* const xmlSMElementNoteList        { "NoteList" };
    constexpr const char* const xmlSMElementNote            { "Note" };
    constexpr const char* const xmlSMElementText            { "Text" };

    //////////////////////////////////////////////////////////////////////////
    // Attribute names
    //////////////////////////////////////////////////////////////////////////

    constexpr const char* const xmlSMAttributeFormatVersion { XmlSI::xmlSIAttributeFormatVersion };
    constexpr const char* const xmlSMAttributeID            { XmlSI::xmlSIAttributeID };
    constexpr const char* const xmlSMAttributeName          { XmlSI::xmlSIAttributeName };
    constexpr const char* const xmlSMAttributeVersion       { XmlSI::xmlSIAttributeVersion };
    constexpr const char* const xmlSMAttributeThreading     { "Threading" };
    constexpr const char* const xmlSMAttributeDataType      { XmlSI::xmlSIAttributeDataType };
    constexpr const char* const xmlSMAttributeValue         { "Value" };
    constexpr const char* const xmlSMAttributeDefault       { "Default" };
    constexpr const char* const xmlSMAttributeTimeout       { "Timeout" };
    constexpr const char* const xmlSMAttributeRepeat        { "Repeat" };
    constexpr const char* const xmlSMAttributeMethodType    { XmlSI::xmlSIAttributeMethodType };
    constexpr const char* const xmlSMAttributeReturn        { "Return" };
    constexpr const char* const xmlSMAttributeImplement     { "Implement" };
    constexpr const char* const xmlSMAttributeIsDeprecated  { XmlSI::xmlSIAttributeIsDeprecated };
    constexpr const char* const xmlSMAttributeLocation      { "Location" };
    constexpr const char* const xmlSMAttributeKind          { "Kind" };
    constexpr const char* const xmlSMAttributeHistory       { "History" };
    constexpr const char* const xmlSMAttributeSubmachine    { "Submachine" };
    constexpr const char* const xmlSMAttributeOnFinal       { "OnFinal" };
    constexpr const char* const xmlSMAttributeStimulusKind  { "StimulusKind" };
    constexpr const char* const xmlSMAttributeStimulus      { "Stimulus" };
    constexpr const char* const xmlSMAttributeTo            { "To" };
    constexpr const char* const xmlSMAttributeCombine       { "Combine" };
    constexpr const char* const xmlSMAttributeLhsKind       { "LHSKind" };
    constexpr const char* const xmlSMAttributeLhs           { "LHS" };
    constexpr const char* const xmlSMAttributeOperator      { "Operator" };
    constexpr const char* const xmlSMAttributeRhsKind       { "RHSKind" };
    constexpr const char* const xmlSMAttributeRhs           { "RHS" };
    constexpr const char* const xmlSMAttributeNegate        { "Negate" };
    constexpr const char* const xmlSMAttributeSource        { "Source" };
    constexpr const char* const xmlSMAttributeAction        { "Action" };
    constexpr const char* const xmlSMAttributeAttribute     { "Attribute" };
    constexpr const char* const xmlSMAttributeTimer         { "Timer" };
    constexpr const char* const xmlSMAttributeEvent         { "Event" };

    // Layout attribute names
    constexpr const char* const xmlSMAttributeOwner         { "Owner" };
    constexpr const char* const xmlSMAttributeZoom          { "Zoom" };
    constexpr const char* const xmlSMAttributeX             { "X" };
    constexpr const char* const xmlSMAttributeY             { "Y" };
    constexpr const char* const xmlSMAttributeWidth         { "Width" };
    constexpr const char* const xmlSMAttributeHeight        { "Height" };
    constexpr const char* const xmlSMAttributeColor         { "Color" };
    constexpr const char* const xmlSMAttributeHeaderColor   { "HeaderColor" };
    constexpr const char* const xmlSMAttributeExpanded      { "Expanded" };
    constexpr const char* const xmlSMAttributeShape         { "Shape" };
    constexpr const char* const xmlSMAttributeBulge         { "Bulge" };
    constexpr const char* const xmlSMAttributeLevel         { "Level" };
    constexpr const char* const xmlSMAttributeGridSize      { "GridSize" };
    constexpr const char* const xmlSMAttributeGridVisible   { "GridVisible" };

    //////////////////////////////////////////////////////////////////////////
    // Shared values
    //////////////////////////////////////////////////////////////////////////

    constexpr const char* const xmlSMValueTrue             { XmlSI::xmlSIValueTrue };
    constexpr const char* const xmlSMValueFalse            { XmlSI::xmlSIValueFalse };
    constexpr const char* const xmlSMShapeArc              { "Arc" };
    constexpr const char* const xmlSMShapeLine             { "Line" };
}

#endif // LUSAN_COMMON_XMLSM_HPP
