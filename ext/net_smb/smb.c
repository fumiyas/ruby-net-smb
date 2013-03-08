/*
 * Ruby/Net::SMB - SMB/CIFS client (Samba libsmbclient binding) for Ruby
 * Net::SMB class
 * Copyright (C) 2012 SATOH Fumiyas @ OSS Technology Corp., Japan
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "rb_smb.h"
#include "dlinklist.h"

VALUE rb_cSMB;
VALUE rb_eSMBError;

static ID id_call;

/* ====================================================================== */

static void smbcctx_auth_fn(SMBCCTX *smbcctx,
	const char *server, const char *share,
	char *workgroup, int wgmaxlen,
	char *username, int unmaxlen,
	char *password, int pwmaxlen)
{
  VALUE self = (VALUE)smbc_getOptionUserData(smbcctx);
  VALUE cred_obj;
  VALUE workgroup_obj;
  VALUE username_obj;
  VALUE password_obj;
  RB_SMB_DATA_FROM_OBJ(self, data);

  if (NIL_P(data->auth_callback)) {
    return;
  }

  cred_obj = rb_funcall(data->auth_callback,
    id_call, 2,
    rb_str_new2(server),
    rb_str_new2(share));

  if (TYPE(cred_obj) != T_ARRAY) {
    rb_raise(rb_eTypeError,
	RB_SMB_NAME
	"#auth_callback must return an array of username, "
	"passsword, and optional workgroup name");
  }
  if (RARRAY_LEN(cred_obj) < 2 || RARRAY_LEN(cred_obj) > 3) {
    rb_raise(rb_eArgError,
	RB_SMB_NAME
	"#auth_callback must return an array of username, "
	"passsword, and optional workgroup name");
  }

  username_obj = RARRAY_PTR(cred_obj)[0];
  password_obj = RARRAY_PTR(cred_obj)[1];
  workgroup_obj = (RARRAY_LEN(cred_obj) >= 3) ? RARRAY_PTR(cred_obj)[2] : Qnil;

  if (!NIL_P(workgroup_obj)) {
    SafeStringValue(workgroup_obj);
    if (RSTRING_LEN(workgroup_obj) > wgmaxlen - 1) {
      rb_raise(rb_eArgError, "Workgroup name too long");
    }
    strcpy(workgroup, RSTRING_PTR(workgroup_obj));
  }
  else {
    workgroup[0] = '\0';
  }
  if (!NIL_P(username_obj)) {
    SafeStringValue(username_obj);
    if (RSTRING_LEN(username_obj) > unmaxlen - 1) {
      rb_raise(rb_eArgError, "Username too long");
    }
    strcpy(username, RSTRING_PTR(username_obj));
  }
  else {
    username[0] = '\0';
  }
  if (!NIL_P(password_obj)) {
    SafeStringValue(password_obj);
    if (RSTRING_LEN(password_obj) > pwmaxlen - 1) {
      rb_raise(rb_eArgError, "Password too long");
    }
    strcpy(password, RSTRING_PTR(password_obj));
  }
  else {
    password[0] = '\0';
  }

  RB_SMB_DEBUG("\\%s\%s %s\%s|%s\n", server, share, workgroup, username, password);
}

static void rb_smb_data_gc_mark(RB_SMB_DATA *data)
{
  rb_gc_mark(data->auth_callback);
}

static void rb_smb_data_free(RB_SMB_DATA *data)
{
  RB_SMBFILE_DATA *smbfile_data;

  for (smbfile_data = data->smbfile_data_list; smbfile_data != NULL;
      smbfile_data = smbfile_data->next) {
    smbfile_data->smb_obj = Qnil;
    smbfile_data->smb_data = NULL;
    smbfile_data->smbcctx = NULL;
    smbfile_data->smbcfile = NULL;
  }
  smbc_free_context(data->smbcctx, 1);

  ruby_xfree(data);
}

static VALUE rb_smb_data_alloc(VALUE klass)
{
  VALUE data_obj;
  RB_SMB_DATA *data = ALLOC(RB_SMB_DATA);
  const char *home_backup = getenv("HOME");

  memset(data, 0, sizeof(*data));

  data_obj = Data_Wrap_Struct(klass, rb_smb_data_gc_mark, rb_smb_data_free, data);

  /* Unset $HOME to ignore $HOME/.smb/smb.conf */
  if (home_backup) {
    unsetenv("HOME");
  }
  data->smbcctx = smbc_new_context();
  if (home_backup) {
    setenv("HOME", home_backup, 1);
  }
  if (data->smbcctx == NULL) {
    rb_sys_fail("smbc_new_context() failed");
  }

  data->auth_callback = Qnil;

  return data_obj;
}

static VALUE rb_smb_initialize(VALUE self)
{
  RB_SMB_DATA_FROM_OBJ(self, data);

  /* FIXME: Take encoding from argument */
  /* FIXME: Read unix charset (?) from smb.conf for default encoding */
  data->enc = rb_enc_find("UTF-8");

  smbc_setDebug(data->smbcctx, 0);
  smbc_setOptionUserData(data->smbcctx, (void *)self);
  smbc_setOptionDebugToStderr(data->smbcctx, SMBC_TRUE);
  smbc_setOptionNoAutoAnonymousLogin(data->smbcctx,  SMBC_TRUE);
  smbc_setFunctionAuthDataWithContext(data->smbcctx, smbcctx_auth_fn);

  if (smbc_init_context(data->smbcctx) == NULL) {
    rb_sys_fail("smbc_init_context() failed");
  }

  return self;
}

static VALUE rb_smb_debug_get(VALUE self)
{
  RB_SMB_DATA_FROM_OBJ(self, data);

  return INT2NUM(smbc_getDebug(data->smbcctx));
}

static VALUE rb_smb_debug_set(VALUE self, VALUE debug)
{
  RB_SMB_DATA_FROM_OBJ(self, data);

  smbc_setDebug(data->smbcctx, NUM2INT(debug));

  return debug;
}

static VALUE rb_smb_use_kerberos_get(VALUE self)
{
  RB_SMB_DATA_FROM_OBJ(self, data);

  return SMBC2RB_BOOL(smbc_getOptionUseKerberos(data->smbcctx));
}

static VALUE rb_smb_use_kerberos_set(VALUE self, VALUE flag)
{
  RB_SMB_DATA_FROM_OBJ(self, data);

  smbc_setOptionUseKerberos(data->smbcctx, RB2SMBC_BOOL(flag));

  return flag;
}

static VALUE rb_smb_auth_callback(int argc, VALUE* argv, VALUE self)
{
  RB_SMB_DATA_FROM_OBJ(self, data);

  VALUE proc;
  VALUE block;

  if (argc == 0 && !rb_block_given_p()) {
    rb_raise(rb_eArgError, "No block or proc given");
  }
  else if (argc > 0 && rb_block_given_p()) {
    rb_raise(rb_eArgError, "Cannot use both block and proc");
  }

  rb_scan_args(argc, argv, "01&", &proc, &block);
  if (argc == 1) {
    data->auth_callback = proc;
  }
  else {
    data->auth_callback = block;
  }

  return Qnil;
}

static VALUE rb_smb_stat(VALUE self, VALUE url_obj)
{
  VALUE args[2];
  VALUE smbstat;

  args[0] = self;
  args[1] = url_obj;
  smbstat = rb_class_new_instance(2, args, rb_cSMBStat);

  return smbstat;
}

static VALUE rb_smb_opendir(VALUE self, VALUE url_obj)
{
  RB_SMB_DATA_FROM_OBJ(self, data);
  VALUE args[2];
  VALUE smbdir;

  args[0] = self;
  args[1] = url_obj;
  smbdir = rb_class_new_instance(2, args, rb_cSMBDir);

  RB_SMBFILE_DATA_FROM_OBJ(smbdir, smbfile_data);
  DLIST_ADD(data->smbfile_data_list, smbfile_data);

  return smbdir;
}

static VALUE rb_smb_open(int argc, VALUE *argv, VALUE self)
{
  RB_SMB_DATA_FROM_OBJ(self, data);
  VALUE argv_new[argc+1];
  VALUE smbfile;

  argv_new[0] = self;
  memcpy(argv_new + 1, argv, sizeof(*argv) * argc);
  smbfile = rb_class_new_instance(argc+1, argv_new, rb_cSMBFile);

  RB_SMBFILE_DATA_FROM_OBJ(smbfile, smbfile_data);
  DLIST_ADD(data->smbfile_data_list, smbfile_data);

  return smbfile;
}

VALUE rb_smb_xattr_get(VALUE self, VALUE url_obj, VALUE name_obj)
{
  RB_SMB_DATA_FROM_OBJ(self, data);
  const char *url = RSTRING_PTR(url_obj);
  const char *name = RSTRING_PTR(name_obj);
  char value[1024];

  smbc_getxattr_fn fn = smbc_getFunctionGetxattr(data->smbcctx);

  if ((*fn)(data->smbcctx, url, name, value, sizeof(value)) < 0) {
    rb_sys_fail("SMBC_getxattr_ctx() failed");
  }

  return rb_str_new2(value);
}

/* ====================================================================== */

void Init_net_smb(void)
{
  VALUE rb_mNet = rb_define_module("Net");

  /* Net::SMB */
  rb_cSMB = rb_define_class_under(rb_mNet, "SMB", rb_cObject);
  rb_define_alloc_func(rb_cSMB, rb_smb_data_alloc);
  rb_define_method(rb_cSMB, "initialize", rb_smb_initialize, 0);
  rb_define_method(rb_cSMB, "debug", rb_smb_debug_get, 0);
  rb_define_method(rb_cSMB, "debug=", rb_smb_debug_set, 1);
  rb_define_method(rb_cSMB, "use_kerberos", rb_smb_use_kerberos_get, 0);
  rb_define_method(rb_cSMB, "use_kerberos=", rb_smb_use_kerberos_set, 1);
  rb_define_method(rb_cSMB, "auth_callback", rb_smb_auth_callback, -1);
  rb_define_alias(rb_cSMB, "auth", "auth_callback");
  rb_define_method(rb_cSMB, "stat", rb_smb_stat, 1);
  rb_define_method(rb_cSMB, "opendir", rb_smb_opendir, 1);
  rb_define_method(rb_cSMB, "open", rb_smb_open, -1);
  rb_define_method(rb_cSMB, "xattr", rb_smb_xattr_get, 2);

  /* Net::SMB::Error */
  rb_eSMBError = rb_define_class_under(rb_cSMB, "Error", rb_eStandardError);

  id_call = rb_intern("call");

  const char *smbc_ver = smbc_version();
  int smbc_ver_major = atoi(smbc_ver);
  int smbc_ver_minor = (smbc_ver = strchr(smbc_ver, '.')) ? atoi(++smbc_ver) : 0;
  int smbc_ver_release = (smbc_ver = strchr(smbc_ver, '.')) ? atoi(++smbc_ver) : 0;
  long smbc_ver_number =
      (smbc_ver_major << 16) +
      (smbc_ver_minor << 8) +
      smbc_ver_release;

  if (smbc_ver_number <= 0x030606L) {
    /*
     * Hack to avoid Samba Bug 9038 - libsmbclient: SMBC_module_init()
     * does not init global parameters if $HOME is not set
     * https://bugzilla.samba.org/show_bug.cgi?id=9038
     */
    const char *home_backup = getenv("HOME");
    /* Unset $HOME to ignore $HOME/.smb/smb.conf */
    if (home_backup) {
      unsetenv("HOME");
    }
    SMBCCTX *smbcctx = smbc_new_context();
    if (home_backup) {
      setenv("HOME", home_backup, 1);
    }
    smbc_init_context(smbcctx);
  }

  Init_net_smbstat();
  Init_net_smbdir();
  Init_net_smbdirentry();
  Init_net_smbfile();
}

