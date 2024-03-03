using System.Reflection;
using System.Xml;
using System.Xml.Linq;
using System.Xml.Schema;

namespace AvaloniaUITest;

[TestClass]
public class TestModelSchema
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
        Console.WriteLine($"Number of errors found in the .SIML file = {_numErrorsFound}");
    }
    
    [TestMethod]
    public void TestValidateService()
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
        schema.Add("", $@"{testFilesPath}\Aregtech.xsd");
        var xmlReader = XmlReader.Create($@"{testFilesPath}\MarksServiceTest.siml", settings);
        var xmlDocument = XDocument.Load(xmlReader);
        
        xmlDocument.Validate(schema, ValidationEventHandler);
    }
    
    static void ValidationEventHandler(object? sender, ValidationEventArgs args)
    {
        _numErrorsFound++;
        
        if (args.Severity == XmlSeverityType.Warning)
            Console.WriteLine("Warning: Matching schema not found.  No validation occurred." + args.Message);
        else
            Console.WriteLine("Validation error: " + args.Message);    }  
}