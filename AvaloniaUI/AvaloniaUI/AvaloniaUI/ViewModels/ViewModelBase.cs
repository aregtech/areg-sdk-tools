using Prism.Events;
using ReactiveUI;

namespace AvaloniaUI.ViewModels;

public abstract class ViewModelBase : ReactiveObject
{
    protected IEventAggregator EventPublisher { get; } = AppContainer.Instance.GetInstance<IEventAggregator>();
}