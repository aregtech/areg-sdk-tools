# Third-Party Notices

## Areg SDK

Lusan is part of the Areg SDK project. Lusan itself and the Areg SDK
components it links against -- the `areg` and `areglogger` libraries, the
`aregextend` static library, `aregsqlite3`, and the `mtrouter`/`logcollector`
services -- are all released under the same Apache License 2.0 (see the
top-level `LICENSE` file). This is not a third-party dependency in the sense
of the rest of this document: there is no license conflict to manage here,
because everything in this group is the same license.

The sections below cover the components Lusan uses that come from outside
the Areg SDK project and carry their own, different licenses.

## Qt 6 (Qt Core, Qt Gui, Qt Widgets)

- License: GNU Lesser General Public License v3 (LGPLv3), or a commercial Qt
  license.
- Full text: `Qt-LGPLv3.txt` in this directory, or https://www.gnu.org/licenses/lgpl-3.0.txt
- How Lusan uses it: dynamically linked (shared libraries), deployed next to
  the `lusan` executable. Dynamic linking is what keeps this compatible with
  the LGPLv3 terms without needing a commercial Qt license -- see
  https://www.qt.io/licensing for the upstream terms.
- Copyright (C) The Qt Company Ltd. and other contributors.

## Qt Advanced Docking System (ADS)

- Project: https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System
- License: GNU Lesser General Public License v2.1 (LGPL-2.1)
- Full text: `Qt-Advanced-Docking-System-LGPL-2.1.txt` in this directory
- How Lusan uses it: dynamically linked (shared library), deployed next to
  the `lusan` executable for the same reason as Qt above -- LGPL-2.1 section
  6 requires either a shared-library link, relinkable object files, or a
  written offer for a statically-linked closed-source distribution; dynamic
  linking is the mechanism Lusan relies on.
- Copyright (C) the Qt Advanced Docking System contributors.

## SQLite3

- Project: https://www.sqlite.org
- License: Public domain.

## Distribution requirement

Any packaged or binary distribution of Lusan must include this directory, or
otherwise make the license texts above and this notice available to
recipients, alongside the deployed Qt and ADS shared libraries.
