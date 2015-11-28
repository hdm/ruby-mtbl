require 'mtbl'
require 'tempfile'

RSpec.describe MTBL::Utils, "#metadata" do
  test_mtbl1 = File.expand_path(File.join(File.dirname(__FILE__), "..", "examples", "test1.mtbl"))

  context "invalid file" do

    it "throws an exception with an invalid path" do
      expect{ MTBL::Utils.metadata("missing.mtbl") }.to raise_error(RuntimeError)
    end

    it "throws an exception with an invalid file" do
      tfile = Tempfile.new("mtbl")
      expect{ MTBL::Utils.metadata(tfile.path) }.to raise_error(RuntimeError)
    end

    it "throws an exception with a non-string argument" do
       expect{ MTBL::Utils.metadata(3000) }.to raise_error(ArgumentError)
    end

  end

  context "valid file" do
    info = MTBL::Utils.metadata(test_mtbl1)
    it "has correct metadata" do
      expect(info[:filesize]).to eq(2866)
      expect(info[:index_block_offset]).to eq(2332)
      expect(info[:index_bytes]).to eq(22)
      expect(info[:data_block_bytes]).to eq(2332)
      expect(info[:data_block_size]).to eq(8192)
      expect(info[:data_block_count]).to eq(1)
      expect(info[:entry_count]).to eq(676)
      expect(info[:key_bytes]).to eq(1352)
      expect(info[:value_bytes]).to eq(1920)
      expect(info[:compression]).to eq("zlib")
    end
  end
end

