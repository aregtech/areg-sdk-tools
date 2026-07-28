# State Machine Designer

The State Machine Designer is Lusan's visual editor for finite state machines targeting the
Areg SDK. A machine is stored in a single `.fsml` document -- an XML file that follows the
same conventions as the `.siml` service interface format, so the two editors feel alike and
the same data types are declared the same way in both.

A `.fsml` document describes **one sequential machine**: its declarations (data types,
attributes, events, timers, methods, constants, includes), its state graph over any number
of nested levels, and the diagram layout. Nothing about the running system is stored in it.

---

## Opening and creating documents

- **File -> New -> State Machine** creates an empty machine with one `Start` state.
- **File -> Open** accepts `*.fsml`; double-clicking a `.fsml` in the workspace tree opens it
  in the designer.
- Saving is atomic: the document is written to a temporary sibling and renamed over the
  target, so an interrupted save never truncates the original.
- While a document has unsaved changes, Lusan writes a `<document>.fsml.autosave` sibling
  every 30 seconds. If that file is newer than the document when you next open it, Lusan
  offers **Restore**, **Discard** or **Cancel**. A clean save deletes the autosave.

---

## The document pages

The editor has eight tabs, mirroring the service interface editor:

| Page | Holds |
|---|---|
| **Overview** | Machine name, user version, threading mode (`Local` or `Shared`), description |
| **Data Types** | Enumerations, structures, containers and imported types -- identical semantics to `.siml` |
| **Attributes** | The machine's data members: name, type, initial value |
| **Events** | Machine-global events with typed payload parameters, and timers with timeout and repeat count |
| **Methods** | Triggers (external stimuli), Actions (things the machine does), Conditions (boolean tests, either handler-implemented or embedded C++) |
| **Constants** | Named typed values usable anywhere a value is expected |
| **Includes** | One list under three headings: `Include Files` (C++ headers the generated code must include), `Data Types` (`.dtml` documents), and `State Machines` (imported `.fsml` submachines, with their alias and pinned version). The heading a file lands under follows its extension, so `Add` never asks what kind it is |
| **Design** | The state graph canvas |

Triggers, events and timers share **one name space** -- a transition names its stimulus and
the `StimulusKind` attribute says which registry to look it up in, so the same name may not
be used twice across the three.

---

## The design canvas

### Tools

Toolbar, the **Design** menu and the canvas context menu all drive the same actions, so
hiding the toolbar (**Design -> Show Toolbar**) removes no functionality.

| Tool | Shortcut | Does |
|---|---|---|
| Select | `Esc` | The default mode; rubber-band selection, move, resize |
| Add State | `S` | Place a `Normal` state |
| Add Final State | `F` | Place a `Final` state |
| Add Transition | `T` | Drag from the source state to the target; drop on empty space cancels |
| Add Note | `N` | Place a free-text annotation |

Holding `Ctrl` when a placement tool completes keeps the tool armed for the next placement.
The armed tool is shown checked in all three surfaces at once.

### Navigation

- Double-click a composite state, or press `Enter` on it, to descend into its level. A state
  that hosts an *imported* machine opens that file instead -- see "Adding a submachine".
- `Backspace`, `Alt`+double-click, or a breadcrumb segment goes back up.
- `Home` centres the machine; `Ctrl`+`+` / `Ctrl`+`-` / `Ctrl`+`0` zoom; `Ctrl`+`Shift`+`0`
  zooms to fit.
- Each level keeps its own zoom and scroll position, saved with the document.

### Editing

- `F2` renames the selected element in place. Renaming a state is safe: transitions
  reference their target by element ID, never by name, so no connection is rewritten.
- `Delete` removes the selection, including a composite's whole subtree, its incoming
  transitions and its layout, as one undo step.
- `Ctrl+C` / `Ctrl+X` / `Ctrl+V` / `Ctrl+D` copy, cut, paste and duplicate; pasted elements
  get fresh IDs and a `Name_<id>` suffix when the name would collide.
- Alignment and distribution act on the selection.
- `Ctrl+Shift+G` toggles snap to grid; grid size and visibility are saved with the document.
- Every gesture is one undo step. Undo history survives page switches.

### Adding a submachine

A state can own a submachine in exactly one of two ways, and never both at once. Which one
you want decides how you add it.

The **Properties -> General** header carries the four controls for all of it: one button that
enters, opens or creates a submachine depending on what is selected, `Go to Parent`,
`Add Submachine...` and `Remove Submachine`. The first one renames itself so you can see what
pressing it will do -- `Add Substate` on a plain state, `Enter Submachine` on a painted one,
`Open Imported Machine` on a state that hosts an imported file.

**Painted -- the submachine lives in this document.** Select a `Normal` state and press
`Add Substate`, double-click its body, or press `Enter`. The editor attaches the nested level,
creates its mandatory `Start` state and descends, all as one undo step. From then on the state
is composite: `Enter` descends, `Backspace` comes back up, and `Delete` on the state removes
the whole subtree. `Add Substate (Painted)` in the canvas context menu is the same operation
under its explicit name.

**Imported -- the submachine is another `.fsml` file.** Press `Add Submachine...`, pick the
file, and that is the whole gesture: the machine is registered on the **Includes** page under
an *alias* (the file's base name by default, editable) with its location and the version it
had at that moment, and if the selected state is a plain `Normal` state the alias is linked to
it in the same undo step. One `Ctrl+Z` reverses both.

You can also register a machine from the Includes page directly: `Add` creates an entry under
`Include Files`, and the moment you browse to a `.fsml` or type one into `Location` the row moves
to `State Machines` and grows its alias and pinned version. The extension is what decides, so
nothing has to be declared. Then pick the alias in **Properties -> General -> Submachine**; the
picker is disabled for `Start` and `Final` states and for a state that already has a painted
subtree.

A file is registered **once**, and that one registration can be hosted by any number of
states; each is an independent instance. Picking a file that is already in the list links the
existing alias instead of adding a second one.

The location is stored relative to *this document's own directory*, so the host has to be
saved before it can import anything -- the editor says so rather than storing a path that
cannot be resolved later.

An imported machine is **sealed**. The host stores only the alias, the location and the pinned
version -- it never reaches inside. The one signal the submachine sends back is the
**On Final** event: set it in Properties and the host raises that event when the imported
machine reaches its top-level `Final` state. Because the import is a separate document,
double-clicking a hosting state does not descend -- it opens that file **read-only** in its own
window, with the origin shown in the breadcrumb. Edit it by opening it as its own document.

**Switching form.** The two forms are mutually exclusive, so press `Remove Submachine` before
setting the other one. On an imported machine it unlinks the alias and clears `On Final` and
`History` in the same undo step; the registration and the file itself stay. On a painted one it
deletes the whole nested subtree -- every level of it, the layout of everything in it, and every
transition from outside that pointed into it. It asks first, naming the counts, and one
`Ctrl+Z` brings all of it back with the same IDs and the same geometry.

**Limits.** Imported machines may nest at most **10** levels of imports, and may never form a
cycle -- a machine cannot import itself, directly or through a chain. Both are refused when you
pick the file, with the offending chain in the message, rather than reported afterwards.
Painted nesting has **no** depth limit: it carries no cross-document reference and cannot form
a cycle.

**When an import breaks.** A missing, unreadable or cyclic import is an error on the
registration *and* on every state that hosts it, but the host document still opens -- a broken
import never costs you your own work. A file that cannot be parsed is still registered, with a
warning, because it may be repaired later. Version drift is graded: a differing **major**
version is an error, **minor** is a warning, **patch** is information, and only the **Update**
button re-pins.

### Panels

- **Outline** (left) -- the hierarchy plus the registries; double-click navigates.
- **Properties** (right) -- editors for the selected element, including the condition
  builder, the parameter-mapping grid and the operations editor. A state's transitions are
  listed here and can be dragged to change their priority.
- **Validation** -- a tab of the global output window, rebound to the active document.
  `F8` and `Shift+F8` step through the findings; activating one navigates to the element.

---

## Behaviour: transitions, guards and operations

A **transition** belongs to its source state and names a stimulus. It is:

- **external** when it has a target -- the machine leaves the source state and enters the
  target;
- **internal** when it has no target -- the operations run, the active state does not change.

Transitions on a composite state fire while any of its descendants is active. Within one
state, **document order is priority order** -- the first transition whose stimulus matches
and whose guard passes is the one that fires.

Entering a composite state normally descends to its `Start` state. A composite can instead be
told to resume where it left off, with `History="Shallow"` (the last direct substate) or
`History="Deep"` (the whole last active path). Set it from the state's Properties, its context
menu or the **Design** menu; the canvas marks it with an `H` or `H*` badge in the state's
header. See [`fsm-history.md`](./fsm-history.md) for the rules and a worked example.

A **guard** is an optional boolean expression built in the condition builder from
comparisons, condition-method calls, attributes, constants, stimulus parameters, literals
and, where structure is not enough, verbatim C++ (a lambda or a raw expression). References
are bound by element ID, so renames never break a guard.

**Operations** are the typed instructions a state's entry / exit / do list or a transition
runs, in list order:

| Operation | Effect |
|---|---|
| `ActionCall` | Calls an Action method, with each parameter mapped to a value source |
| `AttributeSet` | Assigns to an attribute from a value, another declaration or a verbatim expression |
| `TimerStart` / `TimerStop` | Starts or stops a timer, optionally overriding timeout and repeat |
| `EventSend` | Raises a machine event with its payload mapped |
| `InlineCode` | Verbatim C++ emitted at that point |

A state's **do activity** additionally carries a repeat interval and a stop condition.

Every value slot accepts the same set of sources: a literal `Value`, a stimulus `Param`, an
`Attribute`, a `Constant`, a `Condition` result, a verbatim `Expression`, or a `Lambda`.
A parameter that declares a default may be left unmapped; one that does not, may not.

---

## Validation

Validation runs continuously in the background and never blocks editing. Findings appear in
the **Validation** tab with three severities:

- **Errors** must be fixed -- missing or duplicated `Start` states, duplicate IDs or names,
  unresolved references, targets that are not siblings, `Final` states with outgoing
  transitions, unmapped required arguments, type incompatibilities, unparseable literals,
  malformed expression rows, imports that cannot be resolved, that form a cycle or that nest
  more than ten levels of imports, an imported `.fsml` with no alias for a state to name, a
  pinned import version whose major component has moved, and so on.
- **Warnings** are design smells -- unreachable states, dead ends, shadowed transitions,
  declarations nothing references (an imported machine no state hosts included), the same file
  included more than once, events sent but never reacted to, timers reacted to but never
  started, empty internal transitions, comparisons of two constants, empty inline code, a
  pinned import version whose minor component has moved.
- **Information** covers declarations that are simply not used yet: an unused attribute or
  constant is legitimate at any point in a design and is never reported as a warning.

The code generator is a separate track and is not part of this version. When it ships it
re-runs the same validation headlessly and refuses to generate while any error remains, so a
document that is clean here is a document it will accept.

---

## The document format

`.fsml` is plain XML and is meant to be diff-friendly and merge-friendly:

- Element and attribute order is fixed by the writer, never by a map or by reflection, so
  saving an unchanged document reproduces it **byte for byte**.
- Layout lives in one trailing `<Layout>` section keyed by element ID. Moving boxes around
  never touches the logical part of the file, and editing logic never touches the layout --
  the two are independently diffable.
- Code bodies and expressions are stored in CDATA exactly as typed: no trimming, no
  reindenting, no normalization.
- Every element carries a document-unique numeric `ID` from one monotonic counter. IDs are
  never reused and never renumbered, and every reference between elements uses the ID.
- `FormatVersion` on the root governs compatibility; the current version is **1.1.0**. An older
  document is migrated in memory on open and the file is left untouched until you save. A
  document written by a newer **major** version is refused, unmodified. A newer minor or patch
  version opens, and content the current version does not recognize is preserved and written
  back unchanged.
- 1.1.0 folded machine imports into the include list: a `1.0.0` document's `<ImportList>` is
  read and each `<MachineImport>` becomes a `<Location>` carrying `Alias` and `Version`. This is
  the one case where saving does not reproduce the file byte for byte -- the migration is
  one-way, and the second save is byte-identical again.

A schema for editors and validators is available at `.claude/fsml.xsd`, and
`.claude/FullFeature.fsml` is a reference document that exercises every element the format
defines.

---

## Not in this version

By design, the format and the editor do **not** cover:

- parallel (orthogonal) regions -- model concurrency with several machine instances;
- choice pseudo-states -- branch with real states and prioritized conditional transitions;
- a parsed expression language -- verbatim slots are stored and emitted, never interpreted;
- SCXML or other interchange formats;
- framework independence -- the generated machine targets the Areg SDK.
