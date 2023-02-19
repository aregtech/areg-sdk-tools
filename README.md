# The UI tools for AREG SDK

## Introduction

[AREG](https://github.com/aregtech/areg-sdk) is an interface-centric communication engine. The AREG SDK consists of communication engine and the multiple tools. Since AREG engine is interface-centric, i.e. it works with predefined interfaces, there is a need to craete GUI tool do desing at least interface and servicing components. This project is a GUI tool to desing ad visualize service interface files used by code generator to create source codes based on technology provided by AREG engine.

## How to clone

The AREG SDK tool application contains dependency of the thirdparty libraries and repositories, which are added as modules. To clone all sources perform following actions:

```bash
# Step 1: Create areg-sdk-tool directory, can be any name
$ mkdir areg-sdk-tools

# Step 2: Switch to that direcotory
$ cd ./areg-sdk-tools

# Step 3: Clone recursevely all sources, including dependencies that has availonia
$ git clone --recurse-submodules https://github.com/aregtech/areg-sdk-tools.git .
```

## Tools included

This UI Tool for AREG SDK includes following applications:
1. Service Interface designer;
2. Log viewer.

> 💡 This project is under development and not ready yet.
