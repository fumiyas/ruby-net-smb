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

#include <errno.h>

VALUE rb_cSMBDir;

struct rb_smbdir_data {
  VALUE         smb_obj;	/* Net::SMB object */
  SMBCCTX	*smbcctx;
  SMBCFILE	*smbcdir;
};

#define RB_SMBDIR_DATA_FROM_OBJ(obj, data) \
  struct rb_smbdir_data *data; \
  Data_Get_Struct(obj, struct rb_smbdir_data, data);

/* ====================================================================== */

static void rb_smbdir_data_gc_mark(struct rb_smbdir_data *data)
{
  rb_gc_mark(data->smb_obj);
}

static void rb_smbdir_data_free(struct rb_smbdir_data *data)
{
  if (data->smbcdir != NULL) {
    smbc_closedir_fn fn = smbc_getFunctionClosedir(data->smbcctx);

fprintf(stderr, "\n closedir %p %p\n",data->smbcctx, data->smbcdir);
    if ((*fn)(data->smbcctx, data->smbcdir) != 0) {
      rb_sys_fail("SMBC_closedir_ctx()");
    }
  }

  ruby_xfree(data);
}

static VALUE rb_smbdir_data_alloc(VALUE klass)
{
  struct rb_smbdir_data *data = ALLOC(struct rb_smbdir_data);

  memset(data, 0, sizeof(*data));

  data->smb_obj = Qnil;

  return Data_Wrap_Struct(klass, rb_smbdir_data_gc_mark, rb_smbdir_data_free, data);
}

static VALUE rb_smbdir_initialize(VALUE self, VALUE smb_obj, VALUE vurl)
{
  RB_SMB_DATA_FROM_OBJ(smb_obj, smb_data);
  RB_SMBDIR_DATA_FROM_OBJ(self, data);
  smbc_opendir_fn fn;
  const char *url = StringValuePtr(vurl);

  data->smb_obj = smb_obj;
  data->smbcctx = smb_data->smbcctx;

  fn = smbc_getFunctionOpendir(data->smbcctx);
  data->smbcdir = (*fn)(data->smbcctx, url);
fprintf(stderr, "\n xxxxxxx %p xxxxxxxxxxxx\n", data->smbcctx);
  if (data->smbcdir == NULL) {
fprintf(stderr, "\n 2 xxxxxxx %p xxxxxxxxxxxx\n", data->smbcctx);
    rb_sys_fail("SMBC_opendir_ctx()");
  }
fprintf(stderr, "\n 3 xxxxxxx %p xxxxxxxxxxxx\n", data->smbcctx);

  return self;
}

static VALUE rb_smbdir_close(VALUE self)
{
  RB_SMBDIR_DATA_FROM_OBJ(self, data);
  smbc_closedir_fn fn;

  fn = smbc_getFunctionClosedir(data->smbcctx);
  if ((*fn)(data->smbcctx, data->smbcdir) != 0) {
    rb_sys_fail("SMBC_closedir_ctx()");
  }

  data->smbcdir = NULL;

  return self;
}

static VALUE rb_smbdir_read(VALUE self)
{
  RB_SMBDIR_DATA_FROM_OBJ(self, data);
  smbc_readdir_fn fn;
  struct smbc_dirent *smbcdent;

  fn = smbc_getFunctionReaddir(data->smbcctx);

  errno = 0;
  smbcdent = (*fn)(data->smbcctx, data->smbcdir);

  if (smbcdent == NULL) {
    if (errno) {
      rb_sys_fail("SMBC_readdir_ctx()");
    }

    return Qnil;
  }

  return rb_str_new2(smbcdent->name);
}


/* ====================================================================== */

void Init_smbdir(void)
{
  rb_cSMBDir = rb_define_class_under(rb_cSMB, "Dir", rb_cObject);
  rb_define_alloc_func(rb_cSMBDir, rb_smbdir_data_alloc);
  rb_define_method(rb_cSMBDir, "initialize", rb_smbdir_initialize, 2);
  rb_define_method(rb_cSMBDir, "close", rb_smbdir_close, 0);
  rb_define_method(rb_cSMBDir, "read", rb_smbdir_read, 0);
}

