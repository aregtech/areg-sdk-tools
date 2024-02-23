using System;

namespace AvaloniaUI.Models;

/// <summary>
/// Some plugins for testing, only.
/// </summary>
public interface ITestPlugin
{
    void DoSomething();
}

public class TestPlugin1 : ITestPlugin
{
    public TestPlugin1()
    {
    }
    
    public void DoSomething()
    {
        throw new NotImplementedException();
    }
}

public class TestPlugin2 : ITestPlugin
{
    public TestPlugin2()
    {
    }
    
    public void DoSomething()
    {
        throw new NotImplementedException();
    }
}
