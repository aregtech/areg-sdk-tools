using Liquid_Technologies.Ns;

namespace AvaloniaUI.ViewModels;

public class OverViewAspectViewModel : AspectViewModelBase
{
    public string Test { get; } = "Inside of OverViewAspectViewModel !!!!!";
    
    public override ServiceAspectType ServiceAspectType { get; } = ServiceAspectType.Overview;
    
    public override string EditorDescription { get; } = "Service Interface General Description";

    public OverViewAspectViewModel(ServiceInterfaceElm dataSource) : base(dataSource)
    {
    }
}