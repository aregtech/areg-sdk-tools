using AvaloniaUI;
using AvaloniaUI.Models;
using FluentAssertions;

namespace AvaloniaUITest;

/// <summary>
/// Test basic bootstrapping functionality, during which the container registrations occur.
/// </summary>
[TestClass]
public class TestBootstrapper
{
    /// <summary>
    /// Test <see cref="ITestPlugin"/> collections.
    /// </summary>
    [TestMethod]
    public void TestPluginCollections()
    {
        var container = AppContainer.Instance;
        
        // get all plugins of the type
        var pluginList = container.GetAllInstances<ITestPlugin>();

        pluginList.Should().NotBeEmpty()
            .And.HaveCount(2)
            .And.ContainSingle(x => x.GetType() == typeof(TestPlugin1))
            .And.ContainSingle(x => x.GetType() == typeof(TestPlugin2));
    }

    [TestMethod]
    public void TestGettingContainerTwiceOk()
    {
        var container1 = AppContainer.Instance;
        var container2 = AppContainer.Instance;

        container1.Should().BeSameAs(container2);
        
        // each call to get the instances instantiates the plugins afresh ...
        var pluginList1 = container1.GetAllInstances<ITestPlugin>().ToArray();
        var pluginList2 = container2.GetAllInstances<ITestPlugin>().ToArray();

        // ... therefore, the only thing to test is the count
        pluginList1.Should().HaveSameCount(pluginList2);
    }
}