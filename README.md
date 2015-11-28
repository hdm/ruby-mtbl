
ruby interface to the `mtbl` immutable sorted string table library
==================================================================

Introduction
------------

`ruby-mtbl` is a Ruby interface to Farsight Security's [mtbl](https://github.com/farsightsec/mtbl/)
library, which is a C implementation of the Sorted String Table (SSTable)
data structure. The latest version of `mtbl` depends on [zlib](http://www.zlib.net/),
[LZ4](https://github.com/Cyan4973/lz4) (dev branch > 129), and [Snappy](http://google.github.io/snappy/) 
compression libraries.

