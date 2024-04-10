using XsdGen;

namespace AvaloniaUI.ViewModels;

public class MethodAspectViewModel : AspectViewModelBase
{
    public override ServiceAspectType ServiceAspectType { get; } = ServiceAspectType.Methods;

    public override string EditorDescription { get; } = "All Methods";

    public MethodAspectViewModel(ServiceInterface dataSource) : base(dataSource)
    {
    }
}