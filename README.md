# lot
**A faster replacement for vector, due to element construction and destruction on memory reserveration and freeing, rather than on element insertion and deletion. It also allows enabling or disabling range checks independent from compiler debugging settings, and using 32-bit indices on 64-bit system.**

### Requirements

- C++11
- [CMake](https://cmake.org/) (only necessary for running the test)

### Installation

- *lot* is header only. Therefore, simply copy ```includes/mz/lot.h``` into a directory of your choice, and ```#include``` it.
- The namespace is ```std::mz```.

### Tested on

| [Linux][lin-link] | [Windows][win-link] | [Code-coverage][cov-link] |
| :---------------: | :---------------: | :---------------: |
| ![lin-badge]      | ![win-badge]      | ![cov-badge]      | 

[lin-badge]: https://travis-ci.org/HighDefinist/lot.svg?branch=master "Travis build status"
[lin-link]:  https://travis-ci.org/HighDefinist/lot "Travis build status"
[win-badge]: https://ci.appveyor.com/api/projects/status/94h1yhhob3upshju/branch/master?svg=true "AppVeyor build status"
[win-link]:  https://ci.appveyor.com/project/HighDefinist/lot/branch/master "AppVeyor build status"
[cov-badge]: https://codecov.io/gh/HighDefinist/lot/branch/master/graph/badge.svg "Code coverage status"
[cov-link]:  https://codecov.io/gh/HighDefinist/lot/branch/master "Code coverage status"

- Visual Studio 2015 or newer
- GCC 5 or newer
- Clang 4 or newer
- XCode 6.4 or newer
 
### Download 

You can download the latest version of *lot* by cloning the GitHub repository:

	git clone https://github.com/HighDefinist/lot.git
	
### Usage

Take a look at the test file ```test/slot/lot.cpp``` or the header file itself. In general, it supports most methods of ```vector```, including:

- optimized copies, assignments, movement constructors and initializer lists
- auto iterators
- basic functions like clear, reserve, capacity, push_back, etc...

On top of the regular ```vector```, the following methods are also available

- ```Add(*various*)```: A synonym for push_back, but it supports multiple arguments, so multiple elements can be inserted at the same time. It also supports adding another ```lot<>```, by appending all entries.
- ```Take(lot& other)```: Append all elements of ```other```, and clears ```other```.

Unsupported ```vector``` methods:

- insert, emplace, erase
- lexicographic comparisons (==, !=, <, <=, >, >=)
