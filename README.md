# The UI tools for AREG SDK

## Introduction

[AREG](https://github.com/aregtech/areg-sdk) is an interface-centric communication engine. The AREG SDK consists of communication engine and the multiple tools. Since AREG engine is interface-centric, i.e. it works with predefined interfaces, there is a need to craete GUI tool do desing at least interface and servicing components. This project is a GUI tool to desing ad visualize service interface files used by code generator to create source codes based on technology provided by AREG engine.

## How to clone

The AREG SDK tool application is base on Qt6 thirdparty libraries.

```bash
# Step 1: Create areg-sdk-tool directory, can be any name
$ mkdir areg-sdk-tools

# Step 2: Switch to that direcotory
$ cd ./areg-sdk-tools

# Step 3: Clone recursevely all sources, including dependencies that has availonia
$ git clone https://github.com/aregtech/areg-sdk-tools.git .
```

## Software build

**System Requirement**:
1. Qt6.8.1 or newer;
2. QtCreator 15 or newer;
3. CMake 3.20 or newer;
4. One of compilers:
   - GCC or CLang compilers under Linux
   - mingw GCC, mingw LLVM or MSVC compilers under Windows.

To build the software, you'd need Qt6.8 and the QtCreator IDE. Open `areg-sdk-tool` in the QtCreator => select Kit Configuration => run CMake => build application.

> [!NOTE]
> The detailed instruction to build the project with or withoud QtCreator will be update later.

## Tools included

This UI Tool for AREG SDK includes following applications:
1. Service Interface designer;
2. Log viewer.

> [!IMPORTANT]
> 💡 This project is under development and not ready yet for the use. Currently it is actively modified to create a `Service Interface` document. Welcome to join the project. Pick up any unassigned ticket you'd like to fix. Write a short message to assign the ticket to you. Since the application is very specific and designed for the `areg-sdk`, you'd have many questions. Feel free to ask any question.
