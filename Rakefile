begin
  load 'Rakefile.local'
rescue LoadError => e
end

begin
  require 'bundler/gem_tasks'
rescue LoadError => e
end

require 'rake/clean'
require 'rake/testtask'

Rake::TestTask.new

CLEAN.include('ext/**/*.{log,o,so}')
CLEAN.include('ext/**/Makefile')
CLEAN.include('test/log')
CLEAN.include('test/log.[0-9]')
CLOBBER.include('lib/**/*.so')

EXT_PATH = 'net/smb'

file "lib/#{EXT_PATH}.so" => Dir.glob("ext/#{EXT_PATH}/*.{rb,c,h}") do
  Dir.chdir("ext/#{EXT_PATH}") do
    ruby 'extconf.rb'
    sh ENV['MAKE'] || 'make'
  end
  cp "ext/#{EXT_PATH}/#{File.basename(EXT_PATH)}.so", "lib/#{EXT_PATH}.so"
end

task :test => "lib/#{EXT_PATH}.so"

