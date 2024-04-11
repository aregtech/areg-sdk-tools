using System;
using System.Collections.ObjectModel;
using System.Linq;
using AvaloniaUI.Helpers;
using ReactiveUI;
using XsdGen;

namespace AvaloniaUI.ViewModels;

/// <summary>
/// The item for a single aspect of type <see cref="ViewModels.ServiceAspectType"/>.
/// It serves as the View-Model for the ServiceTabView.
/// It serves as the DataContext of each Aspect-View.
/// </summary>
public class ServiceAspectTabItem(AspectViewModelBase aspectViewModelBase)
    : ViewModelBase
{
    public string Header { get; } = aspectViewModelBase.ServiceAspectType.ToString();

    /// <summary>
    /// The specific Aspect-ViewModel for this aspect.
    /// </summary>
    public AspectViewModelBase AspectViewModelBase { get; } = aspectViewModelBase;
}

public class ServiceTabViewModel : ViewModelBase
{
    private ObservableCollection<ServiceAspectTabItem> _serviceAspectTabItems = [];
    private ServiceAspectTabItem? _selectedServiceAspectTabItem;

    public ObservableCollection<ServiceAspectTabItem> ServiceAspectTabItems
    {
        get => _serviceAspectTabItems;
        set => this.RaiseAndSetIfChanged(ref _serviceAspectTabItems, value);
    }

    public ServiceAspectTabItem? SelectedServiceAspectTabItem
    {
        get => _selectedServiceAspectTabItem;
        set => this.RaiseAndSetIfChanged(ref _selectedServiceAspectTabItem, value);
    }

    public ServiceTabViewModel(string openServiceFileFullPath)
    {
        var dataSource = openServiceFileFullPath.ReadServiceFile();
        if (dataSource == null)
        {
            throw new ApplicationException();
        }

        dataSource.PrintServiceInterface();

        foreach (var literal in Enum.GetNames(typeof(ServiceAspectType)))
        {
            var aspectType = literal.ToEnum<ServiceAspectType>();
            _serviceAspectTabItems
                .Add(new ServiceAspectTabItem(AspectViewModelFactory(aspectType, dataSource)));
        }

        SelectedServiceAspectTabItem = _serviceAspectTabItems.First();
    }

    private static AspectViewModelBase AspectViewModelFactory(
        ServiceAspectType aspectType,
        ServiceInterface dataSource)
    {
        return aspectType switch
        {
            ServiceAspectType.Overview => new OverViewAspectViewModel(dataSource),
            ServiceAspectType.DataTypes => new DataTypeAspectViewModel(dataSource),
            ServiceAspectType.Attributes => new AttributeAspectViewModel(dataSource),
            ServiceAspectType.Methods => new MethodAspectViewModel(dataSource),
            ServiceAspectType.Constants => new ConstantsAspectViewModel(dataSource),
            ServiceAspectType.Includes => new IncludeAspectViewModel(dataSource),
            _ => throw new ArgumentOutOfRangeException(nameof(aspectType), aspectType, null)
        };
    }
}