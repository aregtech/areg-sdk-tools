using System.Collections.ObjectModel;
using AvaloniaUI.ViewModels.Msg;
using ReactiveUI;

namespace AvaloniaUI.ViewModels;

public class ServiceTabsViewModel : ViewModelBase
{
    private ObservableCollection<TabItem> _tabItems;
    private TabItem? _selectedTabItem;
    
    public class TabItem(string header, string content)
    {
        public string Header { get; } = header;
        public string Content { get; } = content;
    }

    public ObservableCollection<TabItem> TabItems
    {
        get => _tabItems;
        set => this.RaiseAndSetIfChanged(ref _tabItems, value);
    }

    public TabItem? SelectedTabItem
    {
        get => _selectedTabItem;
        set => this.RaiseAndSetIfChanged(ref _selectedTabItem, value);
    }
        
    public ServiceTabsViewModel()
    {
        _tabItems = new ObservableCollection<TabItem>();
        EventPublisher.GetEvent<OpenServiceMsg>().Subscribe(OnNewServiceMessage);
    }

    private void OnNewServiceMessage(string openServiceFile)
    {
        // Console.WriteLine($"New Service Opened: {openServiceFile}!");
        var newTabItem = new TabItem(openServiceFile, $"Showing Tab Number {_tabNum++}");
        TabItems.Add(newTabItem);
        SelectedTabItem = newTabItem;
    }

    private static int _tabNum = 1;
}