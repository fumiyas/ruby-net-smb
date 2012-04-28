require 'net/smb'
require 'test/unit'

module Net

class SMBTest < Test::Unit::TestCase
  def setup
    ENV['LIBSMB_PROG'] ||= "test/bin/smbd.wrapper"
    ENV['TEST_SMB_CONF'] ||= "test/etc/smb.conf"
    ENV['TEST_SMBD'] ||= "smbd"
    ENV['TEST_SMBD_DEBUGLEVEL'] ||= "10"
    ENV['TEST_SMBD_LOG_DIR'] ||= "test/log"
    ENV['TEST_SMBD_VAR_DIR'] ||= ENV['TEST_SMBD_LOG_DIR'] + "/var"
    ENV['TEST_SMBD_SHARE_DIR'] ||= ENV['TEST_SMBD_LOG_DIR'] + "/share"

    if File.exist?(ENV['TEST_SMBD_LOG_DIR'] + '.9')
      system('/bin/rm -rf "$TEST_SMBD_LOG_DIR.9"');
    end
    if File.exist?(ENV['TEST_SMBD_LOG_DIR'])
      File.rename(ENV['TEST_SMBD_LOG_DIR'], ENV['TEST_SMBD_LOG_DIR'] + '.0')
    end
    9.downto(1) do |i|
      logdir_a = ENV['TEST_SMBD_LOG_DIR'] + '.' + (i-1).to_s
      logdir_b = ENV['TEST_SMBD_LOG_DIR'] + '.' + i.to_s
      if File.exist?(logdir_a)
	File.rename(logdir_a, logdir_b)
      end
    end

    Dir.mkdir(ENV['TEST_SMBD_LOG_DIR'], 0750)
    Dir.mkdir(ENV['TEST_SMBD_VAR_DIR'], 0750)
    Dir.mkdir(ENV['TEST_SMBD_SHARE_DIR'], 0750)
  end

  def teardown
  end

  def test_all
    s = Net::SMBCCTX.new
  end
end

end

