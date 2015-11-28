require 'mtbl'

RSpec.describe MTBL, "#constants" do
  context "class" do
    it "has classes" do
      expect(MTBL.constants.include?(:Reader)).to be(true)
      expect(MTBL.constants.include?(:Writer)).to be(true)
      expect(MTBL.constants.include?(:Utils)).to be(true)
      expect(MTBL.constants.include?(:Sorter)).to be(true)
      expect(MTBL.constants.include?(:COMPRESSION_NONE)).to be(true)
      expect(MTBL.constants.include?(:COMPRESSION_ZLIB)).to be(true)
      expect(MTBL.constants.include?(:COMPRESSION_SNAPPY)).to be(true)
      expect(MTBL.constants.include?(:COMPRESSION_LZ4)).to be(true)
      expect(MTBL.constants.include?(:COMPRESSION_LZ4HC)).to be(true)
    end
  end
end
