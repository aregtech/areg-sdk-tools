using Liquid_Technologies.Ns;

namespace AvaloniaUI.ViewModels;

public class ConstantsAspectViewModel : AspectViewModelBase
{
    public override ServiceAspectType ServiceAspectType { get; } = ServiceAspectType.Constants;

    public override string EditorDescription { get; } = "All Constants";

    public ConstantsAspectViewModel(ServiceInterfaceElm dataSource) : base(dataSource)
    {
    }
}