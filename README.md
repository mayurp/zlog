# zlog

A structured binary logger based on C++17.
Log almost anything without the overhead of string formatting.

zlog is still a work in progress. However most of the basic features are complete.

## Example

Supports fmt compatible format strings using argument names as keys

```
LOGD("Some log message. {size}, {position}, {list}", 123, Vec3(0, -10, 5), std::list<int>{2, 4, 5}); 
```

Only the arguments in binary form are output by the logger. No string formatting takes place.
Log files are written in the Common Trace Format 1.8 which is an open standard for tracing used by LTTNG.
Cross platform tools such as babeltrace2 and TraceCompass can be used to read the log files.



## How it works

The format string is parsed at compiled time to extract argument names. Metadata about each LOG call is registered during static initialisation.
This metadata is then written on startup as a CTF 1.8 TDSL file. 

## What's supported

* Almost all types (see below)
* Multi-threading - each thread writes to it's own stream (file). TraceCompass will automatically merge this into one events table  

### Types
- [x] Primitives
- [x] Strings 
- [x] Custom types (via REFLECTION macro)
- [x] STL containers (sequential and associative)
- [x] Static arrays (std::array and C arrays)
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
* Test deserisation performance with aligned CTF packets 
