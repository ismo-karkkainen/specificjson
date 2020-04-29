#!/usr/bin/env ruby

require 'optparse'
require 'json'
require 'yaml'

#$PIECESASSIGNMENT#


def load_piece(piece, field, directory)
  file = piece.fetch(field, nil)
  file = IO.read(File.join(directory, file)) unless file.nil?
  piece[field] = file
end

def load_more_pieces(recipe_filename)
  recipe = YAML.load(File.open(recipe_filename, 'r'))
  unless recipe.is_a? Array
    STDERR.puts 'Recipe file must be a YAML array of file names.'
    exit(2)
  end
  recipe.each do |filename|
    puts filename
    piece = YAML.load(File.open(filename, 'r'))
    d = File.dirname filename
    piece.each_pair do |name, spec|
      load_piece(spec, 'header', d)
      load_piece(spec, 'source', d)
      load_piece(spec, 'license', d)
      # Set other defaults.
      $PIECES[name] = spec
    end
  end
end


# Used by build script to load the file but avoid parsing options.
unless global_variables.include? :$BYPASSOPTS

$RECIPE = nil
$OUTPUT = nil

parser = OptionParser.new do |opts|
  opts.summary_indent = '  '
  opts.summary_width = 26
  opts.banner = "Usage: build [options]"
  opts.separator ""
  opts.separator "Options:"
  opts.on('-r', '--recipe FILENAME') { |f| $RECIPE = f }
  opts.on('-s', '--script FILENAME') { |f| $SCRIPT = f }
  opts.on('-o', '--output FILENAME') { |f| $OUTPUT = f }
  opts.on('-h', '--help', 'Print this help and exit.') do
    STDOUT.puts opts
    exit 0
  end
end
parser.parse!

unless $RECIPE.nil?
  load_more_pieces($RECIPE)
end

end

