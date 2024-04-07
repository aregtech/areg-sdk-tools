using Liquid_Technologies.Ns;

namespace AvaloniaUI.ViewModels;

public class AttributeAspectViewModel : AspectViewModelBase
{
    public override ServiceAspectType ServiceAspectType { get; } = ServiceAspectType.Attributes;

    public override string EditorDescription { get; } = "All Attributes";

    public AttributeAspectViewModel(ServiceInterfaceElm dataSource) : base(dataSource)
    {
    }
}