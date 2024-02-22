using AvaloniaUI.Models;
using FluentAssertions;

namespace AvaloniaUITest;

/// <summary>
/// Test basic plugin functionality.
/// </summary>
[TestClass]
public class TestPlugins
{
    /// <summary>
    /// Test <see cref="ITopLevelPlugin"/>.
    /// </summary>
    [TestMethod]
    public void TestPluginCollections()
    {
        var container = AvaloniaUI.Bootstrapper.Bootstrap();
        
        // get all plugins of the type
        var pluginList = container.GetAllInstances<ITopLevelPlugin>();

        pluginList.Should().NotBeEmpty()
            .And.HaveCount(2)
            .And.ContainSingle(x => x.GetType() == typeof(TopLevelPlugin1))
            .And.ContainSingle(x => x.GetType() == typeof(TopLevelPlugin2));
    }
}