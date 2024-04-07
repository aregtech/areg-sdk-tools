using Liquid_Technologies.Ns;

namespace AvaloniaUI.ViewModels;

public class MethodAspectViewModel : AspectViewModelBase
{
    public override ServiceAspectType ServiceAspectType { get; } = ServiceAspectType.Methods;

    public override string EditorDescription { get; } = "All Methods";

    public MethodAspectViewModel(ServiceInterfaceElm dataSource) : base(dataSource)
    {
    }
}