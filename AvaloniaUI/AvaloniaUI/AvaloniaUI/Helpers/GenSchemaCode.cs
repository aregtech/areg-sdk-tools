using System;
using Liquid_Technologies.Ns;

namespace AvaloniaUI.Helpers;

/// <summary>
/// This directly uses and relies on the code-generation from a schema
/// </summary>
public static class GenSchemaCode
{
    private const string ElementDoesntExist = @"Doesn't exist!";

    internal static void PrintServiceInterface(this ServiceInterfaceElm root)
    {
        Console.WriteLine(@"Service Interface:");
        PrintOverview(root.Overview);
        PrintDataTypeList(root.DataTypeList);
    }

    private static void PrintOverview(ServiceInterfaceElm.OverviewElm overview)
    {
        Console.WriteLine($@"{Environment.NewLine}Overview:");
        Console.WriteLine($@"ID = {overview.ID}");
        Console.WriteLine($@"Name = {overview.Name}");
        Console.WriteLine($@"Version = {overview.Version}");
        Console.WriteLine($@"IsRemote = {overview.IsRemote}");
        Console.WriteLine(
            $@"IsDeprecated = {(overview.IsDeprecated.HasValue ? overview.IsDeprecated : ElementDoesntExist)}");

        Console.WriteLine($@"Description = {overview.Description}");
        Console.WriteLine($@"DeprecateHint = {overview.DeprecateHint}");
    }

    private static void PrintDataTypeList(ServiceInterfaceElm.DataTypeListElm dataTypeList)
    {
        Console.WriteLine($@"{Environment.NewLine}DataTypeList:");
        foreach (var dataType in dataTypeList.DataTypes)
        {
            Console.WriteLine($@"{Environment.NewLine}New DataType:");
            Console.WriteLine($@"ID = {dataType.ID}");
            Console.WriteLine($@"Name = {dataType.Name}");
            Console.WriteLine($@"Type = {dataType.Type}");
            Console.WriteLine($@"Values = {dataType.Values}");
            Console.WriteLine(
                $@"IsDeprecated = {(dataType.IsDeprecated.HasValue ? dataType.IsDeprecated : ElementDoesntExist)}");
        }
    }
}