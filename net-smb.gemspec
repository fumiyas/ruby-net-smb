# -*- encoding: utf-8 -*-
$:.push File.expand_path("../lib", __FILE__)
require "net/smb/version"

Gem::Specification.new do |s|
  s.name        = "net-smb"
  s.version     = Net::SMB::VERSION
  s.authors     = ["SATOH Fumiyasu"]
  s.email       = ["fumiyas@osstech.co.jp"]
  s.homepage    = "https://github.com/fumiyas/ruby-net-smb"
  s.summary     = %q{SMB/CIFS client (Samba libsmbclient binding)}
  s.description = %q{SMB/CIFS client (Samba libsmbclient binding)}

  s.files         = `git ls-files`.split("\n")
  s.test_files    = `git ls-files -- {test,spec,features}/*`.split("\n")
  s.executables   = `git ls-files -- bin/*`.split("\n").map{ |f| File.basename(f) }
  s.extensions    = `git ls-files -- ext/*.rb`.split("\n")
  s.require_paths = ["lib"]
end

