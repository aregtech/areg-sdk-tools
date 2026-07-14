# CONTRIBUTING TO Areg SDK Tools (Lusan)

Thank you for your interest in contributing to Lusan, the GUI tool for the Areg SDK, part of the Areg SDK Tools open-source repository.  
We welcome developers, companies, researchers, and hobbyists who want to help improve the tool.

This guide explains how to contribute, how copyrights work, and what you need to do so your contribution can be accepted.

---

## 1. Code Licensing

All contributions to this repository (Areg SDK Tools, including the Lusan open-source edition) are licensed under:

**Apache License, Version 2.0**

By submitting a pull request or commit, you agree that your contribution is provided under Apache 2.0.

---

## 2. Developer Certificate of Origin (DCO)

To contribute, you must confirm that you have the right to submit the code.

Areg SDK uses the **Developer Certificate of Origin (DCO)**. It is a simple and widely adopted alternative to contributor license agreements.

Every commit must include a `Signed-off-by` line:

```

Signed-off-by: Your Name [email@example.com](mailto:email@example.com)

```

Most Git tools can add this automatically:

```

git commit -s

```

By signing off, you state that your contribution is your original work or that you have permission to submit it.  
More information can be found at https://developercertificate.org/

---

## 3. Copyright Headers

You may add a copyright header if you want your name or organization to appear in the source file.

If you prefer not to add a header, the project may include a default header for clarity and consistent licensing.  
This does not affect your ownership or your rights in any way.

Example of an optional header:

```cpp
/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright  (c) 2023-2026 Your Name or Aregtech (Artak Avetyan).
 *  \file       lusan/data/sm/SMAttributeData.cpp
 *  \ingroup    Lusan - GUI Tool for Areg SDK
 *  \author     Your Full Name or GitHub ID
 * \brief       Brief description
 *
 ************************************************************************/
```

Minor edits such as typo fixes, grammar corrections, formatting cleanup, comment updates, CMake changes, and YAML updates do not require contributor copyright headers.

> [!NOTE]
> Contributors retain copyright in their own contributions. Under the DCO (Section 2) and the Apache License 2.0 grant, which permits sublicensing and use in proprietary Derivative Works, maintainers may incorporate contributions into both the open-source edition of Lusan and the separate closed-source commercial edition described in Section 4. No additional grant or action from contributors is required.

---

## 4. Dual Licensing and Commercial Use

Areg SDK Tools, including Lusan, is licensed under the Apache License 2.0 in this repository. You retain ownership of your contribution, which will always remain available here under Apache 2.0.

A separate, closed-source commercial edition of Lusan exists in a private repository, built on top of this open-source codebase (via submodule or direct integration, not a fork). The Apache License 2.0 already permits this without any additional agreement: Section 2 grants an irrevocable, sublicensable copyright license, and Section 4 permits distribution as part of Derivative Works, including proprietary ones.

By contributing under the DCO and Apache 2.0, you confirm you understand that your contribution may be incorporated into both the open-source and the commercial editions of Lusan.

---

## 5. Contribution Guidelines

### a. Reporting Bugs

When reporting an issue, please include:

* steps to reproduce
* expected and actual behavior
* platform and compiler information

### b. Submitting Code

Pull requests should follow these rules:

* one clear improvement per pull request
* include tests when possible
* include a `Signed-off-by` line in every commit

### c. Documentation

Improvements to guides, examples, comments, and general documentation are welcome.
Small corrections only require a Signed-off-by line.

---

## 6. Areas Where Help Is Needed

1. C++17 development such as core framework, features, and services
2. Unit tests and example applications
3. Build systems and cross compilation setups
4. UI and UX improvements for the Areg SDK Tools
5. Technical writing including documentation and guides

---

## 7. Code of Conduct

Always communicate respectfully and constructively.
We strive to maintain a welcoming community for everyone.

---

## 8. Questions

If you have any questions about licensing, rights, or the contribution process, open an issue or start a discussion.

We are happy to help you get started.
