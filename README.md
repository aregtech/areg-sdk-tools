# Areg SDK Tools: *Lusan Application*

Lusan is the official UI toolset for the [Areg SDK](https://github.com/aregtech/areg-sdk).  
It is the primary development and diagnostics companion for building, debugging, and operating Areg-based distributed applications.

Areg is interface-centric by design. Lusan turns that model into a **concrete, visual, and controllable workflow** — from interface definition to runtime diagnostics.

---

## Why Lusan Is Essential

Areg-based applications are inherently **distributed, multi-threaded, and multi-process**, often running across multiple machines.  
In such systems:

- Service interfaces define **runtime behavior**, not just APIs.
- Bugs are often **timing-dependent**, sometimes **non-reproducible** and **invisible** to traditional debuggers.
- Logs from a single process are meaningless without system-wide context.

**Lusan exists because developing Areg applications without it is inefficient, fragile, and error-prone.**

Lusan provides a unified visual workspace where developers can:

- **Design and reason about service interfaces before writing code**
- **Observe and control logging across multiple running processes and machines**
- **Analyze offline logs to debug failures and measure real performance**

Even when used *only* for logging, **Lusan still requires integration of the Areg library**.  
This is intentional: Areg’s logging system is tightly coupled with its runtime model and enables capabilities that generic log viewers cannot provide.

💡 **Use Lusan when:** developing or debugging any Areg-based system on Linux or Windows  
⚠️ **Do not use Lusan when:** working with non-Areg applications

> [!NOTE]
> Lusan is **not a general-purpose log viewer**. It relies on the Areg SDK’s built-in logging system. If your application does not use Areg’s communication engine, you must still integrate the Areg library and enable logging (while disabling unused features) to use Lusan.  
> **Example** of integrating Areg logging without using other Areg features: [**Areg Logging Only Example**](https://github.com/aregtech/areg-sdk/blob/master/examples/07_logging/src/main.cpp).

---

## Key Features

### 1. Visual Service Interface Designer

- Create and edit Areg service interface definition files
- Visualize **data types, requests, responses, broadcasts, and attributes**
- Validate interface consistency before code generation

**This eliminates manual interface maintenance and prevents structural errors early.**

---

### 2. Multi-Instance Real-Time Log Viewer & Analyzer

- **Real-time log aggregation** from multiple processes and machines
- **Runtime filtering by scope, priority, module, and source**
- Dynamic log-level control applied *on target processes*
- Designed for debugging **concurrent and distributed execution flows**

**Logs are filtered at the source, not after the fact.**

---

### 3. Offline Log Viewer & Performance Analyzer

- Open and inspect saved `.sqlog` files
- Apply advanced filters and text search
- Measure execution timing and performance characteristics
- Analyze failures that cannot be reproduced live

---

### 4. Live Log Collector Integration

To enable live logging:

- Applications must be built with the **Areg library** with enabled logging (see example [**logging**](https://github.com/aregtech/areg-sdk/blob/master/examples/07_logging/src/main.cpp) of Areg SDK)
- A `logcollector` process of Areg SDK must be running (console or system service)
- A `lusan.init` file defines how Lusan connects to the collector
- Target processes use `areg.init` to define initial logging scopes and priorities

Once `logcollector` is available and `lusan` is connected:

- Lusan immediately receives logs from all connected processes
- Developers can dynamically change logging scopes and priorities
- Changes can be persisted back into `areg.init` for future runs

This enables **system-wide logging control without restarting applications**.

---

## Getting the Sources

Lusan is built with **Qt 6.x**.

Create a directory (any name)  
```bash
mkdir areg-sdk-tools
cd areg-sdk-tools
```

Clone the repository
```bash
git clone https://github.com/aregtech/areg-sdk-tools.git .
````

---

## Build Instructions

### Requirements

* **Qt 6.x**
* **QtCreator 15+**
* **CMake 3.20+**
* **Compilers:** MSVC, GCC, Clang, MinGW

### Build with QtCreator

1. Open `areg-sdk-tools` in **QtCreator**
2. Select a Qt 6.x Kit
3. Configure CMake
4. Build the project

> [!NOTE]
> Command-line and advanced build workflows will be documented separately.

---

## Included Tools

* **Service Interface Designer**
  Visual design and validation of Areg service interfaces

* **Multi-Instance Real-Time Log Viewer & Analyzer**
  Live log streaming, filtering, and runtime control

* **Multi-Instance Offline Log Viewer & Analyzer**
  Load `.sqlog` files, analyze failures, and measure performance

---

## License

The sources of Lusan are released under the **MIT License**. It uses sources of Areg SDK, which are released under the **Apache License 2.0**. 
You are free to use, modify, and distribute the software in accordance with the license terms.

---

## Call to Action

* 🐞 **Found a bug?** Open an issue.
* 💡 **Have an idea?** Start a discussion.
* 🔧 **Want to contribute?** Pick an unassigned issue and leave a short comment before starting.
* 🚀 **Building with Areg?** Use Lusan — it is the fastest way to stay in control of your system.
* 🌟 **Liked Lusan or Areg SDK project?** Star [Areg SDK](https://github.com/aregtech/areg-sdk/) and [Lusan](https://github.com/aregtech/areg-sdk-tools/) to help us to increase the community.

Contributions and feedback are welcome and highly appreciated.
