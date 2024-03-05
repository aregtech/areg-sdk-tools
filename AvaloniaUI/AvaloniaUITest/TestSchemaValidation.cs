using System.Reflection;
using System.Xml;
using System.Xml.Linq;
using System.Xml.Schema;
using FluentAssertions;

namespace AvaloniaUITest;

[TestClass]
public class TestSchemaValidation
{
    private static int _numErrorsFound;

    [TestInitialize]
    public void TestInitialize()
    {
        _numErrorsFound = 0;
    }

    [TestCleanup]
    public void TestCleanup()
    {
        Console.WriteLine($"{Environment.NewLine}Number of errors found in the .SIML file = {_numErrorsFound}");
    }

    /// <summary>
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
        // Set the validation settings.
        var settings = new XmlReaderSettings
        {
            ValidationType = ValidationType.Schema
        };
        settings.ValidationFlags |= XmlSchemaValidationFlags.ProcessInlineSchema;
        settings.ValidationFlags |= XmlSchemaValidationFlags.ProcessSchemaLocation;
        settings.ValidationFlags |= XmlSchemaValidationFlags.ReportValidationWarnings;

        var localPath = new Uri(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location)!).LocalPath;
        var testFilesPath = $@"{localPath}..\..\..\..\TestFiles";

        var schema = new XmlSchemaSet();
        schema.Add("", $@"{testFilesPath}\{schemaFileName}");
        var xmlReader = XmlReader.Create($@"{testFilesPath}\{serviceFileName}", settings);
        
        const LoadOptions loadOptions = LoadOptions.PreserveWhitespace | LoadOptions.SetLineInfo;
        var xmlDocument = XDocument.Load(xmlReader, loadOptions);

        xmlDocument.Validate(schema, ValidationEventHandler);
        _numErrorsFound.Should().Be(0, "We don't want any validation errors.");
    }

    static void ValidationEventHandler(object? sender, ValidationEventArgs args)
    {
        _numErrorsFound++;

        if (args.Severity == XmlSeverityType.Warning)
            Console.WriteLine("Warning: Matching schema not found.  No validation occurred." + args.Message);
        else
            Console.WriteLine(
                $"Validation error: [L {args.Exception.LineNumber}] [Pos {args.Exception.LinePosition}]: {args.Message}");
    }
}