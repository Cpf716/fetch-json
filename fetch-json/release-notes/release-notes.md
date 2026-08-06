# Fetch++

Please see `main.cpp` for sample code.

## Change Log
* Implement a high-performance, proprietary XML serializer/deserializer
* Decouple fetch and json
* Optimize HTTP client performance
* Improve shutdown times tenfold
* Streamline logging
* Fix a bug where empty request targets (lacking / after domain) return "Bad Request"

### Limitations
* Transfer-encoding chunk size lines must not cross packet boundaries