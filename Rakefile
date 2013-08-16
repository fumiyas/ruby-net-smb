begin
  load 'Rakefile.local'
rescue LoadError
  ## Ignore
end

## ======================================================================

require 'bundler/gem_tasks'

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
CLOBBER.include('test/etc/smb.conf')
CLOBBER.include('test/log')
CLOBBER.include('test/log.*')

## ======================================================================

require 'find'

task :default => [:compile]

task :test => [:compile]
task :test => 'test/etc/smb.conf'

file 'test/etc/smb.conf' => 'test/etc/smb.conf.tmpl' do |t|
  test_dir = File.dirname(__FILE__) + '/test'
  tmpl = File.open(t.name + '.tmpl')
  File.open(t.name, 'w') do |out|
    tmpl.each_line do |line|
      out.print line.gsub('@@test_dir@@', test_dir)
    end
  end
  tmpl.close
end

task :clobber_pre do
  ## Fix directory permissions to be able to remove by task :clobber
  Find.find(*Dir.glob("test/log/share"), *Dir.glob("test/log.*/share")) do |path|
    if File.directory?(path)
      File.chmod(0755, path)
    end
  end
end

task :clobber => [:clobber_pre]

