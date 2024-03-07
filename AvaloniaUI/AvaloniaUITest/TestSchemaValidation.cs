using System.Reflection;
using System.Xml;
using System.Xml.Linq;
using System.Xml.Schema;
using FluentAssertions;
using Liquid_Technologies;
using LiquidTechnologies.XmlObjects;
using Liquid_Technologies.Ns;

namespace AvaloniaUITest;

[TestClass]
public class TestSchemaValidation
{
    private const string ElementDoesntExist = @"Doesn't exist!";
    private static int _numErrorsFound;

    private readonly string _localPath =
        new Uri(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location)!).LocalPath;

    private string TestFilesPath => $@"{_localPath}..\..\..\..\TestFiles";

    [TestInitialize]
    public void TestInitialize()
    {
        _numErrorsFound = 0;
    }

    [TestCleanup]
    public void TestCleanup()
    {
        Console.WriteLine(@$"{Environment.NewLine}Number of errors found in the .SIML file = {_numErrorsFound}");

        _numErrorsFound.Should().Be(0, "We don't want any validation errors.");
    }

    /// <summary>
    /// The standard Microsoft way of doing things.
    /// This validates XML files against XSD schemas.
    /// 
    /// Tests "Aregtech.xsd", which was created with Liquid Technologies Online Tools, then edited.
    /// Source was "Sample.siml".
    /// 
    /// Tests "Artak_Sample_March_04.xsd", which was created by Artak by hand.
    /// Source was "Sample.siml" but then modified: enums added, base-types, etc.
    /// </summary>
    [TestMethod]
    [DataRow("Aregtech.xsd", "MarksServiceTest.siml")]
    [DataRow("Aregtech.xsd", "Sample.siml")]
    [DataRow("Artak_Sample_March_04.xsd", "MarksServiceTest.siml")]
    [DataRow("Artak_Sample_March_04.xsd", "Sample.siml")]
    public void TestValidateServiceViaSchema(string schemaFileName, string serviceFileName)
    {
        var xmlReaderSettings = CreateXmlReaderSettings();

        var schema = new XmlSchemaSet();
        schema.Add("", $@"{TestFilesPath}\{schemaFileName}");
        var xmlReader = XmlReader.Create($@"{TestFilesPath}\{serviceFileName}", xmlReaderSettings);

        const LoadOptions loadOptions = LoadOptions.PreserveWhitespace | LoadOptions.SetLineInfo;
        var xmlDocument = XDocument.Load(xmlReader, loadOptions);

        xmlDocument.Validate(schema, ValidationEventHandler);
    }

    /// <summary>
    /// This tests using the generated code from <see href="https://www.liquid-technologies.com/xml-objects"/>.
    /// XSD "Artak_Sample_March_04.xsd" is used, as code was generated against this schema.
    /// See <see cref="CopiedArtakSampleMarch04Validator"/>.
    /// See code at <see href="https://www.liquid-technologies.com/Reference/XmlDataBinding/xml-objects-code-xsd-validation.html"/>.
    /// </summary>
    [TestMethod]
    [DataRow("MarksServiceTest.siml")]
    public void TestLiquidGenCodeFor_Artak_Sample_March_04_Validation_1(string serviceFileName)
    {
        var serviceFileFullPath = $@"{TestFilesPath}\{serviceFileName}";

        var validator = new CopiedArtakSampleMarch04Validator();
        using var validatingReader = validator.CreateValidatingReader(serviceFileFullPath, ValidationEventHandler);

        var serializer = new LxSerializer<ServiceInterfaceElm>();
        var lxReaderSettings = new LxReaderSettings
        {
            ErrorHandler = LxErrorHandler
        };

        var serviceInterfaceElm = serializer.Deserialize(validatingReader, lxReaderSettings);

        // Iterate through the xml file, and print out data
        PrintServiceInterface(serviceInterfaceElm);
    }

    private static void PrintServiceInterface(ServiceInterfaceElm root)
    {
        Console.WriteLine(@"Service Interface:");
        PrintOverview(root.Overview);
        PrintDataTypeList(root.DataTypeList);
    }

    private static void PrintOverview(ServiceInterfaceElm.OverviewElm overview)
    {
        Console.WriteLine($@"{Environment.NewLine}Overview:");
        Console.WriteLine($@"ID = {overview.ID}");
        Console.WriteLine($@"Name = {overview.Name}");
        Console.WriteLine($@"Version = {overview.Version}");
        Console.WriteLine($@"IsRemote = {overview.IsRemote}");
        Console.WriteLine(
            $@"IsDeprecated = {(overview.IsDeprecated.HasValue ? overview.IsDeprecated : ElementDoesntExist)}");

        Console.WriteLine($@"Description = {overview.Description}");
        Console.WriteLine($@"DeprecateHint = {overview.DeprecateHint}");
    }

    private static void PrintDataTypeList(ServiceInterfaceElm.DataTypeListElm dataTypeList)
    {
        Console.WriteLine($@"{Environment.NewLine}DataTypeList:");
        foreach (var dataType in dataTypeList.DataTypes)
        {
            Console.WriteLine($@"{Environment.NewLine}New DataType:");
            Console.WriteLine($@"ID = {dataType.ID}");
            Console.WriteLine($@"Name = {dataType.Name}");
            Console.WriteLine($@"Type = {dataType.Type}");
            Console.WriteLine($@"Values = {dataType.Values}");
            Console.WriteLine(
                $@"IsDeprecated = {(dataType.IsDeprecated.HasValue ? dataType.IsDeprecated : ElementDoesntExist)}");
        }
    }

    /// <summary>
    /// This tests using the generated code from <see href="https://www.liquid-technologies.com/xml-objects"/>.
    /// XSD "Artak_Sample_March_04.xsd" is used, as code was generated against this schema.
    /// See <see cref="CopiedArtakSampleMarch04Validator"/>.
    /// See code at <see href="https://www.liquid-technologies.com/Reference/XmlDataBinding/xml-objects-quick-start-guide.html#i-heading-xml-schema-validation"/>.
    /// </summary>
    [TestMethod]
    [DataRow("MarksServiceTest.siml")]
    public void TestLiquidGenCodeFor_Artak_Sample_March_04_Validation_2(string serviceFileName)
    {
        var xmlReaderSettings = CreateXmlReaderSettings();
        var xmlReader = XmlReader.Create($@"{TestFilesPath}\{serviceFileName}", xmlReaderSettings);

        var validator = new CopiedArtakSampleMarch04Validator();
        using var validatingReader = validator.CreateValidatingReader(xmlReader, ValidationEventHandler);
        var xmlDoc = new XmlDocument();
        xmlDoc.ReadNode(validatingReader);
    }

    private static XmlReaderSettings CreateXmlReaderSettings()
    {
        var settings = new XmlReaderSettings
        {
            ValidationType = ValidationType.Schema
        };
        settings.ValidationFlags |= XmlSchemaValidationFlags.ProcessInlineSchema;
        settings.ValidationFlags |= XmlSchemaValidationFlags.ProcessSchemaLocation;
        settings.ValidationFlags |= XmlSchemaValidationFlags.ReportValidationWarnings;

        return settings;
    }

    private static void ValidationEventHandler(object? sender, ValidationEventArgs args)
    {
        _numErrorsFound++;

        Console.WriteLine(
            $@"Validation: [{args.Severity}] [L {args.Exception.LineNumber}] [Pos {args.Exception.LinePosition}]: {args.Message}");
    }

    /// <summary>
    /// This is called by the Liquid XML Objects serializer.
    /// See <see href="https://www.liquid-technologies.com/Reference/XmlDataBinding/xml-objects-code-xsd-validation.html"/>.
    /// </summary>
    /// <param name="msg"></param>
    /// <param name="severity"></param>
    /// <param name="errorCode"></param>
    /// <param name="location"></param>
    /// <param name="targetObject"></param>
    private static void LxErrorHandler(string msg, LxErrorSeverity severity, LxErrorCode errorCode,
        TextLocation? location, object targetObject)
    {
        Console.WriteLine($@"Liquid XML Objects Validator : {severity} : {msg}");
    }
}