using AvaloniaUI.Helpers;
using Liquid_Technologies.Ns;

namespace AvaloniaUI.ViewModels;

/// <summary>
/// This VM is shared amongst all Aspect-Views of enum <see cref="ServiceAspectType"/>.
/// </summary>
public class AspectViewModel : ViewModelBase
{
    private readonly ServiceInterfaceElm _dataSource;
    
    /// <summary>
    /// Ctor.
    /// </summary>
    /// <param name="dataSource">The data-source used by the Aspect VM shared by Aspect-Views.</param>
    public AspectViewModel(ServiceInterfaceElm dataSource)
    {
        _dataSource = dataSource;
        
        dataSource.PrintServiceInterface();
    }
}