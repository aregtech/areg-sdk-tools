using System;
using System.Xml;
using System.Xml.Linq;
using System.Xml.Schema;
using System.Xml.Serialization;
using AvaloniaUI.ViewModels.Msg;
using Prism.Events;
using XsdGen;

namespace AvaloniaUI.Helpers;

public class SchemaHelper(IEventAggregator eventAggregator)
{
    private int NumErrorsFound { get; set; }
    private IEventAggregator _eventAggregator = eventAggregator;

    public ServiceInterface? ReadServiceFile(string serviceFileFullPath)
    {
        const string turkedSchemaFileLocation =
            @"e:\all\Data\Jobs\Aregtech\Repos\areg-sdk-tools\AvaloniaUI\Schema\Copied_Artak_Sample_March_04.xsd";

        NumErrorsFound = 0;
        try
        {
            var schemaSet = CreateSchemaSet(turkedSchemaFileLocation);
            var xDocument = CreateDocument(serviceFileFullPath);

            xDocument.Validate(schemaSet, ValidationEventHandler);
            var serviceInterface =
                CreateServiceInterface(xDocument, Serializer_UnknownNode, Serializer_UnknownAttribute);

            return serviceInterface;
        }
        catch (Exception e)
        {
            Console.WriteLine(e);
            throw;
        }
    }

    private XmlSchemaSet CreateSchemaSet(string schemaFullFileName)
    {
        var xmlSchemaSet = new XmlSchemaSet();
        xmlSchemaSet.Add("", schemaFullFileName);
        return xmlSchemaSet;
    }

    private XDocument CreateDocument(string serviceFullFileName)
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
    private ServiceInterface? CreateServiceInterface(XDocument xDocument,
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

    private XmlReader? GetReader(XDocument doc) => doc.Root?.CreateReader();

    private XmlReaderSettings CreateXmlReaderSettings()
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
    public void ValidateServiceFile(ServiceInterface serviceInterface)
    {
    }


    private void ValidationEventHandler(object? sender, ValidationEventArgs args)
    {
        var msg =
            $"Validation: [{args.Severity}] [L {args.Exception.LineNumber}] [Pos {args.Exception.LinePosition}]: {args.Message}";
        _eventAggregator.GetEvent<LogMessage>().Publish(msg);
        
        NumErrorsFound++;
    }

    private void Serializer_UnknownNode(object? sender, XmlNodeEventArgs args)
    {
        var msg = "Unknown Node:" + args.LineNumber + "\t" + args.Name + "\t" + args.Text;
        _eventAggregator.GetEvent<LogMessage>().Publish(msg);
        
        NumErrorsFound++;
    }

    private void Serializer_UnknownAttribute(object? sender, XmlAttributeEventArgs args)
    {
        var attr = args.Attr;
        var msg = "Unknown attribute " + attr.Name + "='" + attr.Value + "'";
        _eventAggregator.GetEvent<LogMessage>().Publish(msg);
        
        NumErrorsFound++;
    }
}