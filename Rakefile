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

Rake::ExtensionTask.new("net_smb", GEMSPEC)
Rake::TestTask.new

EXT_PATH = 'net_smb'

file "lib/#{EXT_PATH}.so" => Dir.glob("ext/#{EXT_PATH}/*.{rb,c,h}") do
  Dir.chdir("ext/#{EXT_PATH}") do
    ruby 'extconf.rb'
    sh ENV['MAKE'] || 'make'
  end
  cp "ext/#{EXT_PATH}/#{File.basename(EXT_PATH)}.so", "lib/#{EXT_PATH}.so"
end

CLEAN.include('ext/**/*.{log,o,so}')
CLEAN.include('ext/**/Makefile')
CLEAN.include('lib/**/*.so')
CLOBBER.include('pkg/*')
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

task :test => "lib/#{EXT_PATH}.so"

