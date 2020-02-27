 # Process-Monitoring
Simple Process Monitoring Tool

# Usage

### pmcli usage
| Option   | Long           | Description                                       |
|:-------- |:-------------- |:------------------------------------------------- |
| -h or -? | --help         | produce help message                              |
| -v       | --version      | print version string                              |
| -o       | --output       | output file name                                  |
| -i       | --interval     | interval in ms (default 60000)                    |
| -i       | --interval     | interval in ms (default 60000)                    |
| -p       |  --process-id  | monitoring process id (multiple separated by ,)   |
| -n       | --process-name | monitoring process name (multiple separated by ;) |
| -t       | --type         | memory type (see list below)                      |

### Types
| Abbreviation   | Type                            | Description  |
|:-------------- |:------------------------------- |:------------ |
| pfc            | Page fault count                |              |
| pwss           | Peak working set size           |              |
| wss            | Working set size                | default type |
| qpppu          | Quota peak paged pool usage     |              |
| qppu           | Quota paged pool usage          |              |
| qpnppu         | Quota peak non paged pool usage |              |
| qnppu          | Quota non paged pool usage      |              |
| pfu            | Page file usage                 |              |
| ppfu           | Peak page file usage            |              |

### Examples
pmcli --process-id 1234,5678
pmcli --process-name a.exe;b.exe;pmcli.exe
pmcli --interval 120000 --process-id 1234,5678 --process-name a;b;pmcli

# Build

## Dependencies
[CMake](https://www.cmake.org)

## Process

### Create a build folder
```cmake -E make_directory <new-build-path>```
Example
```cmake -E make_directory "C:\build\pm"```
### Create a build
```cmake -E chdir <path-to-build> cmake -G <generator-name> <path-to-source>```
Example
```cmake -E chdir "C:\build\pm" cmake -G "Visual Studio 16 2019" "C:\src\pm"```
To install into specified folder
```cmake -E chdir "C:\build\pm" cmake -DCMAKE_INSTALL_PREFIX:PATH="C:\install\pm" -G "Visual Studio 16 2019" "C:\src\pm"```
### Build
```cmake --build <path-to-build> --target <target> --config <configuration>```
Example
```cmake --build C:\build\pm --target ALL_BUILD --config RelWithDebInfo```
### Install
```cmake --build <path-to-build> --target INSTALL --config <configuration>```
Example
```cmake --build C:\build\pm --target INSTALL --config RelWithDebInfo```
# Release dependencies
Visual C Redistributable 2019

# To do
Nix support

# Release dependencies
Visual C Redistributable 2019

# To do
Nix support
