begin
  load 'Rakefile.local'
rescue LoadError
  ## Ignore
end

## ======================================================================

require 'bundler/gem_helper'

GEM_SPEC = Bundler::GemHelper.new(Dir.pwd).install.gemspec
EXT_NAME = GEM_SPEC.name.gsub(/-/, '_')

## ======================================================================

require 'rake/extensiontask'

Rake::ExtensionTask.new(EXT_NAME, GEM_SPEC)

## ======================================================================

require 'rake/testtask'

Rake::TestTask.new

## ======================================================================

require 'rake/clean'

CLEAN.include('pkg')
CLOBBER.include('test/log')
CLOBBER.include('test/log.*')

## ======================================================================

require 'find'

task :default => [:compile]
task :test => [:compile]

task :clobber_pre do
  ## Fix directory permissions to be able to remove by task :clobber
  Find.find(*Dir.glob("test/log/share"), *Dir.glob("test/log.*/share")) do |path|
    if File.directory?(path)
      File.chmod(0755, path)
    end
  end
end

task :clobber => [:clobber_pre]

