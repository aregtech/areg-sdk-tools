using System.Reactive.Linq;
using System.Threading.Tasks;
using System.Windows.Input;
using AvaloniaUI.ViewModels.Msg;
using ReactiveUI;

namespace AvaloniaUI.ViewModels;

public class OpenServiceViewModel : ViewModelBase
{
    private string[]? _selectedServiceFiles;
    
    private readonly ObservableMessageProducer<OpenServiceMsg> _openServiceMessageProducer;

    public OpenServiceViewModel()
    {
        _openServiceMessageProducer =
            AppContainer.Instance.GetInstance<ObservableMessageProducer<OpenServiceMsg>>();

        OpenServiceFileInteraction = new Interaction<string?, string[]?>();
        SelectServiceFileCommand = ReactiveCommand.CreateFromTask(SelectFiles);
    }

    // [Reactive]
    public string[]? SelectedServiceFiles
    {
        get => _selectedServiceFiles;
        set => this.RaiseAndSetIfChanged(ref _selectedServiceFiles, value);
    }

    public Interaction<string?, string[]?> OpenServiceFileInteraction { get; }

    // ReSharper disable once UnusedAutoPropertyAccessor.Global
    public ICommand SelectServiceFileCommand { get; }

    private async Task SelectFiles()
    {
        SelectedServiceFiles = await OpenServiceFileInteraction.Handle("Select the service file...");

        foreach (var file in SelectedServiceFiles!)
        {
            _openServiceMessageProducer.NextMessage(new OpenServiceMsg(file));
        }
    }
}