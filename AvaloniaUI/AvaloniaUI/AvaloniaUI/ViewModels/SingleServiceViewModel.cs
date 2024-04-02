namespace AvaloniaUI.ViewModels;

public class SingleServiceViewModel : ViewModelBase
{
    private readonly string _openServiceFileFullPath;
    private static int _tabNum;
    
    public string TabMessage { get; set; }
    
    public SingleServiceViewModel(string openServiceFileFullPath)
    {
        _openServiceFileFullPath = openServiceFileFullPath;
        TabMessage = $"Single-Service-View {_tabNum++} ...";
    }

    static SingleServiceViewModel()
    {
        _tabNum = 1;
    }
}