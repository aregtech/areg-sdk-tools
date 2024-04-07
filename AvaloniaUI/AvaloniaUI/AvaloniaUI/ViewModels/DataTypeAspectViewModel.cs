using Liquid_Technologies.Ns;

namespace AvaloniaUI.ViewModels;

public class DataTypeAspectViewModel : AspectViewModelBase
{
    public string Test { get; } = "Inside of DataTypeAspectViewModel !!!!!";

    public override ServiceAspectType ServiceAspectType { get; } = ServiceAspectType.DataTypes;

    public override string EditorDescription { get; } = "All Data Types";

    public DataTypeAspectViewModel(ServiceInterfaceElm dataSource) : base(dataSource)
    {
    }
}