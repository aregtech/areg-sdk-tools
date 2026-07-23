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

    constexpr QLatin1StringView xmlSMElementStateMachine    { "StateMachine" };
    constexpr QLatin1StringView xmlSMElementOverview        { XmlSI::xmlSIElementOverview };
    constexpr QLatin1StringView xmlSMElementDescription     { XmlSI::xmlSIElementDescription };
    constexpr QLatin1StringView xmlSMElementDeprecateHint   { XmlSI::xmlSIElementDeprecateHint };

    constexpr QLatin1StringView xmlSMElementDataTypeList    { XmlSI::xmlSIElementDataTypeList };
    constexpr QLatin1StringView xmlSMElementDataType        { XmlSI::xmlSIElementDataType };

    constexpr QLatin1StringView xmlSMElementAttributeList   { XmlSI::xmlSIElementAttributeList };
    constexpr QLatin1StringView xmlSMElementAttribute       { XmlSI::xmlSIElementAttribute };

    constexpr QLatin1StringView xmlSMElementEventList       { "EventList" };
    constexpr QLatin1StringView xmlSMElementEvent           { "Event" };

    constexpr QLatin1StringView xmlSMElementParamList       { XmlSI::xmlSIElementParamList };
    constexpr QLatin1StringView xmlSMElementParameter       { XmlSI::xmlSIElementParameter };

    constexpr QLatin1StringView xmlSMElementTimerList       { "TimerList" };
    constexpr QLatin1StringView xmlSMElementTimer           { "Timer" };

    constexpr QLatin1StringView xmlSMElementMethodList      { XmlSI::xmlSIElementMethodList };
    constexpr QLatin1StringView xmlSMElementMethod          { XmlSI::xmlSIElementMethod };
    constexpr QLatin1StringView xmlSMElementBody            { "Body" };

    constexpr QLatin1StringView xmlSMElementConstantList    { XmlSI::xmlSIElementConstantList };
    constexpr QLatin1StringView xmlSMElementConstant        { XmlSI::xmlSIElementConstant };

    constexpr QLatin1StringView xmlSMElementIncludeList     { XmlSI::xmlSIElementIncludeList };
    constexpr QLatin1StringView xmlSMElementLocation        { XmlSI::xmlSIElementLocation };

    constexpr QLatin1StringView xmlSMElementImportList      { "ImportList" };
    constexpr QLatin1StringView xmlSMElementMachineImport   { "MachineImport" };

    constexpr QLatin1StringView xmlSMElementStateList       { "StateList" };
    constexpr QLatin1StringView xmlSMElementState           { "State" };
    constexpr QLatin1StringView xmlSMElementEntryList       { "EntryList" };
    constexpr QLatin1StringView xmlSMElementExitList        { "ExitList" };
    constexpr QLatin1StringView xmlSMElementDoList          { "DoList" };
    constexpr QLatin1StringView xmlSMElementTransitionList  { "TransitionList" };
    constexpr QLatin1StringView xmlSMElementTransition      { "Transition" };
    constexpr QLatin1StringView xmlSMElementConditionList   { "ConditionList" };
    constexpr QLatin1StringView xmlSMElementConditionGroup  { "ConditionGroup" };
    constexpr QLatin1StringView xmlSMElementCondition       { "Condition" };
    constexpr QLatin1StringView xmlSMElementOperationList   { "OperationList" };

    constexpr QLatin1StringView xmlSMElementGuard           { "Guard" };
    constexpr QLatin1StringView xmlSMElementGuardExpr       { "Expr" };
    constexpr QLatin1StringView xmlSMElementGuardDraft      { "Draft" };
    constexpr QLatin1StringView xmlSMElementGuardRendered   { "Rendered" };
    constexpr QLatin1StringView xmlSMElementGuardAnd        { "And" };
    constexpr QLatin1StringView xmlSMElementGuardOr         { "Or" };
    constexpr QLatin1StringView xmlSMElementGuardNot        { "Not" };
    constexpr QLatin1StringView xmlSMElementGuardCmp        { "Cmp" };
    constexpr QLatin1StringView xmlSMElementGuardCall       { "Call" };
    constexpr QLatin1StringView xmlSMElementGuardArg        { "Arg" };
    constexpr QLatin1StringView xmlSMElementGuardAttr       { "Attr" };
    constexpr QLatin1StringView xmlSMElementGuardConst      { "Const" };
    constexpr QLatin1StringView xmlSMElementGuardParam      { "Param" };
    constexpr QLatin1StringView xmlSMElementGuardLit        { "Lit" };
    constexpr QLatin1StringView xmlSMElementGuardLambda     { "Lambda" };
    constexpr QLatin1StringView xmlSMElementGuardRaw        { "Raw" };

    constexpr QLatin1StringView xmlSMElementExpression      { "Expression" };
    constexpr QLatin1StringView xmlSMElementArgumentList    { "ArgumentList" };
    constexpr QLatin1StringView xmlSMElementArgument        { "Argument" };

    // Operation element names (mirror the SMOperationBase kind strings)
    constexpr QLatin1StringView xmlSMElementActionCall      { "ActionCall" };
    constexpr QLatin1StringView xmlSMElementAttributeSet    { "AttributeSet" };
    constexpr QLatin1StringView xmlSMElementTimerStart      { "TimerStart" };
    constexpr QLatin1StringView xmlSMElementTimerStop       { "TimerStop" };
    constexpr QLatin1StringView xmlSMElementEventSend       { "EventSend" };
    constexpr QLatin1StringView xmlSMElementInlineCode      { "InlineCode" };

    // Layout element names
    constexpr QLatin1StringView xmlSMElementLayout          { "Layout" };
    constexpr QLatin1StringView xmlSMElementViewList        { "ViewList" };
    constexpr QLatin1StringView xmlSMElementView            { "View" };
    constexpr QLatin1StringView xmlSMElementNodeList        { "NodeList" };
    constexpr QLatin1StringView xmlSMElementNode            { "Node" };
    constexpr QLatin1StringView xmlSMElementEdgeList        { "EdgeList" };
    constexpr QLatin1StringView xmlSMElementEdge            { "Edge" };
    constexpr QLatin1StringView xmlSMElementPoint           { "Point" };
    constexpr QLatin1StringView xmlSMElementLabel           { "Label" };
    constexpr QLatin1StringView xmlSMElementNoteList        { "NoteList" };
    constexpr QLatin1StringView xmlSMElementNote            { "Note" };
    constexpr QLatin1StringView xmlSMElementText            { "Text" };

    //////////////////////////////////////////////////////////////////////////
    // Attribute names
    //////////////////////////////////////////////////////////////////////////

    constexpr QLatin1StringView xmlSMAttributeFormatVersion { XmlSI::xmlSIAttributeFormatVersion };
    constexpr QLatin1StringView xmlSMAttributeID            { XmlSI::xmlSIAttributeID };
    constexpr QLatin1StringView xmlSMAttributeName          { XmlSI::xmlSIAttributeName };
    constexpr QLatin1StringView xmlSMAttributeVersion       { XmlSI::xmlSIAttributeVersion };
    constexpr QLatin1StringView xmlSMAttributeThreading     { "Threading" };
    constexpr QLatin1StringView xmlSMAttributeDataType      { XmlSI::xmlSIAttributeDataType };
    constexpr QLatin1StringView xmlSMAttributeValue         { "Value" };
    constexpr QLatin1StringView xmlSMAttributeDefault       { "Default" };
    constexpr QLatin1StringView xmlSMAttributeTimeout       { "Timeout" };
    constexpr QLatin1StringView xmlSMAttributeRepeat        { "Repeat" };
    constexpr QLatin1StringView xmlSMAttributeMethodType    { XmlSI::xmlSIAttributeMethodType };
    constexpr QLatin1StringView xmlSMAttributeReturn        { "Return" };
    constexpr QLatin1StringView xmlSMAttributeImplement     { "Implement" };
    constexpr QLatin1StringView xmlSMAttributeIsDeprecated  { XmlSI::xmlSIAttributeIsDeprecated };
    constexpr QLatin1StringView xmlSMAttributeLocation      { "Location" };
    constexpr QLatin1StringView xmlSMAttributeKind          { "Kind" };
    constexpr QLatin1StringView xmlSMAttributeHistory       { "History" };
    constexpr QLatin1StringView xmlSMAttributeSubmachine    { "Submachine" };
    constexpr QLatin1StringView xmlSMAttributeOnFinal       { "OnFinal" };
    constexpr QLatin1StringView xmlSMAttributeInterval      { "Interval" };
    constexpr QLatin1StringView xmlSMAttributeUntil         { "Until" };
    constexpr QLatin1StringView xmlSMAttributeStimulusKind  { "StimulusKind" };
    constexpr QLatin1StringView xmlSMAttributeStimulus      { "Stimulus" };
    constexpr QLatin1StringView xmlSMAttributeTo            { "To" };
    constexpr QLatin1StringView xmlSMAttributeCombine       { "Combine" };
    constexpr QLatin1StringView xmlSMAttributeGuardState    { "state" };
    constexpr QLatin1StringView xmlSMAttributeGuardRefId    { "id" };
    constexpr QLatin1StringView xmlSMAttributeGuardOp       { "op" };
    constexpr QLatin1StringView xmlSMAttributeGuardBreak    { "brk" };
    constexpr QLatin1StringView xmlSMAttributeGuardIndent   { "indent" };
    constexpr QLatin1StringView xmlSMAttributeGuardName     { "name" };
    constexpr QLatin1StringView xmlSMAttributeLhsKind       { "LHSKind" };
    constexpr QLatin1StringView xmlSMAttributeLhs           { "LHS" };
    constexpr QLatin1StringView xmlSMAttributeOperator      { "Operator" };
    constexpr QLatin1StringView xmlSMAttributeRhsKind       { "RHSKind" };
    constexpr QLatin1StringView xmlSMAttributeRhs           { "RHS" };
    constexpr QLatin1StringView xmlSMAttributeNegate        { "Negate" };
    constexpr QLatin1StringView xmlSMAttributeSource        { "Source" };
    constexpr QLatin1StringView xmlSMAttributeAction        { "Action" };
    constexpr QLatin1StringView xmlSMAttributeAttribute     { "Attribute" };
    constexpr QLatin1StringView xmlSMAttributeTimer         { "Timer" };
    constexpr QLatin1StringView xmlSMAttributeEvent         { "Event" };

    // Layout attribute names
    constexpr QLatin1StringView xmlSMAttributeOwner         { "Owner" };
    constexpr QLatin1StringView xmlSMAttributeZoom          { "Zoom" };
    constexpr QLatin1StringView xmlSMAttributeX             { "X" };
    constexpr QLatin1StringView xmlSMAttributeY             { "Y" };
    constexpr QLatin1StringView xmlSMAttributeWidth         { "Width" };
    constexpr QLatin1StringView xmlSMAttributeHeight        { "Height" };
    constexpr QLatin1StringView xmlSMAttributeColor         { "Color" };
    constexpr QLatin1StringView xmlSMAttributeHeaderColor   { "HeaderColor" };
    constexpr QLatin1StringView xmlSMAttributeExpanded      { "Expanded" };
    constexpr QLatin1StringView xmlSMAttributeShape         { "Shape" };
    constexpr QLatin1StringView xmlSMAttributeBulge         { "Bulge" };
    constexpr QLatin1StringView xmlSMAttributeLevel         { "Level" };
    constexpr QLatin1StringView xmlSMAttributeGridSize      { "GridSize" };
    constexpr QLatin1StringView xmlSMAttributeGridVisible   { "GridVisible" };

    //////////////////////////////////////////////////////////////////////////
    // Shared values
    //////////////////////////////////////////////////////////////////////////

    constexpr QLatin1StringView xmlSMValueTrue             { XmlSI::xmlSIValueTrue };
    constexpr QLatin1StringView xmlSMValueFalse            { XmlSI::xmlSIValueFalse };
    constexpr QLatin1StringView xmlSMShapeArc              { "Arc" };
    constexpr QLatin1StringView xmlSMShapeLine             { "Line" };
}

#endif // LUSAN_COMMON_XMLSM_HPP
