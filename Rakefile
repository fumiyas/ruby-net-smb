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
CLOBBER.include('test/log.[0-9]')

task :default => [:compile]
task :test => "lib/#{EXT_PATH}.so"

