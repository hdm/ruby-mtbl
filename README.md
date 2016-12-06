
Ruby bindings for the mtbl sorted string table library
----------------------------------------------------------------

`ruby-mtbl` is a Ruby interface to Farsight Security's [mtbl](https://github.com/farsightsec/mtbl/)
library, which is a C implementation of the Sorted String Table (SSTable)
data structure. The latest version of `mtbl` depends on [zlib](http://www.zlib.net/),
[LZ4](https://github.com/Cyan4973/lz4) (dev branch > 129), and [Snappy](http://google.github.io/snappy/) 
compression libraries.

On Ubuntu 16.04 LTS, the following command will install the required dependencies:
```
$ sudo apt install libmtbl-dev
```

## Usage

### Generate a mtbl from unsorted input
```
require 'mtbl'
msorter = MTBL::Sorter.new
mwriter = MTBL::Writer.new("tiny.mtbl", MTBL::COMPRESSION_SNAPPY)
msorter.add('key1', 'val1')
msorter.add('key0', 'val2')
msorter.add('key3', 'val3')
msorter.write(mwriter)
```

### Display metadata from a mtbl
```
require 'mtbl'
MTBL::Utils.metadata("tiny.mtbl").each_pair do |k,v|
  puts "#{k.to_s.gsub("_", " ")}: ".ljust(23) + v.to_s
end
```

### Read all pairs from a mtbl
```
require 'mtbl'
mreader = MTBL::Reader.new("tiny.mtbl")
mreader.iterator.each do |r|
  puts r.join(" => ")
end
```

### Read pairs starting with 'k'
```
require 'mtbl'
mreader = MTBL::Reader.new("tiny.mtbl")
iterator = mreader.get_prefix("k")
iterator.each do |r|
  puts r.join(" => ")
end
```


See the command-line tools in ``bin`` and ``examples/test.rb`` for additional usage.
