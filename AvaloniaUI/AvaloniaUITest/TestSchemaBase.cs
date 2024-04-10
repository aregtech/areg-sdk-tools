using System.Reflection;
using FluentAssertions;

namespace AvaloniaUITest;

public abstract class TestSchemaBase
{
    private readonly string _localPath =
        new Uri(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location)!).LocalPath;

    protected string TestFilesPath => $@"{_localPath}..\..\..\..\TestFiles";
    
    protected int NumErrorsFound { get; set; }
    
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
}