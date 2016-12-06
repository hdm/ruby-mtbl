# -*- encoding: utf-8 -*-

Gem::Specification.new do |s|
  s.name        = 'ruby-mtbl'
  s.version     = '1.0.1'
  s.author      = 'HD Moore'
  s.email       = 'x@hdm.io'
  s.license     = 'Apache-2.0'
  s.homepage    = "https://www.github.com/hdm/ruby-mtbl"
  s.summary     = %q{ruby-mtbl: Ruby interface to the MTBL SortedString library}
  s.description = %q{
    The ruby-mtbl gem provides a Ruby interface to Farsight Security's MTBL SortedString library.
  }.gsub(/\s+/, ' ').strip
  s.files         = `git ls-files`.split("\n")
  s.test_files    = `git ls-files -- {test,spec,features}/*`.split("\n")
  s.executables   = `git ls-files -- bin/*`.split("\n").map{ |f| File.basename(f) }
  s.extensions    = [ 'ext/mtbl/extconf.rb' ]
  s.require_paths = ['lib']
end
