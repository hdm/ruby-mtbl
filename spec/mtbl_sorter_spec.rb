require 'mtbl'
require 'tempfile'

RSpec.describe MTBL::Sorter, "#new" do

  context "records" do
    sorter =  MTBL::Sorter.new

    it "handles invalid adds" do
      expect { sorter.add(:invalid, "test") }.to raise_error(ArgumentError)
      expect { sorter.add("test", :invalid) }.to raise_error(ArgumentError)
      expect { sorter.add("test", 9000) }.to raise_error(ArgumentError)
      expect { sorter.add(400, "blah") }.to raise_error(ArgumentError)
    end

    it "accepts sequential keys" do
      expect { sorter.add("1000", "test1") }.to_not raise_error
      expect { sorter.add("1005", "test2") }.to_not raise_error
      expect { sorter.add("1010", "test3") }.to_not raise_error
    end

    it "accepts non-sequential keys" do
      expect { sorter.add("1001", "test1") }.to_not raise_error
      expect { sorter.add("1004", "test1") }.to_not raise_error
      expect { sorter.add("1009", "test1") }.to_not raise_error
    end

  end
end
