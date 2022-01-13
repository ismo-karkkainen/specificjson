# frozen_string_literal: true

require 'rubocop/rake_task'

task default: [:install]

desc 'Clean.'
task :clean do
  `rm -f specificjson-*.gem bin/specificjson`
end

desc 'Build gem.'
task gem: [:clean] do
  `./build --import build.yaml --build bin/specificjson`
  `gem build specificjson.gemspec`
end

desc 'Build and install gem.'
task install: [:gem] do
  `gem install specificjson-*.gem`
end

desc 'Lint using Rubocop'
RuboCop::RakeTask.new(:lint) do |t|
  t.patterns = [ 'build', 'specificjson.gemspec' ]
end
