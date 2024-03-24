namespace AvaloniaUI.ViewModels;

public class MainWindowViewModel : ViewModelBase
{
    public MainWindowViewModel()
    {
    }

    public string Greeting => "Welcome to Avalonia!";

    public OpenServiceViewModel OpenServiceInteraction { get; } = new();

}