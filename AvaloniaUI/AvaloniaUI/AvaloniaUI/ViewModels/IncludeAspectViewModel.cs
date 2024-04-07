using Liquid_Technologies.Ns;

namespace AvaloniaUI.ViewModels;

public class IncludeAspectViewModel : AspectViewModelBase
{
    public override ServiceAspectType ServiceAspectType { get; } = ServiceAspectType.Includes;

    public override string EditorDescription { get; } = "All Includes";

    public IncludeAspectViewModel(ServiceInterfaceElm dataSource) : base(dataSource)
    {
    }
}