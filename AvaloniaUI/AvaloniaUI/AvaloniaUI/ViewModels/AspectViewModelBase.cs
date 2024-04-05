using AvaloniaUI.Helpers;
using Liquid_Technologies.Ns;

namespace AvaloniaUI.ViewModels;

/// <summary>
/// The base VM for all Aspect-Views of enum <see cref="ServiceAspectType"/>.
/// </summary>
public abstract class AspectViewModelBase : ViewModelBase
{
    protected ServiceInterfaceElm DataSource { get; }
    
    /// <summary>
    /// The base Ctor for all Aspect View-Models.
    /// </summary>
    /// <param name="dataSource">The data-source shared by all Aspect View-Models.</param>
    protected AspectViewModelBase(ServiceInterfaceElm dataSource)
    {
        DataSource = dataSource;
        
        dataSource.PrintServiceInterface();
    }
}