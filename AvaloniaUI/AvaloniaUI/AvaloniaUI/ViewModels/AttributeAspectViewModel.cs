using XsdGen;

namespace AvaloniaUI.ViewModels;

public class AttributeAspectViewModel : AspectViewModelBase
{
    public override ServiceAspectType ServiceAspectType { get; } = ServiceAspectType.Attributes;

    public override string EditorDescription { get; } = "All Attributes";

    public AttributeAspectViewModel(ServiceInterface dataSource) : base(dataSource)
    {
    }
}