require 'find'

begin
  load 'Rakefile.local'
rescue LoadError
  ## Ignore
end

GEM_SPEC = begin
  require 'bundler/gem_helper'
  helper = Bundler::GemHelper.new(Dir.pwd)
  helper.install
  helper.gemspec
rescue LoadError
  fname = File.basename(Dir.pwd).sub(%r#^(?:ruby-)?(.+?)(?:-\d.*)?$#, '\1.gemspec')
  contents = File.read(fname)
  eval(contents, TOPLEVEL_BINDING, fname)
end

EXT_NAME = GEM_SPEC.name.gsub(/-/, '_')

## ======================================================================

require 'rake/clean'

CLEAN.include('pkg')
CLOBBER.include('test/log')
CLOBBER.include('test/log.*')

## ======================================================================

require 'rake/extensiontask'

Rake::ExtensionTask.new(EXT_NAME, GEM_SPEC) do |task|
  task.source_pattern = '*.{c,h}'
end

## ======================================================================

require 'rake/testtask'

Rake::TestTask.new

## ======================================================================

task :default => [:compile]

task :clobber_pre do
  ## Fix directory permissions to be able to remove by task :clobber
  Find.find(*Dir.glob("test/log/share"), *Dir.glob("test/log.*/share")) do |path|
    if File.directory?(path)
      File.chmod(0755, path)
    end
  end
end

task :clobber => [:clobber_pre]

task :test => "lib/#{EXT_NAME}.so"

