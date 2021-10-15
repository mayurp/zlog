# Logger

Structured binary logger

## Types Supported

- [x] custom types
- [x] STL containers (iterable, associative)
- [x] std::array
- [x] int someArray[10]
- [x] enums

- [ ] pair
- [ ] tuple
- [ ] allow nested type definitions


* Fix style? (camelCase or snake_case - pick one!!!)
* Add log level filtering?
* Make std::cout output via fmt library optional (compile time / Info level and above ?)
* github subrepo dependency on fmt library
* Write tests 
	* check no copies of data are done
	* ctf logging of all integers types


# Future work
* Multi-threading support (Multi producer single consumer ring buffer which is then written to file)
* Tracecompass plugin to display format strings
* Make loglevel work / display in tracecompass?
* Add fmt formatters for any container type (array, list, map, set etc)
* Deal with alignment
	* align primitives to their own size
	* align struct to...? 
