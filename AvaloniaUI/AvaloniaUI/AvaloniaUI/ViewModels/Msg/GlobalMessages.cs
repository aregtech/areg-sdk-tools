using Prism.Events;

namespace AvaloniaUI.ViewModels.Msg;

/// <summary>
/// This message is ticked upon an "Open Service" event.
/// The full file path is published.
/// </summary>
public class OpenServiceFullPathMsg : PubSubEvent<string>;

/// <summary>
/// This message is ticked upon a "Close Application" event.
/// </summary>
public class CloseApplication : PubSubEvent;

/// <summary>
/// This message is used for logging to the local logging window.
/// </summary>
public class LogMessage : PubSubEvent<string>;