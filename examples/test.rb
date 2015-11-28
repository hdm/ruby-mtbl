#!/usr/bin/env ruby
path_base = File.expand_path(File.join(File.dirname(__FILE__), ".."))
path_ext  = File.join(path_base, "lib")

$:.unshift(path_ext)

require 'mtbl'
require 'pp'
require 'tempfile'

test_mtbl1 = File.expand_path(File.join(File.dirname(__FILE__), "test1.mtbl"))

reader = MTBL::Reader.new(test_mtbl1)

iter = reader.iterator
1.upto(10) do |x|
  puts [:iterator, x, iter.next].inspect
end
puts ("-" * 80)

get = reader.get("az")
puts [:get, get.next].inspect
puts ("-" * 80)

get = reader.get_prefix("c")
puts [:get_prefix, get.next].inspect
puts ("-" * 80)

get = reader.get_range("ca", "cz")
while(true)
  x = get.next
  break unless x
  puts [:get_range, x].inspect
end
puts ("-" * 80)

pp MTBL::Utils.metadata(test_mtbl1)
puts ("-" * 80)

test_write_mtbl = Tempfile.new("mtbltest")
test_write_path = test_write_mtbl.path
test_write_mtbl.unlink

writer = MTBL::Writer.new(test_write_path)
writer.add("Hello", "Goodbye")
writer.close

reader = MTBL::Reader.new(test_write_path)
iter = reader.iterator
while (r = iter.next)
  puts [:wrote, r].inspect
end
puts ("-" * 80)

begin
  writer.close
rescue ::RuntimeError
end

File.unlink(test_write_path)

MTBL.constants.grep(/^COMPRESSION_/).each do |cname|
  ctype = MTBL.const_get(cname)
  path = test_write_path + "." + cname.to_s
  writer = MTBL::Writer.new(path, ctype)
  writer.add("Hello", "Goodbye")
  writer.close
  info = MTBL::Utils.metadata(path)
  puts [:compression, cname, info[:compression]].inspect
  File.unlink(path)
end
puts ("-" * 80)

sorter = MTBL::Sorter.new
sorter.add("bbb", "2")
sorter.add("aaa", "1")
sorter.add("ccc", "3")
sorter.add("bbb", "22")
sorter.add("aaa", "11")
sorter.add("ccc", "33")
sorter.add("bbb", "222")
sorter.add("aaa", "111")
sorter.add("ccc", "333")

test_write_path + "." + "sorted"
writer = MTBL::Writer.new(test_write_path)
sorter.write(writer)
sorter.close
writer.close

reader = MTBL::Reader.new(test_write_path)
iter = reader.iterator
while (r = iter.next)
  puts [:sorted, r].inspect
end

File.unlink(test_write_path)


puts ("-" * 80)

