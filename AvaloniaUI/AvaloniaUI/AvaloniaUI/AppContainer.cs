using System;
using SimpleInjector;

namespace AvaloniaUI;

/// <summary>
/// The singleton instance of the application's container.
/// </summary>
public static class AppContainer
{
    private static readonly Lazy<Container> Lazy =
        new Lazy<Container>(() =>
        {
            var temp = new Container();
            
            // Bootstrap the container, once only
            Bootstrapper.Bootstrap(temp);
            return temp;
        });

    public static Container Instance => Lazy.Value;
}