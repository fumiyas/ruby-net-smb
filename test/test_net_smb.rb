## vim:fileencoding=utf-8
## -*- encoding: utf-8 -*-

require 'net/smb'
require 'test/unit'
require 'etc'

module Net

class SMBTest < Test::Unit::TestCase
  @@test_dir = ENV['TEST_DIR'] = "#{Dir.getwd}/test"
  @@username = Etc.getpwuid(Process.uid)['name']
  @@password = 'password'

  @@smb_conf = ENV['TEST_SMB_CONF'] ||= @@test_dir + "/etc/smb.conf"
  @@smbd = ENV['TEST_SMBD'] ||= "smbd"
  @@pdbedit = ENV['TEST_PDBEDIT'] ||= "pdbedit"
  @@smbstatus = ENV['TEST_SMBSTATUS'] ||= "smbstatus"
  @@samba_debug_level = ENV['TEST_SAMBA_DEBUGLEVEL'] ||= "10"
  @@samba_log_dir = ENV['TEST_SAMBA_LOG_DIR'] ||= @@test_dir + "/log"
  @@samba_var_dir = ENV['TEST_SAMBA_VAR_DIR'] ||= @@samba_log_dir + "/var"
  @@share_dir = ENV['TEST_SHARE_DIR'] ||= @@samba_log_dir + "/share"
  @@samba_options = [
    "--configfile=#{@@smb_conf}",
    "--debuglevel=#{@@samba_debug_level}",
    "--log-basename=#{@@samba_var_dir}",
  ]

  @@share_private =	"smb://localhost/private"
  @@share_public =	"smb://localhost/public"
  @@dir_noexist =	"dir.noexist"
  @@dir_writeable =	"dir.writeable"
  @@dir_writeable_m =	"ディレクトリ.writeable"
  @@dirs_writeable =	[@@dir_writeable, @@dir_writeable_m]
  @@dir_readable =	"dir.readable"
  @@dirs_readable =	[@@dir_readable]
  @@dir_noaccess =	"dir.noaccess"
  @@dirs_noaccess =	[@@dir_noaccess]
  @@dirs = [".", ".."] + @@dirs_readable + @@dirs_writeable + @@dirs_noaccess
  @@file_noexist =	"file.noexist"
  @@file_writeable =	"file.writeable"
  @@file_writeable_m =	"ファイル.writeable"
  @@files_writeable =	[@@file_writeable, @@file_writeable_m]
  @@file_readable =	"file.readable"
  @@files_readable =	[@@file_readable]
  @@file_noaccess =	"file.noaccess"
  @@files_noaccess =	[@@file_noaccess]
  @@file_large =	"file.large"
  @@files_misc =	[@@file_large]
  @@files = @@files_readable + @@files_writeable + @@files_noaccess + @@files_misc

  ENV['SMB_CONF_PATH'] = nil;
  ENV['LIBSMB_PROG'] ||= @@test_dir + "/bin/smbd.wrapper"

  ## Rotate log directory
  if File.exist?(@@samba_log_dir + '.9')
    system('/bin/rm', '-rf', @@samba_log_dir + '.9');
  end
  if File.exist?(@@samba_log_dir)
    File.rename(@@samba_log_dir, @@samba_log_dir + '.0')
  end
  9.downto(1) do |i|
    logdir_a = @@samba_log_dir + '.' + (i-1).to_s
    logdir_b = @@samba_log_dir + '.' + i.to_s
    if File.exist?(logdir_a)
      File.rename(logdir_a, logdir_b)
    end
  end

  Dir.mkdir(@@samba_log_dir, 0750)
  Dir.mkdir(@@samba_log_dir + '/private', 0750)
  Dir.mkdir(@@samba_var_dir, 0750)
  Dir.mkdir(@@share_dir, 0750)
  @@dirs_readable.each do |dname|
    Dir.mkdir(@@share_dir + '/' + dname, 0550)
  end
  @@dirs_writeable.each do |dname|
    Dir.mkdir(@@share_dir + '/' + dname, 0750)
  end
  @@dirs_noaccess.each do |dname|
    Dir.mkdir(@@share_dir + '/' + dname, 0000)
  end
  @@files_readable.each do |fname|
    File.open(@@share_dir + '/' + fname, "wb", 0440) do |file|
      file.write(fname)
    end
  end
  @@files_writeable.each do |fname|
    File.open(@@share_dir + '/' + fname, "wb", 0660) do |file|
      file.write(fname)
    end
  end
  @@files_noaccess.each do |fname|
    File.open(@@share_dir + '/' + fname, "wb", 0000) do |file|
      file.write(fname)
    end
  end

  File.open(@@share_dir + '/' + @@file_large, "wb", 0660) do |file|
    random_chars = (0...100).map { rand(256).chr }.join("")
    100000.times do |n|
      file.write(random_chars)
    end
  end

  pdbedit_r, pdbedit_w = IO.pipe
  pdbedit_pid = Kernel.spawn(
    @@pdbedit, *@@samba_options, "--create", "--password-from-stdin", @@username,
    :in => pdbedit_r,
    [:out, :err] => [@@samba_log_dir + '/pdbedit.log', 'w'],
  )
  pdbedit_r.close
  pdbedit_w.print(@@password, "\n")
  pdbedit_w.print(@@password, "\n")
  pdbedit_w.close

  def setup
  end

  def teardown
  end

  def smbstatus
    smbstatus_r, smbstatus_w = IO.pipe
    smbstatus_pid = Kernel.spawn(
      @@smbstatus, *@@samba_options, "--shares",
      :out => smbstatus_w,
      :err => [@@samba_log_dir + '/smbstatus.log', 'w+'],
    )
    smbstatus_w.close
    smbstatus_r.readline
    smbstatus_r.readline
    smbstatus_r.readline

    return smbstatus_r
  end

  def smb
    smb = Net::SMB.new
    smb.auth_callback {|server, share|
      [@@username, @@password]
    }

    return smb
  end

  def test_auth
    smb = Net::SMB.new
    smb.auth_callback {|server, share|
      [@@username, @@password]
    }
    assert_nothing_raised do
      smbdir = smb.opendir(@@share_private)
      smbdir.close
    end

    smb = Net::SMB.new
    smb.auth_callback {|server, share|
      [@@username, 'invalid-password']
    }
    assert_raise(Errno::EPERM) do
      smb.opendir(@@share_private)
    end

    smb = Net::SMB.new
    smb.auth_callback {|server, share|
      ['invalid-user', @@password]
    }
    assert_raise(Errno::EACCES) do
      smb.opendir(@@share_private)
    end

    smb = Net::SMB.new
    smb.auth_callback {|server, share|
      'blah-blah'
    }
    assert_raise(TypeError) do
      smb.opendir(@@share_private)
    end

    smb = Net::SMB.new
    smb.auth_callback {|server, share|
      [@@username]
    }
    assert_raise(ArgumentError) do
      smb.opendir(@@share_private)
    end
  end

  def test_dir_open_close
    smb = self.smb

    smbdir = smb.opendir(@@share_public)
    assert_equal(smb.object_id, smbdir.smb.object_id)
    assert_equal(@@share_public, smbdir.url)

    assert_equal(false, smbdir.closed?)
    smbdir.close
    assert_equal(true, smbdir.closed?)

    assert_raise(IOError) do
      smbdir.close
    end
    assert_raise(IOError) do
      smbdir.read
    end
    assert_raise(IOError) do
      smbdir.pos
    end
    assert_raise(IOError) do
      smbdir.seek(0)
    end

    assert_raise(Errno::ENOENT) do
      smbdir = smb.opendir(@@share_public + '/' + @@dir_noexist)
    end
    assert_raise(Errno::EACCES) do
      smbdir = smb.opendir(@@share_public + '/' + @@dir_noaccess)
    end
    ## Errno::ENOENT is not expected, but Samba 3.5 and 3.6 has a bug:
    ## https://bugzilla.samba.org/show_bug.cgi?id=9021
    assert_raise(Errno::ENOTDIR, Errno::ENOENT) do
      smbdir = smb.opendir(@@share_public + '/' + @@file_writeable)
    end
  end ## test_dir_open_close

  def test_dir_read
    smb = self.smb
    dent_names_all = [*@@dirs, *@@files]

    smbdir = smb.opendir(@@share_private)
    dent_names = dent_names_all.clone
    while dent = smbdir.read
      assert_equal(dent.name, dent_names.delete(dent.name),
	  "Unexpected directory entry: #{dent.name}")
      if @@dirs.include?(dent.name)
	assert_equal(true, dent.dir?)
      elsif @@files.include?(dent.name)
	assert_equal(true, dent.file?)
      end
    end
    assert_empty(dent_names)
    smbdir.close

    smb.opendir(@@share_public) do |smbdir|
      dent_names = dent_names_all.clone
      while dent = smbdir.read
	assert_equal(dent.name, dent_names.delete(dent.name),
	    "Unexpected directory entry: #{dent.name}")
      end
      assert_empty(dent_names)
    end
  end ## test_dir_read

  def test_dir_seek
    smb = self.smb
    dent_names_all = [*@@dirs, *@@files]

    smbdir = smb.opendir(@@share_private)

    smbdir_pos_all = Array.new
    fname_by_pos = Hash.new
    loop do
      pos = smbdir.pos
      smbdir_pos_all << pos
      dent = smbdir.read
      fname_by_pos[pos] = dent ? dent.name : nil;
      break unless dent
    end

    smbdir.rewind
    dent_names = dent_names_all.clone
    smbdir_pos = smbdir_pos_all.clone
    loop do
      pos = smbdir.pos
      assert_equal(smbdir_pos.shift, pos)
      unless dent = smbdir.read
	break
      end
      assert_equal(dent.name, dent_names.delete(dent.name),
	  "Unexpected directory entry: #{dent.name}")
      assert_equal(fname_by_pos[pos], dent.name,
	  "Unexpected directory entry: #{dent.name} #{pos}")
    end
    assert_empty(dent_names)

    smbdir_pos_all.each do |pos|
      smbdir.seek(pos)
      dent = smbdir.read
      dent_name = dent ? dent.name : nil;
      assert_equal(fname_by_pos[pos], dent_name,
	  "Unexpected directory entry: #{dent_name} #{pos}")
    end

    smbdir_pos_all.reverse.each do |pos|
      smbdir.seek(pos)
      dent = smbdir.read
      dent_name = dent ? dent.name : nil;
      assert_equal(fname_by_pos[pos], dent_name,
	  "Unexpected directory entry: #{dent_name} #{pos}")
    end

    smbdir.close
  end ## test_dir_seek

  def test_dir_enum
    smb = self.smb
    dent_names_all = [*@@dirs, *@@files]

    smbdir = smb.opendir(@@share_private)
    dent_names = dent_names_all.clone
    smbdir.each do |dent|
      assert_equal(dent.name, dent_names.delete(dent.name))
    end
    assert_empty(dent_names)
    smbdir.close

    smbdir = smb.opendir(@@share_private)
    dent_names = dent_names_all.clone
    smbdir.read
    smbdir.read
    smbdir.each do |dent|
      assert_equal(dent.name, dent_names.delete(dent.name))
    end
    assert_empty(dent_names)
    smbdir.close

    smbdir = smb.opendir(@@share_private)
    dent_names = dent_names_all.clone
    smbdir_enum = smbdir.each
    dent_names.size.times do |n|
      dent = smbdir_enum.next
      assert_equal(dent.name, dent_names.delete(dent.name),
	  "Unexpected directory entry: #{dent.name}")
    end
    assert_raise(StopIteration) do
      smbdir_enum.next
    end
    assert_empty(dent_names)
    smbdir.close
  end ## test_dir_enum

  def file_time2libsmb_time(file_time)
    ## Discard fractional seconds
    smb_time = Time.at(file_time.to_i)
    ## Round up if fractional seconds is greater than 0.5
    smb_time += 1 if (file_time - smb_time >= 0.5)
    return smb_time
  end

  def test_smb_stat
    smb = self.smb

    smb.opendir(@@share_public) do |smbdir|
      while dent = smbdir.read
	next if (dent.name =~ /^\.\.?$/)

	smb_stat = smb.stat(dent.url)
	file_stat = File.stat(@@share_dir + '/' + dent.name)

	assert_kind_of(Integer, smb_stat.dev, "Net::SMB::Stat#dev #{dent.name}")
	assert_kind_of(Integer, smb_stat.ino, "Net::SMB::Stat#ino #{dent.name}")
	assert_kind_of(Integer, smb_stat.nlink, "Net::SMB::Stat#nlink #{dent.name}")
	#assert_equal(file_stat.mode, smb_stat.mode, "Net::SMB::Stat#mode #{dent.name}")
	assert_equal(file_stat.uid, smb_stat.uid, "Net::SMB::Stat#uid #{dent.name}")
	assert_equal(file_stat.gid, smb_stat.gid, "Net::SMB::Stat#gid #{dent.name}")
	if (dent.dir?)
	  assert_equal(0, smb_stat.size, "Net::SMB::Stat#size #{dent.name}")
	else
	  assert_equal(file_stat.size, smb_stat.size, "Net::SMB::Stat#size #{dent.name}")
	end
	assert_equal(file_time2libsmb_time(file_stat.atime), smb_stat.atime,
			"Net::SMB::Stat#atime #{dent.name}")
	assert_equal(file_time2libsmb_time(file_stat.mtime), smb_stat.mtime,
			"Net::SMB::Stat#mtime #{dent.name}")
	assert_equal(file_time2libsmb_time(file_stat.ctime), smb_stat.ctime,
			"Net::SMB::Stat#ctime #{dent.name}")
      end
    end
  end

  def test_smbfile_stat
    smb = self.smb

    smb.opendir(@@share_public) do |smbdir|
      while dent = smbdir.read
	next if (dent.name =~ /^\.\.?$/)
	next if (dent.name =~ /\.noaccess$/)

        smb_stat = nil
	if (dent.dir?)
	  smb.opendir(dent.url) do |smb_dir|
	    smb_stat = smb_dir.stat
	  end
	else
	  smb.open(dent.url) do |smb_file|
	    smb_stat = smb_file.stat
	  end
	end

	file_stat = File.stat(@@share_dir + '/' + dent.name)

	assert_kind_of(Integer, smb_stat.dev, "Net::SMB::Stat#dev #{dent.name}")
	assert_kind_of(Integer, smb_stat.ino, "Net::SMB::Stat#ino #{dent.name}")
	assert_kind_of(Integer, smb_stat.nlink, "Net::SMB::Stat#nlink #{dent.name}")
	#assert_equal(file_stat.mode, smb_stat.mode, "Net::SMB::Stat#mode #{dent.name}")
	assert_equal(file_stat.uid, smb_stat.uid, "Net::SMB::Stat#uid #{dent.name}")
	assert_equal(file_stat.gid, smb_stat.gid, "Net::SMB::Stat#gid #{dent.name}")
	if (dent.dir?)
	  assert_equal(0, smb_stat.size, "Net::SMB::Stat#size #{dent.name}")
	else
	  assert_equal(file_stat.size, smb_stat.size, "Net::SMB::Stat#size #{dent.name}")
	end
	assert_equal(file_time2libsmb_time(file_stat.atime), smb_stat.atime,
			"Net::SMB::Stat#atime #{dent.name}")
	assert_equal(file_time2libsmb_time(file_stat.mtime), smb_stat.mtime,
			"Net::SMB::Stat#mtime #{dent.name}")
	assert_equal(file_time2libsmb_time(file_stat.ctime), smb_stat.ctime,
			"Net::SMB::Stat#ctime #{dent.name}")
      end
    end
  end

  def test_smb_xattr
    smb = self.smb

    owner = "LOCALHOST\\#{@@username}"

    smb.opendir(@@share_public) do |smbdir|
      while dent = smbdir.read
	next if (dent.name =~ /^\.\.?$/)
	next if (dent.name =~ /\.noaccess$/)

        assert_equal(owner, smb.xattr(dent.url, 'system.nt_sec_desc.owner+'))
        assert_raise(Errno::EINVAL) do
          assert_equal(owner, smb.xattr(dent.url, 'invalid-xattr-name'))
        end
      end
    end
  end

  def test_file_open_read_close
    smb = self.smb

    @@files_readable.each do |filename|
      url = @@share_public + '/' + filename
      smbfile = smb.open(url)

      assert_equal(url, smbfile.url)

      assert_raise(ArgumentError) do
	smbfile.read(-1)
      end

      assert_equal(@@file_readable, smbfile.read)

      assert_equal(false, smbfile.closed?)
      smbfile.close
      assert_equal(true, smbfile.closed?)

      assert_raise(IOError) do
	smbfile.close
      end
      assert_raise(IOError) do
	smbfile.read(1)
      end
      assert_raise(IOError) do
	smbfile.pos
      end
      assert_raise(IOError) do
	smbfile.seek(0)
      end
      assert_raise(IOError) do
	smbfile.eof?
      end
    end
  end ## test_file_open_read_close

  def test_file_read_sequential
    smb = self.smb

    file = File.open(@@share_dir + '/' + @@file_large)
    smbfile = smb.open(@@share_public + '/' + @@file_large)

    buffer_size = smbfile.read_buffer_size
    [
      1..16,
      (buffer_size/3-16)..(buffer_size/3+16),
      (buffer_size/2-16)..(buffer_size/2+16),
      (buffer_size  -16)..(buffer_size  +16),
      (buffer_size*2-16)..(buffer_size*2+16),
      (buffer_size*3-16)..(buffer_size*3+16),
    ].each do |read_size_range| read_size_range.each do |read_size|
      file.rewind
      smbfile.rewind
      (buffer_size / read_size * 4).times do |n|
	file_data = file.read(read_size)
	smbfile_data = smbfile.read(read_size)
	assert_equal(file_data, smbfile_data, "read_size #{read_size}, n=#{n}")
	assert_equal(file.pos, smbfile.pos, "read_size #{read_size}, n=#{n}")
      end
    end end

    smbfile.close
  end ## test_file_read_sequential

  def test_file_read_random
    ## FIXME
  end ## test_file_read_random

  def test_file_read_eof
    smb = self.smb

    smbfile = smb.open(@@share_public + '/' + @@file_readable)

    assert_equal(false, smbfile.eof?)
    smbfile.read
    assert_equal(true, smbfile.eof?)

    assert_equal("", smbfile.read)
    assert_equal("", smbfile.read(0))
    assert_nil(smbfile.read(1))

    smbfile.close
  end ## test_file_read_eof

  def test_file_open_write_close
    ## FIXME
  end ## test_file_open_write_close

  def test_file_write_sequential
    ## FIXME
  end ## test_file_write_sequential

  def test_file_write_random
    ## FIXME
  end ## test_file_write_random
end

end

