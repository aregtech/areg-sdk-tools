using System.Xml.Schema;
using FluentAssertions;

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
        var schemaSet = CreateSchemaSet($@"{TestFilesPath}\{schemaFileName}");
        var xDocument = CreateDocument($@"{TestFilesPath}\{serviceFileName}");
        
        xDocument.Should().NotBeNull();
        xDocument.Validate(schemaSet, ValidationEventHandler);
    }
}