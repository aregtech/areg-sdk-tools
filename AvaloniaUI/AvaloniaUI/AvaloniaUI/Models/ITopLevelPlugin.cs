namespace AvaloniaUI.Models;

/// <summary>
/// !
/// </summary>
public interface ITopLevelPlugin
{
    void DoSomething();
}

public class TopLevelPlugin1 : ITopLevelPlugin
{
    public void DoSomething()
    {
        throw new System.NotImplementedException();
    }
}

public class TopLevelPlugin2 : ITopLevelPlugin
{
    public void DoSomething()
    {
        throw new System.NotImplementedException();
    }
}

