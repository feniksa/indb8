Summary
-------
A userspace service that provides access to the openWebOS database.
This is a fork of DB8 project: https://github.com/openwebos/db8

Description
-----------
InDB8 is a userspace service that provides access to the webOS database.  Access to the database APIs is provided over the luna-service bus.  This initial release provides the infrastructure code to wrap a range of database engines.

Major changes
----------
* No dependecy from openwebos components
* Removed dependency from media services
* Remove pmloglib logging engine, use only internal indb8 logging engine
* Port some bug-fixes from db8 upstream repository
* Add support for getting list of registered kinds
* Refactoring C++ and CMakeLists

How to Build on Linux
=====================

## Building for OpenWebOS

For building under webos, you can follow the same steps as for db8 upstream. See: http://github.com/openwebos/db8

### Dependencies

Below are the tools and libraries (and their minimum versions) required to build this package:

* cmake (version required by openwebos/cmake-modules-webos)
* libdb-4.8 (if compile with berkeleydb support)
* libicu
* glib-2.0
* gthread-2.0
* openwebos/luna-service2
* leveldb
* leveldb-tl (https://github.com/ony/leveldb-tl)

## Building for Gentoo

Current InDB8 supported by gentoo WebOS layer.
To install it via layman, add WebOS layer: 
    
    # layman -A webos

And install from layman:
    
    # emerge -av indb8

Gentoo will automatically install indb8 upstart script and all required dependency.

## Building Standalone

### Using cmake

Once you have downloaded the source, execute the following to build it (after changing into the directory under which it was downloaded):

    $ mkdir build
    $ cd build
    $ cmake ..
    $ make

### Configure database backend

InDB8 support different database backends. To select backend, trigger any available option to true:
backends. Currently support database backends are:

* BUILD_ENGINE_SANDWICH for leveldb with txn support
* BUILD_ENGINE_LEVELDB  for leveldb without txn support
* BUILD_ENGINE_BERKELEY for berkeleydb support

For example: 

    $ cmake -D BUILD_ENGINE_SANDWICH:BOOL=true ..

By default, db8 compiles with sandwich database backend.

### Enable unit tests

To build the unit tests for db8, specify a true value for `BUILD_TESTS` on the cmake command line. For example:

    $ cmake -D BUILD_TESTS:BOOL=true

### Debug build

To build InDB8 in debug mode, add to cmake param -DCMAKE_BUILD_TYPE=debug

    $ cmake -DCMAKE_BUILD_TYPE=debug ..

### Uninstalling

From the directory where you originally ran `make install`, enter:

    $ [sudo] make uninstall

You will need to use `sudo` if you did not specify `WEBOS_INSTALL_ROOT`.

How to configure
=====================

## Configuration file mojodb.conf

By defult, on startup InDB8 read configuration file /etc/palm/db8/maindb.conf
This configuration file self-explained.

To pass into InDB8 path to configuration file directly, you can set -c /path/mojodb.conf
    
    $ ./mojodb-luna -c /etc/palm/db8/maindb.conf

## Configure Logging

### Logging target 

DB8 supportes different output targets. Supported output targets are: 
    file
    syslog
    stdout
    stderr

To set output target, modify mojodb.conf (or maindb.conf) and set appender type to required:
   ...
   "log" : {
      "appender" : {
         "type" : "syslog"
     },
   ...

### Output level

    In release mode, input db8 output different log information into log appender, except debug and trace log messages. Debug and Trace log
messages output only in indb8 debug  build (compiled with -DCMAKE_BUILD_TYPE=debug param)

Supported levels of loggin:

    trace
    debug
    info
    notice
    warning
    error
    critical
    none

To change loggin level,  modify mojodb.conf (maindb.conf) and set log -> level param to required value.
    ...
   "log" : {
        ...
        "levels" : {
            "default" : "notice"
        }
    ...

# Copyright and License Information

All content, including all source code files and documentation files in this repository are:
 Copyright (c) 2009-2015 LG Electronics, Inc.

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


