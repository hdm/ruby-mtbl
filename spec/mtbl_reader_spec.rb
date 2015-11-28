require 'mtbl'

RSpec.describe MTBL::Reader, "#new" do

  context "missing file" do
    it "throws an exception" do
      expect { MTBL::Reader.new("missing.mtbl") }.to raise_error(::RuntimeError)
    end
  end

  context "valid file" do
    test_mtbl1 = File.expand_path(File.join(File.dirname(__FILE__), "..", "examples", "test1.mtbl"))
    it "opens successfully" do
      expect { reader = MTBL::Reader.new(test_mtbl1) }.to_not raise_error
    end
  end
end

RSpec.describe MTBL::Reader, "#iterator" do
  test_mtbl1 = File.expand_path(File.join(File.dirname(__FILE__), "..", "examples", "test1.mtbl"))

  context "finds all records" do
    it "has 676 entries" do
      iterator = MTBL::Reader.new(test_mtbl1).iterator
      count = 0
      while (r = iterator.next)
        count += 1
      end
      expect(count).to be(676)
    end

    it "returns the first entry" do 
      iterator = MTBL::Reader.new(test_mtbl1).iterator
      expect(iterator.next).to eq(["aa","1"])
    end

    it "returns the last entry" do
      iterator = MTBL::Reader.new(test_mtbl1).iterator
      last_entry = nil
      while(r = iterator.next)
        last_entry = r
      end
      expect(last_entry).to eq(["zz","676"])
    end
  end
end

RSpec.describe MTBL::Reader, "#get" do
  test_mtbl1 = File.expand_path(File.join(File.dirname(__FILE__), "..", "examples", "test1.mtbl"))

  context "finds an exact key" do
    it "finds aj" do
      expect(MTBL::Reader.new(test_mtbl1).get("aj").next).to eq(["aj", "10"])
    end
    it "finds zx" do
      expect(MTBL::Reader.new(test_mtbl1).get("zx").next).to eq(["zx", "674"])
    end
  end
end

RSpec.describe MTBL::Reader, "#get_prefix" do
  test_mtbl1 = File.expand_path(File.join(File.dirname(__FILE__), "..", "examples", "test1.mtbl"))

  context "finds prefixes" do
    it "finds a*" do
      count = 0
      iterator = MTBL::Reader.new(test_mtbl1).get_prefix("a")
      while(r = iterator.next)
        count += 1
      end
      expect(count).to be(26)
    end

    it "finds z*" do
      count = 0
      iterator = MTBL::Reader.new(test_mtbl1).get_prefix("z")
      while(r = iterator.next)
        count += 1
      end
      expect(count).to be(26)
    end

    it "finds jj" do
      expect(MTBL::Reader.new(test_mtbl1).get_prefix("jj").next).to eq(["jj", "244"])
    end
  end
end

RSpec.describe MTBL::Reader, "#get_range" do
  test_mtbl1 = File.expand_path(File.join(File.dirname(__FILE__), "..", "examples", "test1.mtbl"))
  
  context "finds ranges" do

    it "finds ab..az" do
       count = 0
      iterator = MTBL::Reader.new(test_mtbl1).get_range("ab", "az")
      while(r = iterator.next)
        count += 1
      end
      expect(count).to eq(25)
    end

    it "finds ca..jp" do
       count = 0
      iterator = MTBL::Reader.new(test_mtbl1).get_range("ca", "jp")
      while(r = iterator.next)
        count += 1
      end
      expect(count).to eq(198)
    end
  end
end 
