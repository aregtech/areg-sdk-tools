using Prism.Events;

namespace AvaloniaUI.ViewModels.Msg;

/// <summary>
/// This message is ticked upon an "Open Service" event.
/// </summary>
public class OpenServiceMsg : PubSubEvent<string>;

/// <summary>
/// This message is ticked upon a "Close Application" event.
/// </summary>
public class CloseApplication : PubSubEvent;