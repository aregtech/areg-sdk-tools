using System.Reflection;
using System.Xml;
using System.Xml.Linq;
using System.Xml.Schema;
using System.Xml.Serialization;
using FluentAssertions;
using XsdGen;

namespace AvaloniaUITest;

public abstract class TestSchemaBase
{
    private readonly string _localPath =
        new Uri(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location)!).LocalPath;

    protected string TestFilesPath => $@"{_localPath}..\..\..\..\TestFiles";

    private int NumErrorsFound { get; set; }

    [TestInitialize]
    public void TestInitialize()
    {
        NumErrorsFound = 0;
    }

    [TestCleanup]
    public void TestCleanup()
    {
        Console.WriteLine(@$"{Environment.NewLine}Number of errors found in the .SIML file = {NumErrorsFound}");

        NumErrorsFound.Should().Be(0, "We don't want any validation errors.");
    }

    protected void ValidationEventHandler(object? sender, ValidationEventArgs args)
    {
        NumErrorsFound++;

        Console.WriteLine(
            $@"Validation: [{args.Severity}] [L {args.Exception.LineNumber}] [Pos {args.Exception.LinePosition}]: {args.Message}");
    }

    protected void Serializer_UnknownNode(object? sender, XmlNodeEventArgs e)
    {
        NumErrorsFound++;

        Console.WriteLine("Unknown Node:" + e.Name + "\t" + e.Text);
    }

    protected void Serializer_UnknownAttribute(object? sender, XmlAttributeEventArgs e)
    {
        NumErrorsFound++;

        var attr = e.Attr;
        Console.WriteLine("Unknown attribute " + attr.Name + "='" + attr.Value + "'");
    }

    protected static XDocument CreateDocument(string serviceFullFileName)
    {
        var xmlReaderSettings = CreateXmlReaderSettings();

        using var xmlReader = XmlReader.Create(serviceFullFileName, xmlReaderSettings);

        const LoadOptions loadOptions = LoadOptions.PreserveWhitespace | LoadOptions.SetLineInfo;
        return XDocument.Load(xmlReader, loadOptions);
    }

    protected static XmlSchemaSet CreateSchemaSet(string schemaFullFileName)
    {
        var xmlSchemaSet = new XmlSchemaSet();
        xmlSchemaSet.Add("", schemaFullFileName);
        return xmlSchemaSet;
    }

    /// <summary>
    /// This de-serializes the XML document into a <see cref="ServiceInterface"/>.
    /// </summary>
    /// <param name="xDocument"></param>
    /// <param name="nodeEventHandler"></param>
    /// <param name="attributeEventHandler"></param>
    /// <returns></returns>
    protected static ServiceInterface? CreateServiceInterface(XDocument xDocument,
        XmlNodeEventHandler nodeEventHandler,
        XmlAttributeEventHandler attributeEventHandler)
    {
        var serializer = new XmlSerializer(typeof(ServiceInterface));
        serializer.UnknownNode += nodeEventHandler;
        serializer.UnknownAttribute += attributeEventHandler;

        using var xmlReader = GetReader(xDocument);
        if (xmlReader == null) return null;
        var serviceInterface = serializer.Deserialize(xmlReader) as ServiceInterface;
        return serviceInterface;
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

    private static XmlReader? GetReader(XDocument doc) => doc.Root?.CreateReader();
}