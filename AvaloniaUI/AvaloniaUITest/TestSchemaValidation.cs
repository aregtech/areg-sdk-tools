using System.Xml;
using System.Xml.Linq;
using System.Xml.Schema;

namespace AvaloniaUITest;

/// <summary>
/// This tests basic validation of the schema and corresponding .siml files.
/// </summary>
[TestClass]
public class TestSchemaValidation : TestSchemaBase
{
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

    private void ValidationEventHandler(object? sender, ValidationEventArgs args)
    {
        NumErrorsFound++;

        Console.WriteLine(
            $@"Validation: [{args.Severity}] [L {args.Exception.LineNumber}] [Pos {args.Exception.LinePosition}]: {args.Message}");
    }
}