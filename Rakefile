begin
  load 'Rakefile.local'
rescue LoadError => e
end

begin
  require 'bundler/gem_tasks'
rescue LoadError => e
end

require 'rake/clean'
require 'rake/extensiontask'
require 'rake/testtask'

GEMSPEC = eval(File.read(File.dirname(__FILE__) + '/net-smb.gemspec'))
EXT_NAME = GEMSPEC.name.gsub(/-/, '_')

Rake::ExtensionTask.new(EXT_NAME, GEMSPEC)
Rake::TestTask.new

CLOBBER.include('pkg')
CLOBBER.include('test/log')
CLOBBER.include('test/log.*')

task :default => [:compile]

task :clobber_pre do
  ## Fix directory permissions to be able to remove by task :clobber
  [*Dir.glob("test/log/share"), *Dir.glob("test/log.*/share")].each do |dir|
    cmd = "find #{dir} -type d -exec chmod u+rwx {} + 2>/dev/null"
    puts cmd
    system cmd
  end
end

task :clobber => [:clobber_pre]

task :test => "lib/#{EXT_NAME}.so"

