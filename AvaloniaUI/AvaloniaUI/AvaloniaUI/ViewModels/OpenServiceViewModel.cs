using System.Reactive.Linq;
using System.Threading.Tasks;
using System.Windows.Input;
using ReactiveUI;

namespace AvaloniaUI.ViewModels;

public class OpenServiceViewModel : ViewModelBase
{
    private readonly Interaction<string?, string[]?> _selectServiceFileInteraction;
    private string[]? _selectedServiceFiles;

    public OpenServiceViewModel()
    {
        _selectServiceFileInteraction = new Interaction<string?, string[]?>();
        SelectServiceFileCommand = ReactiveCommand.CreateFromTask(SelectFiles);
    }
    
    // [Reactive]
    public string[]? SelectedServiceFiles
    {
        get { return _selectedServiceFiles; }
        set { this.RaiseAndSetIfChanged(ref _selectedServiceFiles, value); }
    }

    public Interaction<string?, string[]?> SelectServiceFileInteraction => _selectServiceFileInteraction;

    public ICommand SelectServiceFileCommand { get; }

    private async Task SelectFiles()
    {
        SelectedServiceFiles = await _selectServiceFileInteraction.Handle("Select the service file...");
    }
}