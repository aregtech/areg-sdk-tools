using System.Reactive.Linq;
using System.Threading.Tasks;
using System.Windows.Input;
using AvaloniaUI.ViewModels.Msg;
using ReactiveUI;

namespace AvaloniaUI.ViewModels;

public class OpenServiceViewModel : ViewModelBase
{
    public OpenServiceViewModel()
    {
        OpenServiceFileInteraction = new Interaction<string?, string[]?>();
        SelectServiceFileCommand = ReactiveCommand.CreateFromTask(SelectFiles);
    }

    public Interaction<string?, string[]?> OpenServiceFileInteraction { get; }

    // ReSharper disable once UnusedAutoPropertyAccessor.Global
    public ICommand SelectServiceFileCommand { get; }

    private async Task SelectFiles()
    {
        var selectedServiceFiles = await OpenServiceFileInteraction.Handle("Select the service file...");

        if (selectedServiceFiles != null)
        {
            foreach (var file in selectedServiceFiles)
            {
                EventAggregator.GetEvent<OpenServiceFullPathMsg>().Publish(file);
            }
        }
    }
}