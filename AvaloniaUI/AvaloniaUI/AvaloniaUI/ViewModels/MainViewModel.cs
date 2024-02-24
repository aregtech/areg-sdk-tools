namespace AvaloniaUI.ViewModels;

public class MainViewModel : ViewModelBase
{

    public string Greeting => "Welcome to Avalonia!";

    public OpenServiceViewModel OpenServiceInteraction { get; } = new();

}