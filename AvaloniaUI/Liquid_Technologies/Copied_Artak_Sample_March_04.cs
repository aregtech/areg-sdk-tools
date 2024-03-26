///////////////////////////////////////////////////////////////////////////
//           Liquid XML Objects GENERATED CODE - DO NOT MODIFY           //
//            https://www.liquid-technologies.com/xml-objects            //
//=======================================================================//
// Dependencies                                                          //
//     Nuget : LiquidTechnologies.XmlObjects.Runtime                     //
//           : MUST BE VERSION 20.7.7                                    //
//=======================================================================//
// Online Help                                                           //
//     https://www.liquid-technologies.com/xml-objects-quick-start-guide //
//=======================================================================//
// Licensing Information                                                 //
//     https://www.liquid-technologies.com/eula                          //
///////////////////////////////////////////////////////////////////////////
using System;
using System.ComponentModel;
using System.Collections.Generic;
using System.Linq;
using System.Xml.Linq;
using System.Numerics;
using LiquidTechnologies.XmlObjects;
using LiquidTechnologies.XmlObjects.Attribution;

// ------------------------------------------------------
// |                      Settings                      |
// ------------------------------------------------------
// GenerateCommonBaseClass                  = False
// GenerateUnprocessedNodeHandlers          = False
// RaiseChangeEvents                        = False
// CollectionNaming                         = Pluralize
// Language                                 = CS
// OutputNamespace                          = Liquid_Technologies
// WriteDefaultValuesForOptionalAttributes  = True
// WriteDefaultValuesForOptionalElements    = False
// MixedContentHandling                     = TreatAsAny
// GenerationModel                          = ConformantWithAccessors
// XSD Schema Files
//    E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd


namespace Liquid_Technologies
{
    #region Global Settings
    /// <summary>Contains library level properties, and ensures the version of the runtime used matches the version used to generate it.</summary>
    [LxRuntimeRequirements("20.7.7.12879", "Trial for Non-Commercial Use Expiry [2024-04-09]", "CBMLU2TKYQQG1HAP", LiquidTechnologies.XmlObjects.LicenseTermsType.Trial)]
    public partial class LxRuntimeRequirementsWritten
    {
    }

    #endregion

}

namespace Liquid_Technologies.Ns
{
    #region Enumerations
    /// <summary>An enumeration representing XSD simple type ValidObjects</summary>
    /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/simpleType:ValidObjects</XsdPath>
    /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
    /// <XsdLocation>41:2-48:18</XsdLocation>
    [LxSimpleTypeDefinition("ValidObjects", "")]
    public enum ValidObjectsEnum
    {
        /// <summary>Represents the value 'Enumerate' in the XML</summary>
        [LxEnumValue("Enumerate")]
        Enumerate,
        /// <summary>Represents the value 'Structure' in the XML</summary>
        [LxEnumValue("Structure")]
        Structure,
        /// <summary>Represents the value 'Imported' in the XML</summary>
        [LxEnumValue("Imported")]
        Imported,
        /// <summary>Represents the value 'DefinedType' in the XML</summary>
        [LxEnumValue("DefinedType")]
        DefinedType,
    }
    /// <summary>An enumeration representing XSD simple type ValidTypes</summary>
    /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/simpleType:ValidTypes</XsdPath>
    /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
    /// <XsdLocation>12:2-35:18</XsdLocation>
    [LxSimpleTypeDefinition("ValidTypes", "")]
    public enum ValidTypesEnum
    {
        /// <summary>Represents the value 'bool' in the XML</summary>
        [LxEnumValue("bool")]
        Bool_,
        /// <summary>Represents the value 'char' in the XML</summary>
        [LxEnumValue("char")]
        Char_,
        /// <summary>Represents the value 'int8' in the XML</summary>
        [LxEnumValue("int8")]
        Int8,
        /// <summary>Represents the value 'uint8' in the XML</summary>
        [LxEnumValue("uint8")]
        Uint8,
        /// <summary>Represents the value 'int16' in the XML</summary>
        [LxEnumValue("int16")]
        Int16,
        /// <summary>Represents the value 'uint16' in the XML</summary>
        [LxEnumValue("uint16")]
        Uint16,
        /// <summary>Represents the value 'int32' in the XML</summary>
        [LxEnumValue("int32")]
        Int32,
        /// <summary>Represents the value 'uint32' in the XML</summary>
        [LxEnumValue("uint32")]
        Uint32,
        /// <summary>Represents the value 'int64' in the XML</summary>
        [LxEnumValue("int64")]
        Int64,
        /// <summary>Represents the value 'unt64' in the XML</summary>
        [LxEnumValue("unt64")]
        Unt64,
        /// <summary>Represents the value 'type_id' in the XML</summary>
        [LxEnumValue("type_id")]
        Type_Id,
        /// <summary>Represents the value 'float32' in the XML</summary>
        [LxEnumValue("float32")]
        Float32,
        /// <summary>Represents the value 'float64' in the XML</summary>
        [LxEnumValue("float64")]
        Float64,
        /// <summary>Represents the value 'String' in the XML</summary>
        [LxEnumValue("String")]
        String_,
        /// <summary>Represents the value 'BinaryBuffer' in the XML</summary>
        [LxEnumValue("BinaryBuffer")]
        BinaryBuffer,
        /// <summary>Represents the value 'DateTime' in the XML</summary>
        [LxEnumValue("DateTime")]
        DateTime,
        /// <summary>Represents the value 'SomeStruct' in the XML</summary>
        [LxEnumValue("SomeStruct")]
        SomeStruct,
        /// <summary>Represents the value 'SomeEnum' in the XML</summary>
        [LxEnumValue("SomeEnum")]
        SomeEnum,
        /// <summary>Represents the value 'SomeArray' in the XML</summary>
        [LxEnumValue("SomeArray")]
        SomeArray,
        /// <summary>Represents the value 'NEMemory::uAlign' in the XML</summary>
        [LxEnumValue("NEMemory::uAlign")]
        NEMemory_UAlign,
    }
    #endregion

    #region Unions
    /// <summary>A union representing the XSD simple type ExtendedValidType</summary>
    /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/simpleType:ExtendedValidType</XsdPath>
    /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
    /// <XsdLocation>37:2-39:18</XsdLocation>
    public partial class ExtendedValidTypeUnion : Union<Liquid_Technologies.Ns.ValidTypesEnum, System.String>
    {
        /// <summary>Constructor : create a union from a <see cref="Liquid_Technologies.Ns.ValidTypesEnum" /></summary>
        [LxUnion(LxValueType.Enum, XsdType.Enum)]
        public ExtendedValidTypeUnion(Liquid_Technologies.Ns.ValidTypesEnum value)  : base(value) { }

        /// <summary>Constructor : create a union from a <see cref="System.String" /></summary>
        [LxUnion(LxValueType.Value, XsdType.XsdString, Pattern = "[A-Za-z][A-Za-z_0-9]+")]
        [LxUnion(LxValueType.Value, XsdType.XsdAnyURI)]
        public ExtendedValidTypeUnion(System.String value)  : base(value) { }

    }

    #endregion

    #region Elements
    /// <summary>A class representing the root XSD element ServiceInterface</summary>
    /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface</XsdPath>
    /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
    /// <XsdLocation>49:2-254:15</XsdLocation>
    [LxElementDefinition("ServiceInterface", "", ElementScopeType.GlobalElement)]
    public partial class ServiceInterfaceElm
    {
        /// <summary>The value for the attribute FormatVersion</summary>
        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/attribute:FormatVersion</XsdPath>
        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
        /// <XsdLocation>252:4-252:72</XsdLocation>
        [LxAttribute("FormatVersion", "", LxValueType.Value, XsdType.XsdString, Required = true)]
        public System.String FormatVersion { get; set; } = "";

        /// <summary>A class representing an xs:sequence.</summary>
        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence</XsdPath>
        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
        /// <XsdLocation>51:4-251:18</XsdLocation>
        [LxElementCompositor(1)]
        public Liquid_Technologies.Ns.ServiceInterfaceElm.ServiceInterfaceSeq Seq { get; set; } = new Liquid_Technologies.Ns.ServiceInterfaceElm.ServiceInterfaceSeq();

        #region Simplified read only accessors
        /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.ServiceInterfaceSeq.Overview" />
        public Liquid_Technologies.Ns.ServiceInterfaceElm.OverviewElm Overview { get { return this.Seq.Overview; } }

        /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.ServiceInterfaceSeq.DataTypeList" />
        public Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm DataTypeList { get { return this.Seq.DataTypeList; } }

        /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.ServiceInterfaceSeq.AttributeList" />
        public Liquid_Technologies.Ns.ServiceInterfaceElm.AttributeListElm AttributeList { get { return this.Seq.AttributeList; } }

        /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.ServiceInterfaceSeq.MethodList" />
        public Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm MethodList { get { return this.Seq.MethodList; } }

        /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.ServiceInterfaceSeq.ConstantList" />
        public Liquid_Technologies.Ns.ServiceInterfaceElm.ConstantListElm ConstantList { get { return this.Seq.ConstantList; } }

        /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.ServiceInterfaceSeq.IncludeList" />
        public Liquid_Technologies.Ns.ServiceInterfaceElm.IncludeListElm IncludeList { get { return this.Seq.IncludeList; } }

        #endregion

        /// <summary>A class representing an xs:sequence.</summary>
        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence</XsdPath>
        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
        /// <XsdLocation>51:4-251:18</XsdLocation>
        [LxCompositorDefinition(CompositorType.Sequence)]
        public partial class ServiceInterfaceSeq
        {
            /// <summary>A <see cref="Liquid_Technologies.Ns.ServiceInterfaceElm.OverviewElm" />, Required : should not be set to null</summary>
            [LxElementRef(0)]
            public Liquid_Technologies.Ns.ServiceInterfaceElm.OverviewElm Overview { get; set; } = new Liquid_Technologies.Ns.ServiceInterfaceElm.OverviewElm();

            /// <summary>A <see cref="Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm" />, Required : should not be set to null</summary>
            [LxElementRef(1)]
            public Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm DataTypeList { get; set; } = new Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm();

            /// <summary>A <see cref="Liquid_Technologies.Ns.ServiceInterfaceElm.AttributeListElm" />, Required : should not be set to null</summary>
            [LxElementRef(2)]
            public Liquid_Technologies.Ns.ServiceInterfaceElm.AttributeListElm AttributeList { get; set; } = new Liquid_Technologies.Ns.ServiceInterfaceElm.AttributeListElm();

            /// <summary>A <see cref="Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm" />, Required : should not be set to null</summary>
            [LxElementRef(3)]
            public Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm MethodList { get; set; } = new Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm();

            /// <summary>A <see cref="Liquid_Technologies.Ns.ServiceInterfaceElm.ConstantListElm" />, Required : should not be set to null</summary>
            [LxElementRef(4)]
            public Liquid_Technologies.Ns.ServiceInterfaceElm.ConstantListElm ConstantList { get; set; } = new Liquid_Technologies.Ns.ServiceInterfaceElm.ConstantListElm();

            /// <summary>A <see cref="Liquid_Technologies.Ns.ServiceInterfaceElm.IncludeListElm" />, Required : should not be set to null</summary>
            [LxElementRef(5)]
            public Liquid_Technologies.Ns.ServiceInterfaceElm.IncludeListElm IncludeList { get; set; } = new Liquid_Technologies.Ns.ServiceInterfaceElm.IncludeListElm();

        }

        /// <summary>Represent the inline xs:element Overview.</summary>
        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:Overview</XsdPath>
        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
        /// <XsdLocation>52:5-64:18</XsdLocation>
        [LxElementDefinition("Overview", "", ElementScopeType.InlineElement)]
        public partial class OverviewElm
        {
            /// <summary>The value for the attribute ID</summary>
            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:Overview/attribute:ID</XsdPath>
            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
            /// <XsdLocation>58:7-58:65</XsdLocation>
            [LxAttribute("ID", "", LxValueType.Value, XsdType.XsdInteger, Required = true)]
            public System.Numerics.BigInteger ID { get; set; }

            /// <summary>The value for the attribute Name</summary>
            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:Overview/attribute:Name</XsdPath>
            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
            /// <XsdLocation>59:7-59:67</XsdLocation>
            [LxAttribute("Name", "", LxValueType.Value, XsdType.XsdString, Required = true, Pattern = "[A-Za-z][A-Za-z_0-9]+")]
            public System.String Name { get; set; } = "";

            /// <summary>The value for the attribute Version</summary>
            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:Overview/attribute:Version</XsdPath>
            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
            /// <XsdLocation>60:7-60:69</XsdLocation>
            [LxAttribute("Version", "", LxValueType.Value, XsdType.XsdString, Required = true)]
            public System.String Version { get; set; } = "";

            /// <summary>The value for the attribute isRemote</summary>
            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:Overview/attribute:isRemote</XsdPath>
            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
            /// <XsdLocation>61:7-61:71</XsdLocation>
            [LxAttribute("isRemote", "", LxValueType.Value, XsdType.XsdBoolean, Required = true)]
            public System.Boolean IsRemote { get; set; }

            /// <summary>The value for the optional attribute IsDeprecated</summary>
            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:Overview/attribute:IsDeprecated</XsdPath>
            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
            /// <XsdLocation>62:7-62:75</XsdLocation>
            [LxAttribute("IsDeprecated", "", LxValueType.Value, XsdType.XsdBoolean)]
            public System.Boolean? IsDeprecated { get; set; }

            /// <summary>A class representing an xs:sequence.</summary>
            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:Overview/sequence</XsdPath>
            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
            /// <XsdLocation>54:7-57:21</XsdLocation>
            [LxElementCompositor(5)]
            public Liquid_Technologies.Ns.ServiceInterfaceElm.OverviewElm.OverviewSeq Seq { get; set; } = new Liquid_Technologies.Ns.ServiceInterfaceElm.OverviewElm.OverviewSeq();

            #region Simplified read only accessors
            /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.OverviewElm.OverviewSeq.Description" />
            public System.String Description { get { return this.Seq.Description; } }

            /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.OverviewElm.OverviewSeq.DeprecateHint" />
            public System.String DeprecateHint { get { return this.Seq.DeprecateHint; } }

            #endregion

            /// <summary>A class representing an xs:sequence.</summary>
            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:Overview/sequence</XsdPath>
            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
            /// <XsdLocation>54:7-57:21</XsdLocation>
            [LxCompositorDefinition(CompositorType.Sequence)]
            public partial class OverviewSeq
            {
                /// <summary>A <see cref="System.String" />, Optional : null when not set</summary>
                [LxElementValue(0, "Description", "", LxValueType.Value, XsdType.XsdString, MinOccurs = 0)]
                public System.String Description { get; set; }

                /// <summary>A <see cref="System.String" />, Optional : null when not set</summary>
                [LxElementValue(1, "DeprecateHint", "", LxValueType.Value, XsdType.XsdString, MinOccurs = 0)]
                public System.String DeprecateHint { get; set; }

            }

        }

        /// <summary>Represent the inline xs:element DataTypeList.</summary>
        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList</XsdPath>
        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
        /// <XsdLocation>65:5-137:18</XsdLocation>
        [LxElementDefinition("DataTypeList", "", ElementScopeType.InlineElement)]
        public partial class DataTypeListElm
        {
            /// <summary>A class representing an xs:sequence.</summary>
            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence</XsdPath>
            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
            /// <XsdLocation>67:7-135:21</XsdLocation>
            [LxElementCompositor(0)]
            public Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeListSeq Seq { get; set; } = new Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeListSeq();

            #region Simplified read only accessors
            /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeListSeq.DataTypes" />
            public IEnumerable<Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm> DataTypes { get { return this.Seq.DataTypes; } }

            #endregion

            /// <summary>A class representing an xs:sequence.</summary>
            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence</XsdPath>
            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
            /// <XsdLocation>67:7-135:21</XsdLocation>
            [LxCompositorDefinition(CompositorType.Sequence)]
            public partial class DataTypeListSeq
            {
                /// <summary>A collection of <see cref="Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm" /></summary>
                [LxElementRef(0, MaxOccurs = LxConstants.Unbounded)]
                public List<Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm> DataTypes { get; } = new List<Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm>();

            }

            /// <summary>Represent the inline xs:element DataType.</summary>
            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType</XsdPath>
            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
            /// <XsdLocation>68:8-134:21</XsdLocation>
            [LxElementDefinition("DataType", "", ElementScopeType.InlineElement)]
            public partial class DataTypeElm
            {
                /// <summary>The value for the attribute ID</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/attribute:ID</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>128:10-128:68</XsdLocation>
                [LxAttribute("ID", "", LxValueType.Value, XsdType.XsdInteger, Required = true)]
                public System.Numerics.BigInteger ID { get; set; }

                /// <summary>The value for the attribute Name</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/attribute:Name</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>129:10-129:70</XsdLocation>
                [LxAttribute("Name", "", LxValueType.Value, XsdType.XsdString, Required = true, Pattern = "[A-Za-z][A-Za-z_0-9]+")]
                public System.String Name { get; set; } = "";

                /// <summary>The value for the attribute Type</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/attribute:Type</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>130:10-130:72</XsdLocation>
                [LxAttribute("Type", "", LxValueType.Enum, XsdType.Enum, Required = true, WhiteSpace = WhiteSpaceType.Preserve)]
                public Liquid_Technologies.Ns.ValidObjectsEnum Type { get; set; }

                /// <summary>The value for the optional attribute Values</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/attribute:Values</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>131:10-131:71</XsdLocation>
                [LxAttribute("Values", "", LxValueType.Value, XsdType.XsdString)]
                public System.String Values { get; set; }

                /// <summary>The value for the optional attribute IsDeprecated</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/attribute:IsDeprecated</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>132:10-132:78</XsdLocation>
                [LxAttribute("IsDeprecated", "", LxValueType.Value, XsdType.XsdBoolean)]
                public System.Boolean? IsDeprecated { get; set; }

                /// <summary>A class representing an xs:sequence.</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/sequence</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>70:10-127:24</XsdLocation>
                [LxElementCompositor(5)]
                public Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.DataTypeSeq Seq { get; set; } = new Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.DataTypeSeq();

                #region Simplified read only accessors
                /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.DataTypeSeq.Description" />
                public System.String Description { get { return this.Seq.Description; } }

                /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.DataTypeSeq.Container" />
                public Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.ContainerEnum? Container { get { return this.Seq.Container; } }

                /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.DataTypeSeq.BaseTypeValue" />
                public Liquid_Technologies.Ns.ExtendedValidTypeUnion BaseTypeValue { get { return this.Seq.BaseTypeValue; } }

                /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.DataTypeSeq.BaseTypeKey" />
                public Liquid_Technologies.Ns.ValidTypesEnum? BaseTypeKey { get { return this.Seq.BaseTypeKey; } }

                /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.DataTypeSeq.Namespace_" />
                public System.String Namespace_ { get { return this.Seq.Namespace_; } }

                /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.DataTypeSeq.Location" />
                public System.String Location { get { return this.Seq.Location; } }

                /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.DataTypeSeq.DeprecateHint" />
                public System.String DeprecateHint { get { return this.Seq.DeprecateHint; } }

                /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.DataTypeSeq.FieldLists" />
                public IEnumerable<Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm> FieldLists { get { return this.Seq.FieldLists; } }

                #endregion

                /// <summary>An enumeration representing XSD simple type Container</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/sequence/element:Container</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>72:11-83:24</XsdLocation>
                [LxSimpleTypeDefinition("Container", "")]
                public enum ContainerEnum
                {
                    /// <summary>Represents the value 'Array' in the XML</summary>
                    [LxEnumValue("Array")]
                    Array,
                    /// <summary>Represents the value 'LinkedList' in the XML</summary>
                    [LxEnumValue("LinkedList")]
                    LinkedList,
                    /// <summary>Represents the value 'HashMap' in the XML</summary>
                    [LxEnumValue("HashMap")]
                    HashMap,
                    /// <summary>Represents the value 'Pair' in the XML</summary>
                    [LxEnumValue("Pair")]
                    Pair,
                    /// <summary>Represents the value 'CustomContainer' in the XML</summary>
                    [LxEnumValue("CustomContainer")]
                    CustomContainer,
                    /// <summary>Represents the value 'New Type' in the XML</summary>
                    [LxEnumValue("New Type")]
                    New_Type,
                }
                /// <summary>A class representing an xs:sequence.</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/sequence</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>70:10-127:24</XsdLocation>
                [LxCompositorDefinition(CompositorType.Sequence)]
                public partial class DataTypeSeq
                {
                    /// <summary>A <see cref="System.String" />, Optional : null when not set</summary>
                    [LxElementValue(0, "Description", "", LxValueType.Value, XsdType.XsdString, MinOccurs = 0)]
                    public System.String Description { get; set; }

                    /// <summary>A nullable <see cref="Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.ContainerEnum" />, Optional : null when not set, Default : 'Array'</summary>
                    [LxElementValue(1, "Container", "", LxValueType.Enum, XsdType.Enum, MinOccurs = 0, Default = "Array", WhiteSpace = WhiteSpaceType.Preserve)]
                    public Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.ContainerEnum? Container { get; set; }

                    /// <summary>A <see cref="Liquid_Technologies.Ns.ExtendedValidTypeUnion" />, Optional : null when not set</summary>
                    [LxElementValue(2, "BaseTypeValue", "", LxValueType.Union, XsdType.Union, MinOccurs = 0)]
                    public Liquid_Technologies.Ns.ExtendedValidTypeUnion BaseTypeValue { get; set; }

                    /// <summary>A nullable <see cref="Liquid_Technologies.Ns.ValidTypesEnum" />, Optional : null when not set</summary>
                    [LxElementValue(3, "BaseTypeKey", "", LxValueType.Enum, XsdType.Enum, MinOccurs = 0, WhiteSpace = WhiteSpaceType.Preserve)]
                    public Liquid_Technologies.Ns.ValidTypesEnum? BaseTypeKey { get; set; }

                    /// <summary>A <see cref="System.String" />, Optional : null when not set</summary>
                    [LxElementValue(4, "Namespace", "", LxValueType.Value, XsdType.XsdString, MinOccurs = 0, Pattern = "[A-Za-z][A-Za-z_0-9]+")]
                    public System.String Namespace_ { get; set; }

                    /// <summary>A <see cref="System.String" />, Optional : null when not set</summary>
                    [LxElementValue(5, "Location", "", LxValueType.Value, XsdType.XsdString, MinOccurs = 0)]
                    public System.String Location { get; set; }

                    /// <summary>A <see cref="System.String" />, Optional : null when not set</summary>
                    [LxElementValue(6, "DeprecateHint", "", LxValueType.Value, XsdType.XsdString, MinOccurs = 0)]
                    public System.String DeprecateHint { get; set; }

                    /// <summary>A collection of <see cref="Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm" /></summary>
                    [LxElementRef(7, MinOccurs = 0, MaxOccurs = LxConstants.Unbounded)]
                    public List<Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm> FieldLists { get; } = new List<Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm>();

                }

                /// <summary>Represent the inline xs:element FieldList.</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/sequence/element:FieldList</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>89:11-126:24</XsdLocation>
                [LxElementDefinition("FieldList", "", ElementScopeType.InlineElement)]
                public partial class FieldListElm
                {
                    /// <summary>A class representing an xs:sequence.</summary>
                    /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/sequence/element:FieldList/sequence</XsdPath>
                    /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                    /// <XsdLocation>91:13-124:27</XsdLocation>
                    [LxElementCompositor(0)]
                    public Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.FieldListSeq Seq { get; set; } = new Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.FieldListSeq();

                    #region Simplified read only accessors
                    /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.FieldListSeq.EnumEntries" />
                    public IEnumerable<Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.EnumEntryElm> EnumEntries { get { return this.Seq.EnumEntries; } }

                    /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.FieldListSeq.Fields" />
                    public IEnumerable<Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.FieldElm> Fields { get { return this.Seq.Fields; } }

                    #endregion

                    /// <summary>A class representing an xs:sequence.</summary>
                    /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/sequence/element:FieldList/sequence</XsdPath>
                    /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                    /// <XsdLocation>91:13-124:27</XsdLocation>
                    [LxCompositorDefinition(CompositorType.Sequence)]
                    public partial class FieldListSeq
                    {
                        /// <summary>A collection of <see cref="Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.EnumEntryElm" /></summary>
                        [LxElementRef(0, MinOccurs = 0, MaxOccurs = LxConstants.Unbounded)]
                        public List<Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.EnumEntryElm> EnumEntries { get; } = new List<Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.EnumEntryElm>();

                        /// <summary>A collection of <see cref="Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.FieldElm" /></summary>
                        [LxElementRef(1, MinOccurs = 0, MaxOccurs = LxConstants.Unbounded)]
                        public List<Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.FieldElm> Fields { get; } = new List<Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.FieldElm>();

                    }

                    /// <summary>Represent the inline xs:element EnumEntry.</summary>
                    /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/sequence/element:FieldList/sequence/element:EnumEntry</XsdPath>
                    /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                    /// <XsdLocation>92:14-102:27</XsdLocation>
                    [LxElementDefinition("EnumEntry", "", ElementScopeType.InlineElement)]
                    public partial class EnumEntryElm
                    {
                        /// <summary>The value for the attribute ID</summary>
                        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/sequence/element:FieldList/sequence/element:EnumEntry/attribute:ID</XsdPath>
                        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                        /// <XsdLocation>98:16-98:74</XsdLocation>
                        [LxAttribute("ID", "", LxValueType.Value, XsdType.XsdInteger, Required = true)]
                        public System.Numerics.BigInteger ID { get; set; }

                        /// <summary>The value for the attribute Name</summary>
                        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/sequence/element:FieldList/sequence/element:EnumEntry/attribute:Name</XsdPath>
                        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                        /// <XsdLocation>99:16-99:76</XsdLocation>
                        [LxAttribute("Name", "", LxValueType.Value, XsdType.XsdString, Required = true, Pattern = "[A-Za-z][A-Za-z_0-9]+")]
                        public System.String Name { get; set; } = "";

                        /// <summary>The value for the optional attribute IsDeprecated</summary>
                        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/sequence/element:FieldList/sequence/element:EnumEntry/attribute:IsDeprecated</XsdPath>
                        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                        /// <XsdLocation>100:16-100:84</XsdLocation>
                        [LxAttribute("IsDeprecated", "", LxValueType.Value, XsdType.XsdBoolean)]
                        public System.Boolean? IsDeprecated { get; set; }

                        /// <summary>A class representing an xs:sequence.</summary>
                        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/sequence/element:FieldList/sequence/element:EnumEntry/sequence</XsdPath>
                        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                        /// <XsdLocation>94:16-97:30</XsdLocation>
                        [LxElementCompositor(3)]
                        public Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.EnumEntryElm.EnumEntrySeq Seq { get; set; } = new Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.EnumEntryElm.EnumEntrySeq();

                        #region Simplified read only accessors
                        /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.EnumEntryElm.EnumEntrySeq.Value1" />
                        public System.String Value1 { get { return this.Seq.Value1; } }

                        /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.EnumEntryElm.EnumEntrySeq.DeprecateHint" />
                        public System.String DeprecateHint { get { return this.Seq.DeprecateHint; } }

                        #endregion

                        /// <summary>A class representing an xs:sequence.</summary>
                        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/sequence/element:FieldList/sequence/element:EnumEntry/sequence</XsdPath>
                        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                        /// <XsdLocation>94:16-97:30</XsdLocation>
                        [LxCompositorDefinition(CompositorType.Sequence)]
                        public partial class EnumEntrySeq
                        {
                            /// <summary>A <see cref="System.String" />, Required : should not be set to null</summary>
                            [LxElementValue(0, "Value", "", LxValueType.Value, XsdType.XsdString)]
                            public System.String Value1 { get; set; } = "";

                            /// <summary>A <see cref="System.String" />, Optional : null when not set</summary>
                            [LxElementValue(1, "DeprecateHint", "", LxValueType.Value, XsdType.XsdString, MinOccurs = 0)]
                            public System.String DeprecateHint { get; set; }

                        }

                    }

                    /// <summary>Represent the inline xs:element Field.</summary>
                    /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/sequence/element:FieldList/sequence/element:Field</XsdPath>
                    /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                    /// <XsdLocation>103:14-123:27</XsdLocation>
                    [LxElementDefinition("Field", "", ElementScopeType.InlineElement)]
                    public partial class FieldElm
                    {
                        /// <summary>The value for the attribute DataType</summary>
                        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/sequence/element:FieldList/sequence/element:Field/attribute:DataType</XsdPath>
                        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                        /// <XsdLocation>118:16-118:87</XsdLocation>
                        [LxAttribute("DataType", "", LxValueType.Union, XsdType.Union, Required = true)]
                        public Liquid_Technologies.Ns.ExtendedValidTypeUnion DataType { get; set; }

                        /// <summary>The value for the attribute ID</summary>
                        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/sequence/element:FieldList/sequence/element:Field/attribute:ID</XsdPath>
                        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                        /// <XsdLocation>119:16-119:74</XsdLocation>
                        [LxAttribute("ID", "", LxValueType.Value, XsdType.XsdInteger, Required = true)]
                        public System.Numerics.BigInteger ID { get; set; }

                        /// <summary>The value for the attribute Name</summary>
                        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/sequence/element:FieldList/sequence/element:Field/attribute:Name</XsdPath>
                        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                        /// <XsdLocation>120:16-120:76</XsdLocation>
                        [LxAttribute("Name", "", LxValueType.Value, XsdType.XsdString, Required = true, Pattern = "[A-Za-z][A-Za-z_0-9]+")]
                        public System.String Name { get; set; } = "";

                        /// <summary>The value for the optional attribute IsDeprecated</summary>
                        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/sequence/element:FieldList/sequence/element:Field/attribute:IsDeprecated</XsdPath>
                        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                        /// <XsdLocation>121:16-121:84</XsdLocation>
                        [LxAttribute("IsDeprecated", "", LxValueType.Value, XsdType.XsdBoolean)]
                        public System.Boolean? IsDeprecated { get; set; }

                        /// <summary>A class representing an xs:sequence.</summary>
                        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/sequence/element:FieldList/sequence/element:Field/sequence</XsdPath>
                        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                        /// <XsdLocation>105:16-117:30</XsdLocation>
                        [LxElementCompositor(4)]
                        public Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.FieldElm.FieldSeq Seq { get; set; } = new Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.FieldElm.FieldSeq();

                        #region Simplified read only accessors
                        /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.FieldElm.FieldSeq.Value1" />
                        public Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.FieldElm.ValueElm Value1 { get { return this.Seq.Value1; } }

                        /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.FieldElm.FieldSeq.Description" />
                        public System.String Description { get { return this.Seq.Description; } }

                        /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.FieldElm.FieldSeq.DeprecateHint" />
                        public System.String DeprecateHint { get { return this.Seq.DeprecateHint; } }

                        #endregion

                        /// <summary>A class representing an xs:sequence.</summary>
                        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/sequence/element:FieldList/sequence/element:Field/sequence</XsdPath>
                        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                        /// <XsdLocation>105:16-117:30</XsdLocation>
                        [LxCompositorDefinition(CompositorType.Sequence)]
                        public partial class FieldSeq
                        {
                            /// <summary>A <see cref="Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.FieldElm.ValueElm" />, Required : should not be set to null</summary>
                            [LxElementRef(0)]
                            public Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.FieldElm.ValueElm Value1 { get; set; } = new Liquid_Technologies.Ns.ServiceInterfaceElm.DataTypeListElm.DataTypeElm.FieldListElm.FieldElm.ValueElm();

                            /// <summary>A <see cref="System.String" />, Optional : null when not set</summary>
                            [LxElementValue(1, "Description", "", LxValueType.Value, XsdType.XsdString, MinOccurs = 0)]
                            public System.String Description { get; set; }

                            /// <summary>A <see cref="System.String" />, Optional : null when not set</summary>
                            [LxElementValue(2, "DeprecateHint", "", LxValueType.Value, XsdType.XsdString, MinOccurs = 0)]
                            public System.String DeprecateHint { get; set; }

                        }

                        /// <summary>Represent the inline xs:element Value.</summary>
                        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/sequence/element:FieldList/sequence/element:Field/sequence/element:Value</XsdPath>
                        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                        /// <XsdLocation>106:17-114:30</XsdLocation>
                        [LxElementDefinition("Value", "", ElementScopeType.InlineElement)]
                        public partial class ValueElm
                        {
                            /// <summary>The value for the optional attribute IsDefault</summary>
                            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/sequence/element:FieldList/sequence/element:Field/sequence/element:Value/attribute:IsDefault</XsdPath>
                            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                            /// <XsdLocation>110:21-110:86</XsdLocation>
                            [LxAttribute("IsDefault", "", LxValueType.Value, XsdType.XsdBoolean)]
                            public System.Boolean? IsDefault { get; set; }

                            /// <summary>Holds the <see cref="System.String" /> (xs:http://www.w3.org/2001/XMLSchema:string) value of the element</summary>
                            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:DataTypeList/sequence/element:DataType/sequence/element:FieldList/sequence/element:Field/sequence/element:Value</XsdPath>
                            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                            /// <XsdLocation>106:17-114:30</XsdLocation>
                            [LxValue(LxValueType.Value, XsdType.XsdString)]
                            public System.String Value { get; set; } = "";

                        }

                    }

                }

            }

        }

        /// <summary>Represent the inline xs:element AttributeList.</summary>
        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:AttributeList</XsdPath>
        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
        /// <XsdLocation>138:5-162:18</XsdLocation>
        [LxElementDefinition("AttributeList", "", ElementScopeType.InlineElement)]
        public partial class AttributeListElm
        {
            /// <summary>A class representing an xs:sequence.</summary>
            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:AttributeList/sequence</XsdPath>
            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
            /// <XsdLocation>140:7-160:21</XsdLocation>
            [LxElementCompositor(0)]
            public Liquid_Technologies.Ns.ServiceInterfaceElm.AttributeListElm.AttributeListSeq Seq { get; set; } = new Liquid_Technologies.Ns.ServiceInterfaceElm.AttributeListElm.AttributeListSeq();

            #region Simplified read only accessors
            /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.AttributeListElm.AttributeListSeq.Attributes" />
            public IEnumerable<Liquid_Technologies.Ns.ServiceInterfaceElm.AttributeListElm.AttributeElm> Attributes { get { return this.Seq.Attributes; } }

            #endregion

            /// <summary>A class representing an xs:sequence.</summary>
            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:AttributeList/sequence</XsdPath>
            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
            /// <XsdLocation>140:7-160:21</XsdLocation>
            [LxCompositorDefinition(CompositorType.Sequence)]
            public partial class AttributeListSeq
            {
                /// <summary>A collection of <see cref="Liquid_Technologies.Ns.ServiceInterfaceElm.AttributeListElm.AttributeElm" /></summary>
                [LxElementRef(0, MaxOccurs = LxConstants.Unbounded)]
                public List<Liquid_Technologies.Ns.ServiceInterfaceElm.AttributeListElm.AttributeElm> Attributes { get; } = new List<Liquid_Technologies.Ns.ServiceInterfaceElm.AttributeListElm.AttributeElm>();

            }

            /// <summary>Represent the inline xs:element Attribute.</summary>
            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:AttributeList/sequence/element:Attribute</XsdPath>
            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
            /// <XsdLocation>141:8-159:21</XsdLocation>
            [LxElementDefinition("Attribute", "", ElementScopeType.InlineElement)]
            public partial class AttributeElm
            {
                /// <summary>The value for the attribute DataType</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:AttributeList/sequence/element:Attribute/attribute:DataType</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>147:10-147:81</XsdLocation>
                [LxAttribute("DataType", "", LxValueType.Union, XsdType.Union, Required = true)]
                public Liquid_Technologies.Ns.ExtendedValidTypeUnion DataType { get; set; }

                /// <summary>The value for the attribute ID</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:AttributeList/sequence/element:Attribute/attribute:ID</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>148:10-148:68</XsdLocation>
                [LxAttribute("ID", "", LxValueType.Value, XsdType.XsdInteger, Required = true)]
                public System.Numerics.BigInteger ID { get; set; }

                /// <summary>The value for the attribute Name</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:AttributeList/sequence/element:Attribute/attribute:Name</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>149:10-149:70</XsdLocation>
                [LxAttribute("Name", "", LxValueType.Value, XsdType.XsdString, Required = true, Pattern = "[A-Za-z][A-Za-z_0-9]+")]
                public System.String Name { get; set; } = "";

                /// <summary>The value for the attribute Notify</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:AttributeList/sequence/element:Attribute/attribute:Notify</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>150:10-156:25</XsdLocation>
                [LxAttribute("Notify", "", LxValueType.Value, XsdType.XsdString, Required = true, Pattern = "Always|OnChange")]
                public System.String Notify { get; set; } = "";

                /// <summary>The value for the optional attribute IsDeprecated</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:AttributeList/sequence/element:Attribute/attribute:IsDeprecated</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>157:10-157:78</XsdLocation>
                [LxAttribute("IsDeprecated", "", LxValueType.Value, XsdType.XsdBoolean)]
                public System.Boolean? IsDeprecated { get; set; }

                /// <summary>A class representing an xs:sequence.</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:AttributeList/sequence/element:Attribute/sequence</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>143:10-146:24</XsdLocation>
                [LxElementCompositor(5)]
                public Liquid_Technologies.Ns.ServiceInterfaceElm.AttributeListElm.AttributeElm.AttributeSeq Seq { get; set; } = new Liquid_Technologies.Ns.ServiceInterfaceElm.AttributeListElm.AttributeElm.AttributeSeq();

                #region Simplified read only accessors
                /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.AttributeListElm.AttributeElm.AttributeSeq.Description" />
                public System.String Description { get { return this.Seq.Description; } }

                /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.AttributeListElm.AttributeElm.AttributeSeq.DeprecateHint" />
                public System.String DeprecateHint { get { return this.Seq.DeprecateHint; } }

                #endregion

                /// <summary>A class representing an xs:sequence.</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:AttributeList/sequence/element:Attribute/sequence</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>143:10-146:24</XsdLocation>
                [LxCompositorDefinition(CompositorType.Sequence)]
                public partial class AttributeSeq
                {
                    /// <summary>A <see cref="System.String" />, Optional : null when not set</summary>
                    [LxElementValue(0, "Description", "", LxValueType.Value, XsdType.XsdString, MinOccurs = 0)]
                    public System.String Description { get; set; }

                    /// <summary>A <see cref="System.String" />, Optional : null when not set</summary>
                    [LxElementValue(1, "DeprecateHint", "", LxValueType.Value, XsdType.XsdString, MinOccurs = 0)]
                    public System.String DeprecateHint { get; set; }

                }

            }

        }

        /// <summary>Represent the inline xs:element MethodList.</summary>
        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:MethodList</XsdPath>
        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
        /// <XsdLocation>163:5-214:18</XsdLocation>
        [LxElementDefinition("MethodList", "", ElementScopeType.InlineElement)]
        public partial class MethodListElm
        {
            /// <summary>A class representing an xs:sequence.</summary>
            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:MethodList/sequence</XsdPath>
            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
            /// <XsdLocation>165:7-212:21</XsdLocation>
            [LxElementCompositor(0)]
            public Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodListSeq Seq { get; set; } = new Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodListSeq();

            #region Simplified read only accessors
            /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodListSeq.Methods" />
            public IEnumerable<Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm> Methods { get { return this.Seq.Methods; } }

            #endregion

            /// <summary>A class representing an xs:sequence.</summary>
            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:MethodList/sequence</XsdPath>
            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
            /// <XsdLocation>165:7-212:21</XsdLocation>
            [LxCompositorDefinition(CompositorType.Sequence)]
            public partial class MethodListSeq
            {
                /// <summary>A collection of <see cref="Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm" /></summary>
                [LxElementRef(0, MaxOccurs = LxConstants.Unbounded)]
                public List<Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm> Methods { get; } = new List<Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm>();

            }

            /// <summary>Represent the inline xs:element Method.</summary>
            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:MethodList/sequence/element:Method</XsdPath>
            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
            /// <XsdLocation>166:8-211:21</XsdLocation>
            [LxElementDefinition("Method", "", ElementScopeType.InlineElement)]
            public partial class MethodElm
            {
                /// <summary>The value for the attribute ID</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:MethodList/sequence/element:Method/attribute:ID</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>199:10-199:68</XsdLocation>
                [LxAttribute("ID", "", LxValueType.Value, XsdType.XsdInteger, Required = true)]
                public System.Numerics.BigInteger ID { get; set; }

                /// <summary>The value for the attribute MethodType</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:MethodList/sequence/element:Method/attribute:MethodType</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>200:10-206:25</XsdLocation>
                [LxAttribute("MethodType", "", LxValueType.Value, XsdType.XsdString, Required = true, Pattern = "request|response|broadcast")]
                public System.String MethodType { get; set; } = "";

                /// <summary>The value for the attribute Name</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:MethodList/sequence/element:Method/attribute:Name</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>207:10-207:70</XsdLocation>
                [LxAttribute("Name", "", LxValueType.Value, XsdType.XsdString, Required = true, Pattern = "[A-Za-z][A-Za-z_0-9]+")]
                public System.String Name { get; set; } = "";

                /// <summary>The value for the optional attribute Response</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:MethodList/sequence/element:Method/attribute:Response</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>208:10-208:73</XsdLocation>
                [LxAttribute("Response", "", LxValueType.Value, XsdType.XsdString)]
                public System.String Response { get; set; }

                /// <summary>The value for the optional attribute IsDeprecated</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:MethodList/sequence/element:Method/attribute:IsDeprecated</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>209:10-209:78</XsdLocation>
                [LxAttribute("IsDeprecated", "", LxValueType.Value, XsdType.XsdBoolean)]
                public System.Boolean? IsDeprecated { get; set; }

                /// <summary>A class representing an xs:sequence.</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:MethodList/sequence/element:Method/sequence</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>168:10-198:24</XsdLocation>
                [LxElementCompositor(5)]
                public Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm.MethodSeq Seq { get; set; } = new Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm.MethodSeq();

                #region Simplified read only accessors
                /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm.MethodSeq.Description" />
                public System.String Description { get { return this.Seq.Description; } }

                /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm.MethodSeq.DeprecateHint" />
                public System.String DeprecateHint { get { return this.Seq.DeprecateHint; } }

                /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm.MethodSeq.ParamList" />
                public Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm.ParamListElm ParamList { get { return this.Seq.ParamList; } }

                #endregion

                /// <summary>A class representing an xs:sequence.</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:MethodList/sequence/element:Method/sequence</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>168:10-198:24</XsdLocation>
                [LxCompositorDefinition(CompositorType.Sequence)]
                public partial class MethodSeq
                {
                    /// <summary>A <see cref="System.String" />, Required : should not be set to null</summary>
                    [LxElementValue(0, "Description", "", LxValueType.Value, XsdType.XsdString)]
                    public System.String Description { get; set; } = "";

                    /// <summary>A <see cref="System.String" />, Optional : null when not set</summary>
                    [LxElementValue(1, "DeprecateHint", "", LxValueType.Value, XsdType.XsdString, MinOccurs = 0)]
                    public System.String DeprecateHint { get; set; }

                    /// <summary>A <see cref="Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm.ParamListElm" />, Optional : null when not set</summary>
                    [LxElementRef(2, MinOccurs = 0)]
                    public Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm.ParamListElm ParamList { get; set; }

                }

                /// <summary>Represent the inline xs:element ParamList.</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:MethodList/sequence/element:Method/sequence/element:ParamList</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>171:11-197:24</XsdLocation>
                [LxElementDefinition("ParamList", "", ElementScopeType.InlineElement)]
                public partial class ParamListElm
                {
                    /// <summary>A class representing an xs:sequence.</summary>
                    /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:MethodList/sequence/element:Method/sequence/element:ParamList/sequence</XsdPath>
                    /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                    /// <XsdLocation>173:13-195:27</XsdLocation>
                    [LxElementCompositor(0)]
                    public Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm.ParamListElm.ParamListSeq Seq { get; set; } = new Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm.ParamListElm.ParamListSeq();

                    #region Simplified read only accessors
                    /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm.ParamListElm.ParamListSeq.Parameters" />
                    public IEnumerable<Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm.ParamListElm.ParameterElm> Parameters { get { return this.Seq.Parameters; } }

                    #endregion

                    /// <summary>A class representing an xs:sequence.</summary>
                    /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:MethodList/sequence/element:Method/sequence/element:ParamList/sequence</XsdPath>
                    /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                    /// <XsdLocation>173:13-195:27</XsdLocation>
                    [LxCompositorDefinition(CompositorType.Sequence)]
                    public partial class ParamListSeq
                    {
                        /// <summary>A collection of <see cref="Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm.ParamListElm.ParameterElm" /></summary>
                        [LxElementRef(0, MaxOccurs = LxConstants.Unbounded)]
                        public List<Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm.ParamListElm.ParameterElm> Parameters { get; } = new List<Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm.ParamListElm.ParameterElm>();

                    }

                    /// <summary>Represent the inline xs:element Parameter.</summary>
                    /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:MethodList/sequence/element:Method/sequence/element:ParamList/sequence/element:Parameter</XsdPath>
                    /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                    /// <XsdLocation>174:14-194:27</XsdLocation>
                    [LxElementDefinition("Parameter", "", ElementScopeType.InlineElement)]
                    public partial class ParameterElm
                    {
                        /// <summary>The value for the attribute DataType</summary>
                        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:MethodList/sequence/element:Method/sequence/element:ParamList/sequence/element:Parameter/attribute:DataType</XsdPath>
                        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                        /// <XsdLocation>189:16-189:87</XsdLocation>
                        [LxAttribute("DataType", "", LxValueType.Union, XsdType.Union, Required = true)]
                        public Liquid_Technologies.Ns.ExtendedValidTypeUnion DataType { get; set; }

                        /// <summary>The value for the attribute ID</summary>
                        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:MethodList/sequence/element:Method/sequence/element:ParamList/sequence/element:Parameter/attribute:ID</XsdPath>
                        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                        /// <XsdLocation>190:16-190:74</XsdLocation>
                        [LxAttribute("ID", "", LxValueType.Value, XsdType.XsdInteger, Required = true)]
                        public System.Numerics.BigInteger ID { get; set; }

                        /// <summary>The value for the attribute Name</summary>
                        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:MethodList/sequence/element:Method/sequence/element:ParamList/sequence/element:Parameter/attribute:Name</XsdPath>
                        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                        /// <XsdLocation>191:16-191:76</XsdLocation>
                        [LxAttribute("Name", "", LxValueType.Value, XsdType.XsdString, Required = true, Pattern = "[A-Za-z][A-Za-z_0-9]+")]
                        public System.String Name { get; set; } = "";

                        /// <summary>The value for the optional attribute IsDeprecated</summary>
                        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:MethodList/sequence/element:Method/sequence/element:ParamList/sequence/element:Parameter/attribute:IsDeprecated</XsdPath>
                        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                        /// <XsdLocation>192:16-192:84</XsdLocation>
                        [LxAttribute("IsDeprecated", "", LxValueType.Value, XsdType.XsdBoolean)]
                        public System.Boolean? IsDeprecated { get; set; }

                        /// <summary>
                        ///   A class representing an xs:sequence.
                        ///   <br/>Because it is contained in an xs:choice it is optional (initially null).
                        /// </summary>
                        /// <remarks>
                        ///   null if this is not the populated item within the containing xs:choice
                        /// </remarks>
                        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:MethodList/sequence/element:Method/sequence/element:ParamList/sequence/element:Parameter/sequence</XsdPath>
                        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                        /// <XsdLocation>176:16-188:30</XsdLocation>
                        [LxElementCompositor(4, MinOccurs = 0)]
                        public Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm.ParamListElm.ParameterElm.ParameterSeq Seq { get; set; }

                        #region Simplified read only accessors
                        /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm.ParamListElm.ParameterElm.ParameterSeq.Value1" />
                        public Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm.ParamListElm.ParameterElm.ValueElm Value1 { get { return this.Seq?.Value1; } }

                        /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm.ParamListElm.ParameterElm.ParameterSeq.Description" />
                        public System.String Description { get { return this.Seq?.Description; } }

                        /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm.ParamListElm.ParameterElm.ParameterSeq.DeprecateHint" />
                        public System.String DeprecateHint { get { return this.Seq?.DeprecateHint; } }

                        #endregion

                        /// <summary>A class representing an xs:sequence.</summary>
                        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:MethodList/sequence/element:Method/sequence/element:ParamList/sequence/element:Parameter/sequence</XsdPath>
                        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                        /// <XsdLocation>176:16-188:30</XsdLocation>
                        [LxCompositorDefinition(CompositorType.Sequence)]
                        public partial class ParameterSeq
                        {
                            /// <summary>A <see cref="Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm.ParamListElm.ParameterElm.ValueElm" />, Optional : null when not set</summary>
                            [LxElementRef(0, MinOccurs = 0)]
                            public Liquid_Technologies.Ns.ServiceInterfaceElm.MethodListElm.MethodElm.ParamListElm.ParameterElm.ValueElm Value1 { get; set; }

                            /// <summary>A <see cref="System.String" />, Optional : null when not set</summary>
                            [LxElementValue(1, "Description", "", LxValueType.Value, XsdType.XsdString, MinOccurs = 0)]
                            public System.String Description { get; set; }

                            /// <summary>A <see cref="System.String" />, Optional : null when not set</summary>
                            [LxElementValue(2, "DeprecateHint", "", LxValueType.Value, XsdType.XsdString, MinOccurs = 0)]
                            public System.String DeprecateHint { get; set; }

                        }

                        /// <summary>Represent the inline xs:element Value.</summary>
                        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:MethodList/sequence/element:Method/sequence/element:ParamList/sequence/element:Parameter/sequence/element:Value</XsdPath>
                        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                        /// <XsdLocation>177:17-185:30</XsdLocation>
                        [LxElementDefinition("Value", "", ElementScopeType.InlineElement)]
                        public partial class ValueElm
                        {
                            /// <summary>The value for the attribute IsDefault</summary>
                            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:MethodList/sequence/element:Method/sequence/element:ParamList/sequence/element:Parameter/sequence/element:Value/attribute:IsDefault</XsdPath>
                            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                            /// <XsdLocation>181:21-181:86</XsdLocation>
                            [LxAttribute("IsDefault", "", LxValueType.Value, XsdType.XsdBoolean, Required = true)]
                            public System.Boolean IsDefault { get; set; }

                            /// <summary>Holds the <see cref="System.String" /> (xs:http://www.w3.org/2001/XMLSchema:string) value of the element</summary>
                            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:MethodList/sequence/element:Method/sequence/element:ParamList/sequence/element:Parameter/sequence/element:Value</XsdPath>
                            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                            /// <XsdLocation>177:17-185:30</XsdLocation>
                            [LxValue(LxValueType.Value, XsdType.XsdString)]
                            public System.String Value { get; set; } = "";

                        }

                    }

                }

            }

        }

        /// <summary>Represent the inline xs:element ConstantList.</summary>
        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:ConstantList</XsdPath>
        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
        /// <XsdLocation>215:5-233:18</XsdLocation>
        [LxElementDefinition("ConstantList", "", ElementScopeType.InlineElement)]
        public partial class ConstantListElm
        {
            /// <summary>A class representing an xs:sequence.</summary>
            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:ConstantList/sequence</XsdPath>
            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
            /// <XsdLocation>217:7-231:21</XsdLocation>
            [LxElementCompositor(0)]
            public Liquid_Technologies.Ns.ServiceInterfaceElm.ConstantListElm.ConstantListSeq Seq { get; set; } = new Liquid_Technologies.Ns.ServiceInterfaceElm.ConstantListElm.ConstantListSeq();

            #region Simplified read only accessors
            /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.ConstantListElm.ConstantListSeq.Constants" />
            public IEnumerable<Liquid_Technologies.Ns.ServiceInterfaceElm.ConstantListElm.ConstantElm> Constants { get { return this.Seq.Constants; } }

            #endregion

            /// <summary>A class representing an xs:sequence.</summary>
            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:ConstantList/sequence</XsdPath>
            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
            /// <XsdLocation>217:7-231:21</XsdLocation>
            [LxCompositorDefinition(CompositorType.Sequence)]
            public partial class ConstantListSeq
            {
                /// <summary>A collection of <see cref="Liquid_Technologies.Ns.ServiceInterfaceElm.ConstantListElm.ConstantElm" /></summary>
                [LxElementRef(0, MaxOccurs = LxConstants.Unbounded)]
                public List<Liquid_Technologies.Ns.ServiceInterfaceElm.ConstantListElm.ConstantElm> Constants { get; } = new List<Liquid_Technologies.Ns.ServiceInterfaceElm.ConstantListElm.ConstantElm>();

            }

            /// <summary>Represent the inline xs:element Constant.</summary>
            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:ConstantList/sequence/element:Constant</XsdPath>
            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
            /// <XsdLocation>218:8-230:21</XsdLocation>
            [LxElementDefinition("Constant", "", ElementScopeType.InlineElement)]
            public partial class ConstantElm
            {
                /// <summary>The value for the attribute DataType</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:ConstantList/sequence/element:Constant/attribute:DataType</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>225:10-225:74</XsdLocation>
                [LxAttribute("DataType", "", LxValueType.Enum, XsdType.Enum, Required = true, WhiteSpace = WhiteSpaceType.Preserve)]
                public Liquid_Technologies.Ns.ValidTypesEnum DataType { get; set; }

                /// <summary>The value for the attribute ID</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:ConstantList/sequence/element:Constant/attribute:ID</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>226:10-226:68</XsdLocation>
                [LxAttribute("ID", "", LxValueType.Value, XsdType.XsdInteger, Required = true)]
                public System.Numerics.BigInteger ID { get; set; }

                /// <summary>The value for the attribute Name</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:ConstantList/sequence/element:Constant/attribute:Name</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>227:10-227:70</XsdLocation>
                [LxAttribute("Name", "", LxValueType.Value, XsdType.XsdString, Required = true, Pattern = "[A-Za-z][A-Za-z_0-9]+")]
                public System.String Name { get; set; } = "";

                /// <summary>The value for the optional attribute IsDeprecated</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:ConstantList/sequence/element:Constant/attribute:IsDeprecated</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>228:10-228:78</XsdLocation>
                [LxAttribute("IsDeprecated", "", LxValueType.Value, XsdType.XsdBoolean)]
                public System.Boolean? IsDeprecated { get; set; }

                /// <summary>A class representing an xs:sequence.</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:ConstantList/sequence/element:Constant/sequence</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>220:10-224:24</XsdLocation>
                [LxElementCompositor(4)]
                public Liquid_Technologies.Ns.ServiceInterfaceElm.ConstantListElm.ConstantElm.ConstantSeq Seq { get; set; } = new Liquid_Technologies.Ns.ServiceInterfaceElm.ConstantListElm.ConstantElm.ConstantSeq();

                #region Simplified read only accessors
                /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.ConstantListElm.ConstantElm.ConstantSeq.Value1" />
                public System.String Value1 { get { return this.Seq.Value1; } }

                /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.ConstantListElm.ConstantElm.ConstantSeq.Description" />
                public System.String Description { get { return this.Seq.Description; } }

                /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.ConstantListElm.ConstantElm.ConstantSeq.DeprecateHint" />
                public System.String DeprecateHint { get { return this.Seq.DeprecateHint; } }

                #endregion

                /// <summary>A class representing an xs:sequence.</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:ConstantList/sequence/element:Constant/sequence</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>220:10-224:24</XsdLocation>
                [LxCompositorDefinition(CompositorType.Sequence)]
                public partial class ConstantSeq
                {
                    /// <summary>A <see cref="System.String" />, Required : should not be set to null</summary>
                    [LxElementValue(0, "Value", "", LxValueType.Value, XsdType.XsdString)]
                    public System.String Value1 { get; set; } = "";

                    /// <summary>A <see cref="System.String" />, Optional : null when not set</summary>
                    [LxElementValue(1, "Description", "", LxValueType.Value, XsdType.XsdString, MinOccurs = 0)]
                    public System.String Description { get; set; }

                    /// <summary>A <see cref="System.String" />, Optional : null when not set</summary>
                    [LxElementValue(2, "DeprecateHint", "", LxValueType.Value, XsdType.XsdString, MinOccurs = 0)]
                    public System.String DeprecateHint { get; set; }

                }

            }

        }

        /// <summary>Represent the inline xs:element IncludeList.</summary>
        /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:IncludeList</XsdPath>
        /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
        /// <XsdLocation>234:5-250:18</XsdLocation>
        [LxElementDefinition("IncludeList", "", ElementScopeType.InlineElement)]
        public partial class IncludeListElm
        {
            /// <summary>A class representing an xs:sequence.</summary>
            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:IncludeList/sequence</XsdPath>
            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
            /// <XsdLocation>236:7-248:21</XsdLocation>
            [LxElementCompositor(0)]
            public Liquid_Technologies.Ns.ServiceInterfaceElm.IncludeListElm.IncludeListSeq Seq { get; set; } = new Liquid_Technologies.Ns.ServiceInterfaceElm.IncludeListElm.IncludeListSeq();

            #region Simplified read only accessors
            /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.IncludeListElm.IncludeListSeq.Locations" />
            public IEnumerable<Liquid_Technologies.Ns.ServiceInterfaceElm.IncludeListElm.LocationElm> Locations { get { return this.Seq.Locations; } }

            #endregion

            /// <summary>A class representing an xs:sequence.</summary>
            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:IncludeList/sequence</XsdPath>
            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
            /// <XsdLocation>236:7-248:21</XsdLocation>
            [LxCompositorDefinition(CompositorType.Sequence)]
            public partial class IncludeListSeq
            {
                /// <summary>A collection of <see cref="Liquid_Technologies.Ns.ServiceInterfaceElm.IncludeListElm.LocationElm" /></summary>
                [LxElementRef(0, MaxOccurs = LxConstants.Unbounded)]
                public List<Liquid_Technologies.Ns.ServiceInterfaceElm.IncludeListElm.LocationElm> Locations { get; } = new List<Liquid_Technologies.Ns.ServiceInterfaceElm.IncludeListElm.LocationElm>();

            }

            /// <summary>Represent the inline xs:element Location.</summary>
            /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:IncludeList/sequence/element:Location</XsdPath>
            /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
            /// <XsdLocation>237:8-247:21</XsdLocation>
            [LxElementDefinition("Location", "", ElementScopeType.InlineElement)]
            public partial class LocationElm
            {
                /// <summary>The value for the attribute ID</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:IncludeList/sequence/element:Location/attribute:ID</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>243:10-243:68</XsdLocation>
                [LxAttribute("ID", "", LxValueType.Value, XsdType.XsdInteger, Required = true)]
                public System.Numerics.BigInteger ID { get; set; }

                /// <summary>The value for the attribute Name</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:IncludeList/sequence/element:Location/attribute:Name</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>244:10-244:69</XsdLocation>
                [LxAttribute("Name", "", LxValueType.Value, XsdType.XsdString, Required = true)]
                public System.String Name { get; set; } = "";

                /// <summary>The value for the optional attribute IsDeprecated</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:IncludeList/sequence/element:Location/attribute:IsDeprecated</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>245:10-245:78</XsdLocation>
                [LxAttribute("IsDeprecated", "", LxValueType.Value, XsdType.XsdBoolean)]
                public System.Boolean? IsDeprecated { get; set; }

                /// <summary>A class representing an xs:sequence.</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:IncludeList/sequence/element:Location/sequence</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>239:10-242:24</XsdLocation>
                [LxElementCompositor(3)]
                public Liquid_Technologies.Ns.ServiceInterfaceElm.IncludeListElm.LocationElm.LocationSeq Seq { get; set; } = new Liquid_Technologies.Ns.ServiceInterfaceElm.IncludeListElm.LocationElm.LocationSeq();

                #region Simplified read only accessors
                /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.IncludeListElm.LocationElm.LocationSeq.Description" />
                public System.String Description { get { return this.Seq.Description; } }

                /// <inheritdoc cref="Liquid_Technologies.Ns.ServiceInterfaceElm.IncludeListElm.LocationElm.LocationSeq.DeprecateHint" />
                public System.String DeprecateHint { get { return this.Seq.DeprecateHint; } }

                #endregion

                /// <summary>A class representing an xs:sequence.</summary>
                /// <XsdPath>schema:Copied_Artak_Sample_Mar...xsd/element:ServiceInterface/sequence/element:IncludeList/sequence/element:Location/sequence</XsdPath>
                /// <XsdFile>E:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Liquid_Technologies\Copied_Artak_Sample_March_04.xsd</XsdFile>
                /// <XsdLocation>239:10-242:24</XsdLocation>
                [LxCompositorDefinition(CompositorType.Sequence)]
                public partial class LocationSeq
                {
                    /// <summary>A <see cref="System.String" />, Optional : null when not set</summary>
                    [LxElementValue(0, "Description", "", LxValueType.Value, XsdType.XsdString, MinOccurs = 0)]
                    public System.String Description { get; set; }

                    /// <summary>A <see cref="System.String" />, Optional : null when not set</summary>
                    [LxElementValue(1, "DeprecateHint", "", LxValueType.Value, XsdType.XsdString, MinOccurs = 0)]
                    public System.String DeprecateHint { get; set; }

                }

            }

        }

    }

    #endregion

}

namespace Liquid_Technologies.Xs
{
    #region Complex Types
    /// <summary>A class representing the root XSD complexType anyType@http://www.w3.org/2001/XMLSchema</summary>
    /// <XsdPath>schema:.../www.w3.org/2001/XMLSchema/complexType:anyType</XsdPath>
    /// <XsdFile>http://www.w3.org/2001/XMLSchema</XsdFile>
    /// <XsdLocation>Empty</XsdLocation>
    [LxComplexTypeDefinition("anyType", "http://www.w3.org/2001/XMLSchema")]
    public partial class AnyTypeCt : XElement
    {
        /// <summary>Constructor : create a <see cref="AnyTypeCt" /> element &lt;anyType xmlns='http://www.w3.org/2001/XMLSchema'&gt;</summary>
        public AnyTypeCt()  : base(XName.Get("anyType", "http://www.w3.org/2001/XMLSchema")) { }

    }

    #endregion

}

namespace Liquid_Technologies
{
    /// <summary>
    /// Provides a validator based on the original XSD schema files. 
    /// </summary>
    public partial class CopiedArtakSampleMarch04Validator : LiquidTechnologies.XmlObjects.XsdValidator
    {
        /// <summary>
        /// Initializes the validator, loads and compiles the XSD schemas.
        /// </summary>
        /// <remarks>
        /// This is an expensive operation so consider caching this object.
        /// </remarks>
        public CopiedArtakSampleMarch04Validator()
            : base(typeof(CopiedArtakSampleMarch04Validator).Assembly, "Liquid_Technologies.CopiedArtakSampleMarch04Resources.SchemaData")
        {
        }
    }
}