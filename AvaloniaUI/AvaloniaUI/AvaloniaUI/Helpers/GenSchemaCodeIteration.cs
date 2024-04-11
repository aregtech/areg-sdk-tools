using System;
using System.Collections.ObjectModel;
using XsdGen;

namespace AvaloniaUI.Helpers;

/// <summary>
/// This directly uses and relies on the code-generation from a schema.
/// It is throw-away code for development purposes only.
/// </summary>
public static class GenSchemaCodeIteration
{
    private const string ElementDoesntExist = @"Doesn't exist!";

    public static void PrintServiceInterface(this ServiceInterface root)
    {
        Console.WriteLine(@"Service Interface:");
        PrintOverview(root.Overview);
        PrintDataTypeList(root.DataTypeList);
    }

    private static void PrintOverview(ServiceInterfaceOverview overview)
    {
        Console.WriteLine($@"{Environment.NewLine}Overview:");
        Console.WriteLine($@"ID = {overview.Id}");
        Console.WriteLine($@"IsDeprecated = {overview.IsDeprecated}");
        Console.WriteLine($@"Name = {overview.Name}");
        Console.WriteLine($@"Version = {overview.Version}");
        Console.WriteLine($@"IsRemote = {overview.IsRemote}");
        Console.WriteLine(
            $@"IsDeprecated = {(overview.IsDeprecated.HasValue ? overview.IsDeprecated : ElementDoesntExist)}");

        Console.WriteLine($@"Description = {overview.Description}");
        Console.WriteLine($@"DeprecateHint = {overview.DeprecateHint}");
    }

    private static void PrintDataTypeList(Collection<ServiceInterfaceDataTypeListDataType> dataTypeList)
    {
        Console.WriteLine($@"{Environment.NewLine}DataTypeList:");
        foreach (var dataType in dataTypeList)
        {
            Console.WriteLine($@"{Environment.NewLine}New DataType:");
            Console.WriteLine($@"ID = {dataType.Id}");
            Console.WriteLine($@"IsDeprecated = {dataType.IsDeprecated}");
            Console.WriteLine($@"Name = {dataType.Name}");
            Console.WriteLine($@"Type = {dataType.Type}");
            Console.WriteLine($@"Values = {dataType.Values}");
            Console.WriteLine(
                $@"IsDeprecated = {(dataType.IsDeprecated.HasValue ? dataType.IsDeprecated : ElementDoesntExist)}");
        }
    }
}