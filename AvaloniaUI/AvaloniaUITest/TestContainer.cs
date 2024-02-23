using AvaloniaUI;
using FluentAssertions;

namespace AvaloniaUITest;

/// <summary>
/// Test basic container functionality.
/// </summary>
[TestClass]
public class TestContainer
{
    /// <summary>
    /// Basic container test.
    /// </summary>
    [TestMethod]
    public void TestContainer_1()
    {
        var container = AppContainer.Instance;
        container.Should().NotBeNull();
    }
}