begin
  load 'Rakefile.local'
rescue LoadError
  ## Ignore
end

## ======================================================================

require 'bundler'
require 'bundler/gem_tasks'

Bundler.setup

## ======================================================================

require 'rake/extensiontask'

Rake::ExtensionTask.new('smb') do |ext|
  ext.ext_dir = 'ext/net/smb'
  ext.lib_dir = 'lib/net/smb'
end

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

