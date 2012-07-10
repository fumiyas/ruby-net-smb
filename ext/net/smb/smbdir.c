/*
 * Ruby/Net::SMB - SMB/CIFS client (Samba libsmbclient binding) for Ruby
 * Net::SMB::Dir class
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

#include <errno.h>

VALUE rb_cSMBDir;

/* ====================================================================== */

static void rb_smbdir_data_gc_mark(RB_SMBFILE_DATA *data)
{
  rb_gc_mark(data->smb_obj);
}

static void rb_smbdir_close_by_data(RB_SMBFILE_DATA *data)
{
  if (data->smbcfile == NULL) {
    rb_raise(rb_eIOError, "Closed directory object");
  }

  smbc_closedir_fn fn = smbc_getFunctionClosedir(data->smbcctx);

  if ((*fn)(data->smbcctx, data->smbcfile) != 0) {
    rb_sys_fail("SMBC_closedir_ctx() failed");
  }
}

static void rb_smbdir_close_and_deref_by_data(RB_SMBFILE_DATA *data)
{
  rb_smbdir_close_by_data(data);

  data->smbcctx = NULL;
  data->smbcfile = NULL;

  DLIST_REMOVE(data->smb_data->smbfile_data_list, data);
}

static void rb_smbdir_data_free(RB_SMBFILE_DATA *data)
{
  if (data->smbcfile != NULL) {
    rb_smbdir_close_and_deref_by_data(data);
  }

  ruby_xfree(data);
}

static VALUE rb_smbdir_data_alloc(VALUE klass)
{
  RB_SMBFILE_DATA *data = ALLOC(RB_SMBFILE_DATA);

  memset(data, 0, sizeof(*data));

  data->smb_obj = Qnil;

  return Data_Wrap_Struct(klass, rb_smbdir_data_gc_mark, rb_smbdir_data_free, data);
}

static VALUE rb_smbdir_initialize(VALUE self, VALUE smb_obj, VALUE url_obj)
{
  RB_SMBFILE_DATA_FROM_OBJ(self, data);
  RB_SMB_DATA_FROM_OBJ(smb_obj, smb_data);
  smbc_opendir_fn fn;
  const char *url = StringValueCStr(url_obj);

  /* FIXME: Take encoding from argument */
  /* FIXME: Read unix charset (?) from smb.conf for default encoding */
  data->enc = rb_enc_find("UTF-8");

  data->smb_obj = smb_obj;
  data->smb_data = smb_data;
  data->smbcctx = smb_data->smbcctx;

  fn = smbc_getFunctionOpendir(data->smbcctx);
  data->smbcfile = (*fn)(data->smbcctx, url);
  if (data->smbcfile == NULL) {
    rb_sys_fail("SMBC_opendir_ctx() failed");
  }

  return self;
}

static VALUE rb_smbdir_smb(VALUE self)
{
  RB_SMBFILE_DATA_FROM_OBJ(self, data);

  return data->smb_obj;
}

static VALUE rb_smbdir_close(VALUE self)
{
  RB_SMBFILE_DATA_FROM_OBJ(self, data);

  rb_smbdir_close_and_deref_by_data(data);

  return self;
}

static VALUE rb_smbdir_tell(VALUE self)
{
  RB_SMBFILE_DATA_FROM_OBJ(self, data);
  smbc_telldir_fn fn;
  off_t offset;

  fn = smbc_getFunctionTelldir(data->smbcctx);

  errno = 0;
  offset = (*fn)(data->smbcctx, data->smbcfile);
  if (offset == (off_t)-1) {
    if (errno != 0) {
      rb_sys_fail("SMBC_telldir_ctx() failed");
    }
  }

  return LONG2NUM(offset);
}

static VALUE rb_smbdir_seek(VALUE self, VALUE offset_num)
{
  RB_SMBFILE_DATA_FROM_OBJ(self, data);
  smbc_lseekdir_fn fn;
  off_t offset = (off_t)NUM2LONG(offset_num);

  fn = smbc_getFunctionLseekdir(data->smbcctx);

  errno = 0;
  if ((*fn)(data->smbcctx, data->smbcfile, offset) == -1) {
    rb_sys_fail("SMBC_lseekdir_ctx() failed");
  }

  return self;
}

static VALUE rb_smbdir_rewind(VALUE self)
{
  return rb_smbdir_seek(self, LONG2NUM(0));
}

static VALUE rb_smbdir_read(VALUE self)
{
  RB_SMBFILE_DATA_FROM_OBJ(self, data);
  smbc_readdir_fn fn;
  struct smbc_dirent *smbcdent;

  fn = smbc_getFunctionReaddir(data->smbcctx);

  errno = 0;
  smbcdent = (*fn)(data->smbcctx, data->smbcfile);

  if (smbcdent == NULL) {
    if (errno) {
      rb_sys_fail("SMBC_readdir_ctx() failed");
    }

    return Qnil;
  }

  return rb_external_str_new_with_enc(smbcdent->name, strlen(smbcdent->name), data->enc);
}

static VALUE rb_smbdir_each(VALUE self)
{
  VALUE name;

  RETURN_ENUMERATOR(self, 0, 0);

  while (!NIL_P(name = rb_smbdir_read(self))) {
    rb_yield(name);
  }

  return self;
}

/* ====================================================================== */

void Init_smbdir(void)
{
  rb_cSMBDir = rb_define_class_under(rb_cSMB, "Dir", rb_cObject);
  rb_define_alloc_func(rb_cSMBDir, rb_smbdir_data_alloc);
  rb_define_method(rb_cSMBDir, "initialize", rb_smbdir_initialize, 2);
  rb_define_method(rb_cSMBDir, "smb", rb_smbdir_smb, 0);
  rb_define_method(rb_cSMBDir, "close", rb_smbdir_close, 0);
  rb_define_method(rb_cSMBDir, "tell", rb_smbdir_tell, 0);
  rb_define_alias(rb_cSMBDir, "pos", "tell");
  rb_define_method(rb_cSMBDir, "seek", rb_smbdir_seek, 1);
  rb_define_method(rb_cSMBDir, "rewind", rb_smbdir_rewind, 0);
  rb_define_method(rb_cSMBDir, "read", rb_smbdir_read, 0);
  rb_define_method(rb_cSMBDir, "each", rb_smbdir_each, 0);
}

