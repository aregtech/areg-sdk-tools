using System;
using System.Collections.ObjectModel;
using System.Linq;
using AvaloniaUI.Helpers;
using Liquid_Technologies.Ns;
using ReactiveUI;

namespace AvaloniaUI.ViewModels;

/// <summary>
/// The item for a single aspect of type <see cref="ViewModels.ServiceAspectType"/>.
/// It serves as the View-Model for the ServiceTabView.
/// It serves as the DataContext of each Aspect-View.
/// </summary>
public class ServiceAspectTabItem(ServiceAspectType serviceAspectType, AspectViewModelBase aspectViewModelBase)
    : ViewModelBase
{
    public string Header { get; } = serviceAspectType.ToString();

    /// <summary>
    /// Need this: it is the trigger for switching the view, and is once per tab.
    /// </summary>
    public ServiceAspectType ServiceAspectType { get; } = serviceAspectType;

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

        foreach (var literal in Enum.GetNames(typeof(ServiceAspectType)))
        {
            var aspectType = literal.ToEnum<ServiceAspectType>();
            _serviceAspectTabItems
                .Add(new ServiceAspectTabItem(aspectType, AspectViewModelFactory(aspectType, dataSource)));
        }

        SelectedServiceAspectTabItem = _serviceAspectTabItems.First();
    }

    private static AspectViewModelBase AspectViewModelFactory(ServiceAspectType aspectType,
        ServiceInterfaceElm dataSource)
    {
        switch (aspectType)
        {
            case ServiceAspectType.Overview:
                return new OverViewAspectViewModel(dataSource);
            case ServiceAspectType.DataTypes:
            case ServiceAspectType.Attributes:
            case ServiceAspectType.Methods:
            case ServiceAspectType.Constants:
            case ServiceAspectType.Includes:
            default:
                return new OverViewAspectViewModel(dataSource);
                // throw new ArgumentOutOfRangeException(nameof(aspectType), aspectType, null);
        }
    }
}