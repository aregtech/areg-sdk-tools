using System;
using System.Xml.Schema;
using Liquid_Technologies;
using Liquid_Technologies.Ns;
using LiquidTechnologies.XmlObjects;

namespace AvaloniaUI.Helpers;

public static class SchemaHelpers
{
    private static int _numReadErrorsFound;

    public static ServiceInterfaceElm ReadServiceFile(this string serviceFileFullPath)
    {
        var validator = new CopiedArtakSampleMarch04Validator();
        using var validatingReader = validator.CreateValidatingReader(serviceFileFullPath, ValidationEventHandler);

        var serializer = new LxSerializer<ServiceInterfaceElm>();
        var lxReaderSettings = new LxReaderSettings
        {
            ErrorHandler = LxErrorHandler
        };

        var serviceInterfaceElm = serializer.Deserialize(validatingReader, lxReaderSettings);
        return serviceInterfaceElm;
    }
    
    private static void ValidationEventHandler(object? sender, ValidationEventArgs args)
    {
        _numReadErrorsFound++;

        Console.WriteLine(
            $@"Validation: [{args.Severity}] [L {args.Exception.LineNumber}] [Pos {args.Exception.LinePosition}]: {args.Message}");
    }

    /// <summary>
    /// This is called by the Liquid XML Objects serializer.
    /// See <see href="https://www.liquid-technologies.com/Reference/XmlDataBinding/xml-objects-code-xsd-validation.html"/>.
    /// </summary>
    private static void LxErrorHandler(string msg, LxErrorSeverity severity, LxErrorCode errorCode,
        TextLocation? location, object targetObject)
    {
        Console.WriteLine($@"Liquid XML Objects Validator : {severity} : {msg}");
    }
}