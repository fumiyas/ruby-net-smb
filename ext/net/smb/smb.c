/*
 * Ruby/Net::SMB - SMB/CIFS client (Samba libsmbclient binding) for Ruby
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

static VALUE rb_smb_eError;

struct rb_smb_data {
  SMBCCTX       *smbcctx;
  VALUE         auth_callback;
};

#define RB_SMB_DATA_FROM_SELF(self, data) \
  struct rb_smb_data *data; \
  Data_Get_Struct(self, struct rb_smb_data, data);

/* ====================================================================== */

static void smbcctx_auth_fn(SMBCCTX *smbcctx,
	const char *server, const char *share,
	char *workgroup, int wgmaxlen,
	char *username, int unmaxlen,
	char *password, int pwmaxlen)
{
  VALUE self = (VALUE)smbc_getOptionUserData(smbcctx);
  VALUE ary;
  VALUE wg;
  VALUE un;
  VALUE pw;
  RB_SMB_DATA_FROM_SELF(self, data);

  if (NIL_P(data->auth_callback)) {
    return;
  }

  ary = rb_funcall(data->auth_callback,
    rb_intern("call"), 2,
    rb_str_new2(server),
    rb_str_new2(share));

  if (TYPE(ary) != T_ARRAY) {
    return;
  }
  if (RARRAY_LEN(ary) < 2 || RARRAY_LEN(ary) > 3) {
    rb_raise(rb_eArgError,
	"Array should contain username, passsword, and optional workgroup name");
  }

  un = RARRAY_PTR(ary)[0];
  pw = RARRAY_PTR(ary)[1];
  wg = (RARRAY_LEN(ary) >= 3) ? RARRAY_PTR(ary)[2] : Qnil;

  if (!NIL_P(wg)) {
    SafeStringValue(wg);
    if (RSTRING_LEN(wg) > wgmaxlen - 1) {
      rb_raise(rb_eArgError, "Workgroup too long");
    }
    strcpy(workgroup, RSTRING_PTR(wg));
  }
  else {
    strcpy(workgroup, "");
  }
  if (!NIL_P(un)) {
    SafeStringValue(un);
    if (RSTRING_LEN(un) > unmaxlen - 1) {
      rb_raise(rb_eArgError, "Username too long");
    }
    strcpy(username, RSTRING_PTR(un));
  }
  else {
    strcpy(username, "");
  }
  if (!NIL_P(pw)) {
    SafeStringValue(pw);
    if (RSTRING_LEN(pw) > pwmaxlen - 1) {
      rb_raise(rb_eArgError, "Password too long");
    }
    strcpy(password, RSTRING_PTR(pw));
  }
  else {
    strcpy(password, "");
  }
}

static void rb_smb_data_gc_mark(struct rb_smb_data *data)
{
  rb_gc_mark(data->auth_callback);
}

static void rb_smb_data_free(struct rb_smb_data *data)
{
  smbc_free_context(data->smbcctx, 1);
  ruby_xfree(data);
}

static VALUE rb_smb_data_alloc(VALUE klass)
{
  struct rb_smb_data *data = ALLOC(struct rb_smb_data);

  memset(data, 0, sizeof(*data));

  /* FIXME: Unset $HOME to ignore $HOME/.smb/smb.conf */
  data->smbcctx = smbc_new_context();
  if (data->smbcctx == NULL) {
    rb_sys_fail("smbc_new_context()");
  }

  data->auth_callback = Qnil;

  return Data_Wrap_Struct(klass, rb_smb_data_gc_mark, rb_smb_data_free, data);
}

static VALUE rb_smb_initialize(VALUE self)
{
  RB_SMB_DATA_FROM_SELF(self, data);

  smbc_setDebug(data->smbcctx, 0);
  smbc_setOptionUserData(data->smbcctx, (void *)self);
  smbc_setOptionDebugToStderr(data->smbcctx, SMBCCTX_TRUE);
  smbc_setOptionNoAutoAnonymousLogin(data->smbcctx,  SMBCCTX_TRUE);
  smbc_setFunctionAuthDataWithContext(data->smbcctx, smbcctx_auth_fn);

  if (smbc_init_context(data->smbcctx) == NULL) {
    rb_sys_fail("smbc_init_context()");
  }

  return self;
}

static VALUE rb_smb_debug_get(VALUE self)
{
  RB_SMB_DATA_FROM_SELF(self, data);

  return INT2NUM(smbc_getDebug(data->smbcctx));
}

static VALUE rb_smb_debug_set(VALUE self, VALUE debug)
{
  RB_SMB_DATA_FROM_SELF(self, data);

  smbc_setDebug(data->smbcctx, NUM2INT(debug));

  return debug;
}

static VALUE rb_smb_use_kerberos_get(VALUE self)
{
  RB_SMB_DATA_FROM_SELF(self, data);

  return smbc_getOptionUseKerberos(data->smbcctx) ? Qtrue : Qfalse;
}

static VALUE rb_smb_use_kerberos_set(VALUE self, VALUE flag)
{
  RB_SMB_DATA_FROM_SELF(self, data);

  smbc_setOptionUseKerberos(data->smbcctx, TRUE_P(flag) ? SMBCCTX_TRUE : SMBCCTX_FALSE);

  return flag;
}

static VALUE rb_smb_on_auth(int argc, VALUE* argv, VALUE self)
{
  RB_SMB_DATA_FROM_SELF(self, data);

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

/* ====================================================================== */

void Init_smb(void)
{
  VALUE rb_mNet = rb_define_module("Net");
  /* Net::SMB */
  VALUE rb_cSMB = rb_define_class_under(rb_mNet, "SMB", rb_cObject);

  /* Net::SMB::Error */
  rb_smb_eError = rb_define_class_under(rb_cSMB, "Error", rb_eStandardError);

  /* Net::SMB */
  rb_define_alloc_func(rb_cSMB, rb_smb_data_alloc);
  rb_define_method(rb_cSMB, "initialize", rb_smb_initialize, 0);
  rb_define_method(rb_cSMB, "debug", rb_smb_debug_get, 0);
  rb_define_method(rb_cSMB, "debug=", rb_smb_debug_set, 1);
  rb_define_method(rb_cSMB, "use_kerberos", rb_smb_use_kerberos_get, 0);
  rb_define_method(rb_cSMB, "use_kerberos=", rb_smb_use_kerberos_set, 1);
  rb_define_method(rb_cSMB, "on_auth", rb_smb_on_auth, -1);
  rb_define_alias(rb_cSMB, "on_authentication", "on_auth");

}

