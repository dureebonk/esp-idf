# Build System
This document explains the implementation of the ESP-IDF build system and the concept of "components". Read this document if you want to know how to organize and build a new ESP-IDF project or component.

## Overview
An ESP-IDF project can be seen as an amalgamation of a number of components. For example, for a web server that shows the current humidity, there could be:

The ESP-IDF base libraries (libc, ROM bindings, etc)

* The Wi-Fi drivers
* A TCP/IP stack
* The FreeRTOS operating system
* A web server
* A driver for the humidity sensor
* Main code tying it all together

ESP-IDF makes these components explicit and configurable. To do that, when a project is compiled, the build system will look up all the components in the ESP-IDF directories, the project directories and (optionally) in additional custom component directories. It then allows the user to configure the ESP-IDF project using a text-based menu system to customize each component. After the components in the project are configured, the build system will compile the project.

## Concepts
A **project** is a directory that contains all the files and configuration to build a single app (executable), as well as additional supporting elements such as a partition table, data partitions or filesystem partitions, and a bootloader.

Project configuration is held in a single file called **sdkconfig** in the root directory of the project. This configuration file is modified via idf.py menuconfig to customize the configuration of the project. A single project contains exactly one project configuration.

An **app** is an executable that is built by ESP-IDF. A single project will usually build two apps - a "project app" (the main executable, ie your custom firmware) and a "bootloader app" (the initial bootloader program which launches the project app).

**Components** are modular pieces of standalone code that are compiled into static libraries (.a files) and linked to an app. Some are provided by ESP-IDF itself, others may be sourced from other places.

**Target** is the hardware for which an application is built. A full list of supported targets in your version of ESP-IDF can be seen by running idf.py --list-targets.

Some things are not part of the project:

**ESP-IDF** is not part of the project. Instead, it is standalone, and linked to the project via the IDF_PATH environment variable which holds the path of the esp-idf directory. This allows the ESP-IDF framework to be decoupled from your project.

The **toolchain** for compilation is not part of the project. The toolchain should be installed in the system command line PATH.

## Using the Build System
### idf.py
The idf.py command-line tool provides a front-end for easily managing your project builds. It manages the following tools:

## Example Project
An example project directory tree might look like this:

```text
- myProject/
             - CMakeLists.txt
             - sdkconfig
             - dependencies.lock
             - bootloader_components/ - boot_component/ - CMakeLists.txt
                                                        - Kconfig
                                                        - src1.c
             - components/ - component1/ - CMakeLists.txt
                                         - Kconfig
                                         - src1.c
                           - component2/ - CMakeLists.txt
                                         - Kconfig
                                         - src1.c
                                         - include/ - component2.h
             - managed_components/ - namespace__component-name/ - CMakelists.txt
                                                                - src1.c
                                                                - idf_component.yml
                                                                - include/ - src1.h
             - main/       - CMakeLists.txt
                           - src1.c
                           - src2.c
                           - idf_component.yml
             - build/
```

This example "myProject" contains the following elements:

* A top-level project CMakeLists.txt file. This is the primary file which CMake uses to learn how to build the project; and may set project-wide CMake variables. It includes the file /tools/cmake/project.cmake which implements the rest of the build system. Finally, it sets the project name and defines the project.

* "sdkconfig" project configuration file. This file is created/updated when idf.py menuconfig runs, and holds the configuration for all of the components in the project (including ESP-IDF itself). The sdkconfig file may or may not be added to the source control system of the project. More information about this file can be found in the sdkconfig file section in the Configuration Guide.

* "dependencies.lock" file contains the list of all managed components, and their versions, that are currently in used in the project. The dependencies.lock file is generated or updated automatically when IDF Component Manager is used to add or update project components. So this file should never be edited manually! If the project does not have idf_component.yml files in any of its components, dependencies.lock will not be created.

* Optional "idf_component.yml" file contains metadata about the component and its dependencies. It is used by the IDF Component Manager to download and resolve these dependencies. More information about this file can be found in the idf_component.yml section.

* Optional "bootloader_components" directory contains components that need to be compiled and linked inside the bootloader project. A project does not have to contain custom bootloader components of this kind, but it can be useful in case the bootloader needs to be modified to embed new features.

* Optional "components" directory contains components that are part of the project. A project does not have to contain custom components of this kind, but it can be useful for structuring reusable code or including third-party components that aren't part of ESP-IDF. Alternatively, EXTRA_COMPONENT_DIRS can be set in the top-level CMakeLists.txt to look for components in other places.

* **"main" directory** is a special component that contains source code for the project itself. "main" is a default name, the CMake variable COMPONENT_DIRS includes this component but you can modify this variable. See the renaming main section for more info. If you have a lot of source files in your project, we recommend grouping most into components instead of putting them all in "main".

* "build" directory is where the build output is created. This directory is created by idf.py if it doesn't already exist. CMake configures the project and generates interim build files in this directory. Then, after the main build process is run, this directory will also contain interim object files and libraries as well as final binary output files. This directory is usually not added to source control or distributed with the project source code.

* "managed_components" directory is created by the IDF Component Manager to store components managed by this tool. Each managed component typically includes a idf_component.yml manifest file defining the component's metadata, such as version and dependencies. However, for components sourced from Git repositories, the manifest file is optional. Users should avoid manually modifying the contents of the "managed_components" directory. If alterations are needed, the component can be copied to the components directory. The "managed_components" directory is usually not versioned in Git and not distributed with the project source code.

Component directories each contain a component CMakeLists.txt file. This file contains variable definitions to control the build process of the component, and its integration into the overall project. See Component CMakeLists Files for more details.

Each component may also include a Kconfig file defining the component configuration options that can be set via menuconfig. Some components may also include Kconfig.projbuild and project_include.cmake files, which are special files for overriding parts of the project.
