using Liquid_Technologies.Ns;

namespace AvaloniaUI.ViewModels;

public class OverViewAspectViewModel : AspectViewModelBase, IAspectViewModel
{
    public string Test { get; } = "Inside of OverViewAspectViewModel !!!!!";
    
    public ServiceAspectType ServiceAspectType { get; } = ServiceAspectType.Overview;
    
    public string EditorDescription { get; } = "Service Interface Overview";

    public OverViewAspectViewModel(ServiceInterfaceElm dataSource) : base(dataSource)
    {
    }
}