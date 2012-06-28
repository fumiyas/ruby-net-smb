## vim:fileencoding=utf-8
## -*- encoding: utf-8 -*-

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

    @share_private = "smb://localhost/private"
    @share_public = "smb://localhost/public"

    ENV['SMB_CONF_PATH'] = nil;
    ENV['LIBSMB_PROG'] ||= @test_dir + "/bin/smbd.wrapper"

    ## Rotate log directory
    if File.exist?(@samba_log_dir + '.9')
      system('/bin/rm', '-rf', @samba_log_dir + '.9');
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

  def test_auth
    smb = Net::SMB.new
    smb.on_auth {|server, share|
      [@username, @password]
    }
    assert_nothing_raised do
      smbdir = smb.opendir(@share_private)
      smbdir.close
    end

    smb = Net::SMB.new
    smb.on_auth {|server, share|
      [@username, 'invalid-password']
    }
    assert_raise(Errno::EPERM) do
      smb.opendir(@share_private)
    end

    smb = Net::SMB.new
    smb.on_auth {|server, share|
      ['invalid-user', @password]
    }
    assert_raise(Errno::EACCES) do
      smb.opendir(@share_private)
    end

    smb = Net::SMB.new
    smb.on_auth {|server, share|
      'blah-blah'
    }
    assert_raise(TypeError) do
      smb.opendir(@share_private)
    end

    smb = Net::SMB.new
    smb.on_auth {|server, share|
      [@username]
    }
    assert_raise(ArgumentError) do
      smb.opendir(@share_private)
    end
  end

  def test_dir
    dents_dirs = ["dir", "ディレクトリ"]
    dents_files = ["file", "ファイル"]
    dents_all = [".", "..", *dents_dirs, *dents_files]
    dents_dirs.each do |dname|
      Dir.mkdir(@share_dir + '/' + dname, 0750)
    end
    dents_files.each do |fname|
      File.open(@share_dir + '/' + fname, "wb") do |file|
	file.write(fname)
      end
    end

    smb = Net::SMB.new
    smb.on_auth {|server, share|
      [@username, @password]
    }

    smbdir = smb.opendir(@share_private)
    dents = dents_all.clone
    while fname = smbdir.read
      assert_equal(fname, dents.delete(fname), "Unexpected directory entry: #{fname}")
    end
    assert_empty(dents)

    smbdir = smb.opendir(@share_private)
    dents = dents_all.clone
    smbdir.each do |fname|
      assert(dents.delete(fname) != nil)
    end
    assert_empty(dents)

    smbdir = smb.opendir(@share_private)
    dents = dents_all.clone
    smbdir_enum = smbdir.each
    dents.size.times do |n|
      fname = smbdir_enum.next
      assert_equal(fname, dents.delete(fname), "Unexpected directory entry: #{fname}")
    end
    assert_raise(StopIteration) do
      smbdir_enum.next
    end
    assert_empty(dents)
  end
end

end

