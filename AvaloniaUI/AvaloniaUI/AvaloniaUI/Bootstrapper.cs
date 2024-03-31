using System.Reflection;
using AvaloniaUI.Models;
using Prism.Events;
using SimpleInjector;

namespace AvaloniaUI;

/// <summary>
/// The <see cref="Bootstrapper"/> bootstraps the application.
/// This is the location where service-registrations should occur.
/// <remarks>
/// No verification needed: container.Options.EnableAutoVerification is enabled, by default.
/// <seealso href="https://docs.simpleinjector.org/en/latest/using.html#verifying-the-container-s-configuration"/>
/// </remarks>
/// 
/// </summary>
internal static class Bootstrapper
{
    /// <summary>
    /// Create a container, add required registrations, return that container.
    /// </summary>
    /// <returns>The instantiated container containing all required registrations.</returns>
    internal static void Bootstrap(Container container)
    {
        // Register all instances of ITopLevelPlugin
        container.Collection.Register<ITestPlugin>(Assembly.GetExecutingAssembly());

        // Register the event-aggregator
        container.Register<IEventAggregator, EventAggregator>(Lifestyle.Singleton);
        
        // No verification required; it is performed automatically upon the first retrieval of
        // a service by the app.
        // container.Verify();
    }
}