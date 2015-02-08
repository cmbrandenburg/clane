# clane

_Clane_ is a C++ library for embedding an HTTP server. _Clane_'s API design
draws inspiration from the Go programming language's
[http](http://golang.org/pkg/net/http) package.

_Clane_ currently supports server-side HTTP development and includes built-in
request handler types to carry out request-routing and file-serving.
Applications may also implement custom handlers.

_Clane_ is licensed under the [Mozilla Public License
2.0](http://www.mozilla.org/MPL/2.0/).

## Requirements

* Linux
* GCC >= 4.4, <= 4.7
* C++11 (`-std=c++0x` or `-std=c++11`)
* Boost >= 1.49

## Getting started

In the source tree, the `examples/` directory contains three examples of
applications running as an HTTP server. The `examples/ajax/` directory contains
a web application serving a static page (`index.html`) with some dynamic content
via AJAX POST requests.

Build _Clane_ by running `./bootstrap && ./configure && make install`.

Build examples by running `./configure --enable-examples` when configuring. The
examples don't install anywhere; run them from within the source tree.

Build and run unit tests by running `make check`.

Run `make doc` in the source tree's root directory to generate API documentation
in the `doc/html/` directory.

Applications may use _Clane_ by including the header `<clane/clane.hpp>` and linking
against the library via `-lclane`. Applications must also link against
`-lboost_regex`, `-lboost_filesystem`, `-lboost_system`, and `-lpthread`.

Alternatively, _Clane_ is available as two source files that may be copied into
an application and built as part of the application. Build the amalgam by
running `make amalgam` from the source tree's root directory. Doing so will
create the two files: `amalgam/clane.cpp` and `amalgam/clane.hpp`. Applications
using the amalgam must link against `-lboost_regex`, `-lboost_filesystem`,
`-lboost_system`, and `-lpthread`.

