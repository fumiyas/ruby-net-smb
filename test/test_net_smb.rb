require 'net/smb'
require 'test/unit'
require 'etc'

module Net

class SMBTest < Test::Unit::TestCase
  def setup
    @test_dir = ENV['TEST_DIR'] = "#{Dir.getwd}/test"
    @username = Etc.getpwuid(Process.uid)['name']
    @password = 'password'

    @smb_conf = ENV['TEST_SMB_CONF'] ||= @test_dir + "/etc/smb.conf"
    @smbd = ENV['TEST_SMBD'] ||= "smbd"
    @pdbedit = ENV['TEST_PDBEDIT'] ||= "pdbedit"
    @samba_debug_level = ENV['TEST_SAMBA_DEBUGLEVEL'] ||= "10"
    @samba_log_dir = ENV['TEST_SAMBA_LOG_DIR'] ||= @test_dir + "/log"
    @samba_var_dir = ENV['TEST_SAMBA_VAR_DIR'] ||= @samba_log_dir + "/var"
    @share_dir = ENV['TEST_SHARE_DIR'] ||= @samba_log_dir + "/share"

    ENV['SMB_CONF_PATH'] = nil;
    ENV['LIBSMB_PROG'] ||= @test_dir + "/bin/smbd.wrapper"

    ## Rotate log directory
    if File.exist?(@samba_log_dir + '.9')
      system('/bin/rm -rf "$TEST_SAMBA_LOG_DIR.9"');
    end
    if File.exist?(@samba_log_dir)
      File.rename(@samba_log_dir, @samba_log_dir + '.0')
    end
    9.downto(1) do |i|
      logdir_a = @samba_log_dir + '.' + (i-1).to_s
      logdir_b = @samba_log_dir + '.' + i.to_s
      if File.exist?(logdir_a)
	File.rename(logdir_a, logdir_b)
      end
    end

    Dir.mkdir(@samba_log_dir, 0750)
    Dir.mkdir(@samba_var_dir, 0750)
    Dir.mkdir(@share_dir, 0750)
    Dir.mkdir(@share_dir + '/dir1', 0750)
    Dir.mkdir(@share_dir + '/dir2', 0750)
    File.open(@share_dir + "/file1.txt", "wb") do |file|
      file.write("file1.txt")
    end

    pdbedit_r, pdbedit_w = IO.pipe
    pdbedit_pid = Kernel.spawn(
      @pdbedit, "--configfile", @smb_conf, "--create", "--password-from-stdin", @username,
      :in => pdbedit_r,
      [:out, :err] => [@samba_log_dir + '/pdbedit.log', 'w'],
    )
    pdbedit_r.close
    pdbedit_w.print(@password, "\n")
    pdbedit_w.print(@password, "\n")
    pdbedit_w.close
  end

  def teardown
  end

  def test_smb
    smb = Net::SMB.new
    smb.on_auth {|server, share|
      [@username, @password]
    }

    smbdir = smb.opendir("smb://localhost/share")
    p smbdir.read
    p smbdir.read
    p smbdir.read
    p smbdir.read
  end
end

end

