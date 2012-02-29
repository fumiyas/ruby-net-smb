require 'net/smb'
require 'test/unit'

module Net

class SMBTest < Test::Unit::TestCase
  def setup
  end

  def teardown
  end

  def test_all
    smbcctx = Net::SMBCCTX.new
    p smbcctx.debug
    smbcctx.debug = 5
    p smbcctx.debug
  end
end

end

