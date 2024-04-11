using XsdGen;

namespace AvaloniaUI.ViewModels;

/// <summary>
/// The base VM for all Aspect-Views of enum <see cref="ServiceAspectType"/>.
/// </summary>
public abstract class AspectViewModelBase : ViewModelBase , IAspectViewModel
{
    protected ServiceInterface DataSource { get; }
    
    /// <summary>
    /// The base Ctor for all Aspect View-Models.
    /// </summary>
    /// <param name="dataSource">The data-source shared by all Aspect View-Models.</param>
    protected AspectViewModelBase(ServiceInterface dataSource)
    {
        DataSource = dataSource;
    }

    public abstract ServiceAspectType ServiceAspectType { get; }
    
    public abstract string EditorDescription { get; }
}