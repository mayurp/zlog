# Logger

Structured binary logger

# MVP:

* Pass in bare ctf context (perhaps using TLS or directly has a parameter)
* Add support for reflected types
* Add log level filtering?
* Make std::cout output via fmt library optional (compile time / Info level and above ?)
* github subrepo dependency on fmt library
* write tests - mainly check no copies of data are done


# Future work
 
* Multi-threading support (Multi producer single consumer ring buffer which is then written to file)
* Formatted string support (tracecompass plugin?)
* Add fmt formatters for any container type (array, list, map, set etc)
* tidy up template stuff (extract type traits?)
