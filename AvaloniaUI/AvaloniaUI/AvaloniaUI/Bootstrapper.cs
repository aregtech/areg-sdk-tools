using System.Reflection;
using AvaloniaUI.Models;
using SimpleInjector;

namespace AvaloniaUI;

public static class Bootstrapper
{
    public static Container Bootstrap()
    {
        // Create the container as usual.
        var container = new Container();

        // Register all instances of ITopLevelPlugin
        container.Collection.Register<ITopLevelPlugin>(Assembly.GetExecutingAssembly());
        
        // container.Register<ITopLevelPlugin, TopLevelPlugin1>(Lifestyle.Transient);
        // container.Register<ITopLevelPlugin, TopLevelPlugin2>(Lifestyle.Transient);


        // Perform verification
        container.Verify();

        return container;
    }
}