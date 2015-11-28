require 'mtbl'
require 'tempfile'

RSpec.describe MTBL::Writer, "#new" do

  context "arguments" do
    it "throws an exception on missing filename" do
      expect { MTBL::Writer.new() }.to raise_error(ArgumentError)
    end

    it "does not throw an exception with a valid filename" do
      tfile = Tempfile.new("mtbl-writer")
      tpath = tfile.path
      tfile.unlink
      expect { MTBL::Writer.new(tpath) }.to_not raise_error
      File.unlink(tpath)
    end

    it "throws an exception with existing file" do
      tfile = Tempfile.new("mtbl-writer")
      expect { MTBL::Writer.new(tfile.path) }.to raise_error(ArgumentError)
    end

    it "throws an exception with non-string arguments" do
      expect { MTBL::Writer.new(9000) }.to raise_error(ArgumentError)
    end

    it "throws an exception with non-integer compression type" do
      tfile = Tempfile.new("mtbl-writer")
      tpath = tfile.path
      tfile.unlink
      expect { MTBL::Writer.new(tpath, "blagh") }.to raise_error(ArgumentError)
    end

    it "throws an exception with an invalid compression type" do
      tfile = Tempfile.new("mtbl-writer")
      tpath = tfile.path
      tfile.unlink
      expect { MTBL::Writer.new(tpath, 999) }.to raise_error(ArgumentError)
    end

    it "does not throw an exception with a valid compression type" do
      tfile = Tempfile.new("mtbl-writer")
      tpath = tfile.path
      tfile.unlink
      expect { MTBL::Writer.new(tpath, MTBL::COMPRESSION_ZLIB) }.to_not raise_error
      File.unlink(tpath)
    end

    it "throws an exception with an non-integer block size" do
      tfile = Tempfile.new("mtbl-writer")
      tpath = tfile.path
      tfile.unlink
      expect { MTBL::Writer.new(tpath, MTBL::COMPRESSION_ZLIB, "blargh") }.to raise_error(ArgumentError)
    end

    it "does not throw an exception with an integer block size" do
      tfile = Tempfile.new("mtbl-writer")
      tpath = tfile.path
      tfile.unlink
      expect { MTBL::Writer.new(tpath, MTBL::COMPRESSION_ZLIB, 2048) }.to_not raise_error
      File.unlink(tpath)
    end

    it "throws an exception with an non-integer restart interval" do
      tfile = Tempfile.new("mtbl-writer")
      tpath = tfile.path
      tfile.unlink
      expect { MTBL::Writer.new(tpath, MTBL::COMPRESSION_ZLIB, 4096, "blargh") }.to raise_error(ArgumentError)
    end

  end

  context "records" do
    tfile = Tempfile.new("mtbl-writer")
    tpath = tfile.path
    tfile.unlink
    writer =  MTBL::Writer.new(tpath)

    it "handles invalid adds" do
      expect { writer.add(:invalid, "test") }.to raise_error(ArgumentError)
      expect { writer.add("test", :invalid) }.to raise_error(ArgumentError)
      expect { writer.add("test", 9000) }.to raise_error(ArgumentError)
      expect { writer.add(400, "blah") }.to raise_error(ArgumentError)
    end

    it "accepts sequential keys" do
      expect { writer.add("1000", "test1") }.to_not raise_error
      expect { writer.add("1005", "test2") }.to_not raise_error
      expect { writer.add("1010", "test3") }.to_not raise_error
    end

    it "does not accept non-sequential keys" do
      expect { writer.add("1001", "test1") }.to raise_error(RuntimeError)
      expect { writer.add("1004", "test1") }.to raise_error(RuntimeError)
      expect { writer.add("1009", "test1") }.to raise_error(RuntimeError)
    end

    File.unlink(tpath)
  end
end
