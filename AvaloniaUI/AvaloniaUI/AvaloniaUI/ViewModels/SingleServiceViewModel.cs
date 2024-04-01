namespace AvaloniaUI.ViewModels;

public class SingleServiceViewModel : ViewModelBase
{
    private static int _tabNum;
    
    public string TabMessage { get; set; }
    
    public SingleServiceViewModel()
    {
        TabMessage = $"Single-Service-View {_tabNum++} ...";
    }

    static SingleServiceViewModel()
    {
        _tabNum = 1;
    }
}