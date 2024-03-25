using System.Linq;
using System.Threading.Tasks;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Platform.Storage;
using Avalonia.ReactiveUI;
using Avalonia.Styling;
using AvaloniaUI.ViewModels;
using ReactiveUI;

namespace AvaloniaUI.Views;

public partial class MainView : ReactiveUserControl<MainViewModel>
{
    public MainView()
    {
        InitializeComponent();

        // For switching between Light & Dark themes:
        ThemeVariants.SelectedItem = Application.Current!.RequestedThemeVariant;
        ThemeVariants.SelectionChanged += (_, _) =>
        {
            if (ThemeVariants.SelectedItem is ThemeVariant themeVariant)
            {
                Application.Current.RequestedThemeVariant = themeVariant;
            }
        };

        this
            .WhenActivated(d =>
            {
                this.ViewModel?
                    .OpenServiceInteraction
                    .SelectServiceFileInteraction
                    .RegisterHandler(this.InteractionHandler);
            });
    }
    
    private async Task InteractionHandler(InteractionContext<string?, string[]?> context)
    {
        // Get our parent top level control in order to get the needed service (in our sample the storage provider. Can also be the clipboard etc.)
        var topLevel = TopLevel.GetTopLevel(this);
        
        var storageFiles = await topLevel!.StorageProvider
            .OpenFilePickerAsync(
                new FilePickerOpenOptions
                {
                    AllowMultiple = true,
                    Title = context.Input,
                    FileTypeFilter = new FilePickerFileType[]
                    {
                        new("Service Interfaces")
                        {
                            Patterns = new[] { "*.siml" }
                        }
                    }
                });
               
        context.SetOutput(storageFiles.Select(x => x.Name).ToArray());
    }
}