namespace AvaloniaUI.ViewModels;

/// <summary>
/// An interface for all Aspect View-Models.
/// </summary>
public interface IAspectViewModel
{
    ServiceAspectType ServiceAspectType { get; }
    
    /// <summary>
    /// A short description of what the Aspect does.
    /// </summary>
    string EditorDescription { get; }
}