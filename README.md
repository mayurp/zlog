# zlog

A C++17 based structured binary logger.

Log almost anything without the overhead of string formatting.

zlog is still a work in progress. However, most of the basic features are complete.

## Example

```cpp
LOGD("Some log message. {size}, {position}, {list}", 123, Vec3(0, -10, 5), std::list<int>{2, 4, 5}); 
```

Supports fmt compatible format strings using argument names as keys.
 
Only the arguments in binary form are output by the logger. No string formatting takes place in the program.
Log files are written in the Common Trace Format 1.8 which is an open standard for tracing used by LTTNG.
Cross platform tools such as babeltrace2 and TraceCompass can be used to parse the log files and display them in a human readable format

For example this is how to 
```sh
$ babeltrace2 trace_dir --fields loglevel

[16:29:11.519119000] TRACE_DEBUG (14) [18] Some log message. {size}, {position}, {list}: { size = 123, position = { x = 0, y = -10, z = 5 }, list_length = 3, list = [ [0] = 2, [1] = 4, [2] = 5 ] }
```

## How it works

The format string is parsed at compiled time to extract argument names. Metadata about each LOG statement is then registered during static initialisation.
This metadata is then written on startup as a CTF 1.8 TDSL file.

Logging of custom types is supported via compile time reflection. Use the REFLECT macro in the same compilation as the LOG statement as below.
Only public members are currenty supported:

```cpp
struct Vec3
{
    float x, y, z;    
};
REFLECT(Vec3, x, y, z)
```

Example using a member function:

```cpp
struct SomeType
{
    int a, b;
    int sum() const
    {
        return a + b;
    }
};
REFLECT(SomeType, sum)
```

## What's supported

* Almost all types (see below)
* Multi-threading - each thread writes to it's own stream (file). TraceCompass will automatically merge this into one events table  
* Source code lookup in TraceCompass

### Types
- [x] primitives
- [x] strings 
- [x] custom types (via REFLECTION macro)
- [x] STL containers (sequential and associative)
- [x] static arrays (std::array and C arrays)
- [x] enums
- [x] std::pair
- [ ] std::tuple
- [ ] pointers

## TODOs
* Control log level filtering at runtime
* Support optional text based output via fmt library for convenience
* Submodules for third party dependencies
* More tests

## Future work
* Asynchronous logging (MPSC ring buffer which is then written to disk on another thread)
* Tracecompass plugin to display format strings and loglevel
* Babeltrace2 plugin to display arguments inline in format strings
* Test deserisation performance with aligned CTF packets 
