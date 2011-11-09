require 'mkmf'

## Perfer $LIBPATH to $DEFLIBPATH
alias original_try_link try_link
def try_link(*args)
  deflibpath_saved = $DEFLIBPATH
  $DEFLIBPATH = ["."]|$LIBPATH|$DEFLIBPATH
  ret = original_try_link(*args)
  $DEFLIBPATH = deflibpath_saved
  return ret
end

$CFLAGS += " " + ENV["CFLAGS"] if ENV["CFLAGS"]
$CPPFLAGS += " " + ENV["CPPFLAGS"] if ENV["CPPFLAGS"]
$LDFLAGS += " " + ENV["LDFLAGS"] if ENV["LDFLAGS"]

dir_config "smb"

h = have_header("libsmbclient.h")
l = have_library("smbclient", "smbc_new_context")
unless h && l
  exit 1
end

have_func("smbc_set_credentials")

## Samba 3.2.0+ hide AllowDebugChange variable.
if try_link(<<-'EOS')
    extern int AllowDebugChange;
    int main(int argc, char **argv) { AllowDebugChange = 0; }
  EOS
  $defs << "-DHAVE_SMBC_ALLOWDEBUGCHANGE"
end

create_makefile "smb"

