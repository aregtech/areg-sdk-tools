using System;
using System.Xml;
using System.Xml.Linq;
using System.Xml.Schema;
using System.Xml.Serialization;
using XsdGen;

namespace AvaloniaUI.Helpers;

public static class SchemaHelpers
{
    private static int NumErrorsFound { get; set; }

    public static ServiceInterface? ReadServiceFile(this string serviceFileFullPath)
    {
        const string turkedSchemaFileLocation = @"e:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Schema\Copied_Artak_Sample_March_04.xsd";

        NumErrorsFound = 0;
        try
        {
            var schemaSet = CreateSchemaSet(turkedSchemaFileLocation);
            var xDocument = CreateDocument(serviceFileFullPath);
        
            xDocument.Validate(schemaSet, ValidationEventHandler);
            var serviceInterface = CreateServiceInterface(xDocument, Serializer_UnknownNode, Serializer_UnknownAttribute);

            return serviceInterface;
        }
        catch (Exception e)
        {
            Console.WriteLine(e);
            throw;
        }
    }
    
    private static XmlSchemaSet CreateSchemaSet(string schemaFullFileName)
    {
        var xmlSchemaSet = new XmlSchemaSet();
        xmlSchemaSet.Add("", schemaFullFileName);
        return xmlSchemaSet;
    }

    private static XDocument CreateDocument(string serviceFullFileName)
    {
        var xmlReaderSettings = CreateXmlReaderSettings();

        using var xmlReader = XmlReader.Create(serviceFullFileName, xmlReaderSettings);

        const LoadOptions loadOptions = LoadOptions.PreserveWhitespace | LoadOptions.SetLineInfo;
        return XDocument.Load(xmlReader, loadOptions);
    }

    /// <summary>
    /// This de-serializes the XML document into a <see cref="ServiceInterface"/>.
    /// </summary>
    /// <param name="xDocument"></param>
    /// <param name="nodeEventHandler"></param>
    /// <param name="attributeEventHandler"></param>
    /// <returns></returns>
    private static ServiceInterface? CreateServiceInterface(XDocument xDocument,
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
    
    private static XmlReader? GetReader(XDocument doc) => doc.Root?.CreateReader();
    
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


    /// <summary>
    /// Currently, the reading of the service interface file does not validate.
    /// This would do it as a separate step.
    /// </summary>
    /// <param name="serviceInterface"></param>
    public static void ValidateServiceFile(ServiceInterface serviceInterface)
    {
    }

    
    private static void ValidationEventHandler(object? sender, ValidationEventArgs args)
    {
        Console.WriteLine(
            $@"Validation: [{args.Severity}] [L {args.Exception.LineNumber}] [Pos {args.Exception.LinePosition}]: {args.Message}");

        NumErrorsFound++;
        
        // throw new ValidationException(args.Message);
    }

    private static void Serializer_UnknownNode(object? sender, XmlNodeEventArgs args)
    {
        Console.WriteLine("Unknown Node:" + args.LineNumber + "\t" + args.Name + "\t" + args.Text);
        NumErrorsFound++;
    }

    private static void Serializer_UnknownAttribute(object? sender, XmlAttributeEventArgs args)
    {
        var attr = args.Attr;
        Console.WriteLine("Unknown attribute " + attr.Name + "='" + attr.Value + "'");
        NumErrorsFound++;
    }
}