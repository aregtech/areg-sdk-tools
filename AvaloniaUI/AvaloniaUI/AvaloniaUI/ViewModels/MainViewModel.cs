namespace AvaloniaUI.ViewModels;

public class MainViewModel : ViewModelBase
{
    public MainViewModel()
    {
    }

    public string Greeting => "Welcome to Avalonia!";

    public ToolbarViewModel ToolBarViewModel { get; } = new();
    
    public OpenServiceViewModel OpenServiceInteraction { get; } = new();

}