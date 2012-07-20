require 'rake/clean'
require 'rake/extensiontask'
require 'rake/testtask'

require 'find'

begin
  load 'Rakefile.local'
rescue LoadError
  ## Ignore
end

begin
  require 'bundler/gem_tasks'
rescue LoadError
  load "net-smb.gemspec"
end

EXT_NAME = GEMSPEC.name.gsub(/-/, '_')

## ======================================================================

CLEAN.include('pkg')
CLOBBER.include('test/log')
CLOBBER.include('test/log.*')

Rake::ExtensionTask.new(EXT_NAME, GEMSPEC)
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

