Gem::Specification.new do |s|
  s.name        = 'specificjson'
  s.version     = '0.1.0'
  s.date        = '2021-09-16'
  s.summary     = 'Generate C++ JSON-parsing source from input description.'
  s.description = %q(
A tool to generate C++ header and source file with templates for parsing and
writing JSON from expected input description in YAML.

Source: https://github.com/ismo-karkkainen/specificjson

Licensed under Universal Permissive License, see LICENSE.txt.
)
  s.authors     = [ 'Ismo Kärkkäinen' ]
  s.email       = 'ismokarkkainen@icloud.com'
  s.files       = [ 'LICENSE.txt' ]
  s.executables << 'specificjson'
  s.homepage    = 'http://rubygems.org/gems/specificjson'
  s.license     = 'UPL-1.0'
end
