Summary
-------
A userspace service that provides access to the openWebOS database.
This is a fork of DB8 project: https://github.com/openwebos/db8

Description
-----------
InDB8 is a userspace service that provides access to the webOS database.  Access to the database APIs is provided over the luna-service bus.  This initial release provides the infrastructure code to wrap a range of database engines.

Major changes
----------
No dependecy from openwebos cmake module
Removed dependency from media stuff
Add compatibility with other services
Remove pmloglib logging engine, use only internal engine
Move bug-fixes from db8 repository

How to Build on Linux
=====================

## Building for Gentoo

Current InDB8 supported by gentoo WebOS layer.
To install it via layman, add WebOS layer: 
    
    # layman -A webos

And install from layman:
    
    # emerge -av indb8

Gentoo will automatically install indb8 upstart script and all required dependency.

## Building using OpenEmbedded 

For building under webos, you can follow the same steps as for db8 (original fork). See: http://github.com/openwebos/db8

### Dependencies

Below are the tools and libraries (and their minimum versions) required to build this package:

* openwebos/cmake-modules-webos 1.0.0 RC7
* cmake (version required by openwebos/cmake-modules-webos)
* libdb-4.8-dev
* libicu-dev
* glib-2.0
* gthread-2.0
* openwebos/luna-service2

### Configure database backend

InDB8 support different database backends. To select db8 backend, set <tt>cmake</tt> variable <tt>WEBOS_DB8_BACKEND</tt> to one of supported database
backends. Currently support database backends are:

* leveldb
* berkeleydb
* sandwich

For example: 

    $ cmake -D WEBOS_INSTALL_ROOT:PATH=$HOME/projects/openwebos -D WEBOS_DB8_BACKEND:STRING=leveldb ..

By default, db8 compiles with sandwich database backend.

### Enable unit tests

To build the unit tests for db8, specify a true value for `WEBOS_CONFIG_BUILD_TESTS` on the cmake command line. For example:

    $ cmake -D WEBOS_CONFIG_BUILD_TESTS:BOOL=TRUE

### Building Standalone

#### Using cmake

Once you have downloaded the source, execute the following to build it (after changing into the directory under which it was downloaded):

    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
    $ sudo make install

### Debug build

To build InDB8 in debug mode, add to cmake param -DCMAKE_BUILD_TYPE=debug

    $ cmake -DCMAKE_BUILD_TYPE=debug ..

#### Uninstalling

From the directory where you originally ran `make install`, enter:

    $ [sudo] make uninstall

You will need to use `sudo` if you did not specify `WEBOS_INSTALL_ROOT`.

# Copyright and License Information

All content, including all source code files and documentation files in this repository are:
 Copyright (c) 2009-2014 LG Electronics, Inc.

All content, including all source code files and documentation files in this repository are:
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this content except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

# end Copyright and License Information


