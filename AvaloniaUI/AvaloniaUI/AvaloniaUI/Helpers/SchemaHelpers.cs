using System;
using System.IO;
using System.Xml.Serialization;
using XsdGen;

namespace AvaloniaUI.Helpers;

/// <summary>
/// ----------------------------------
/// Note:
/// serializer.Deserialize(fs) is nothing but a wrapper around XmlReader.Create ( )
/// See: TestValidateServiceViaSchema, which validates !!
/// See: TestReadSimlFile !!
/// ----------------------------------
/// </summary>
public static class SchemaHelpers
{
    public static ServiceInterface? ReadServiceFile(this string serviceFileFullPath)
    {
        var serializer = new XmlSerializer(typeof(ServiceInterface));
        
        // If the XML document has been altered with unknown nodes or attributes, handle them with the
        // UnknownNode and UnknownAttribute events.
        serializer.UnknownNode += Serializer_UnknownNode;
        serializer.UnknownAttribute += Serializer_UnknownAttribute;
        
        var fs = new FileStream(serviceFileFullPath, FileMode.Open);

        var si = serializer.Deserialize(fs) as ServiceInterface;
        return si;
    }

    /// <summary>
    /// Currently, the reading of the service interface file does not validate.
    /// This would do it as a separate step.
    /// </summary>
    /// <param name="serviceInterface"></param>
    public static void ValidateServiceFile(ServiceInterface serviceInterface)
    {
    }
    

    private static void Serializer_UnknownNode(object? sender, XmlNodeEventArgs e)
    {
        Console.WriteLine("Unknown Node:" + e.Name + "\t" + e.Text);
        throw new ApplicationException();
    }

    private static void Serializer_UnknownAttribute(object? sender, XmlAttributeEventArgs e)
    {
        var attr = e.Attr;
        Console.WriteLine("Unknown attribute " + attr.Name + "='" + attr.Value + "'");
        throw new ApplicationException();
    }
}