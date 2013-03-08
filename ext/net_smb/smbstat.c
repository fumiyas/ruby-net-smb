/*
 * Ruby/Net::SMB - SMB/CIFS client (Samba libsmbclient binding) for Ruby
 * Net::SMB::Stat class
 * Copyright (C) 2012-2013 SATOH Fumiyas @ OSS Technology Corp., Japan
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

#ifndef NUM2DEVT
# define NUM2DEVT(v) NUM2UINT(v)
#endif
#ifndef DEVT2NUM
# define DEVT2NUM(v) UINT2NUM(v)
#endif

VALUE rb_cSMBStat;

static ID id_to_s;

/* ====================================================================== */

static void rb_smbstat_data_free(RB_SMBSTAT_DATA *data)
{
  ruby_xfree(data);
}

static VALUE rb_smbstat_data_alloc(VALUE klass)
{
  RB_SMBSTAT_DATA *data = ALLOC(RB_SMBSTAT_DATA);

  memset(data, 0, sizeof(*data));

  return Data_Wrap_Struct(klass, NULL, rb_smbstat_data_free, data);
}

static VALUE rb_smbstat_initialize(int argc, VALUE *argv, VALUE self)
{
  RB_SMBSTAT_DATA_FROM_OBJ(self, data);
  VALUE smb_or_file_obj;
  VALUE url_obj;

  rb_scan_args(argc, argv, "11", &smb_or_file_obj, &url_obj);

  if (rb_obj_is_kind_of(smb_or_file_obj, rb_cSMB)) {
    RB_SMB_DATA_FROM_OBJ(smb_or_file_obj, smb_data);

    if (rb_obj_is_kind_of(url_obj, rb_cString)) {
      /* OK */
    }
    else if (rb_respond_to(url_obj, id_to_s)) {
      url_obj = rb_funcall(url_obj, id_to_s, 0);
    }
    else {
      rb_raise(rb_eTypeError, "String was expected");
    }

    const char *url = StringValueCStr(url_obj);
    smbc_stat_fn fn = smbc_getFunctionStat(smb_data->smbcctx);
    if ((*fn)(smb_data->smbcctx, url, &data->stat)) {
      rb_sys_fail_str(url_obj);
    }
  }
  else if (rb_obj_is_kind_of(smb_or_file_obj, rb_cSMBFile)) {
    RB_SMBFILE_DATA_FROM_OBJ(smb_or_file_obj, smbfile_data);
    smbc_fstat_fn fn = smbc_getFunctionFstat(smbfile_data->smbcctx);

    if ((*fn)(smbfile_data->smbcctx, smbfile_data->smbcfile, &data->stat)) {
      rb_sys_fail(smbfile_data->url);
    }
  }
  else if (rb_obj_is_kind_of(smb_or_file_obj, rb_cSMBDir)) {
    RB_SMBFILE_DATA_FROM_OBJ(smb_or_file_obj, smbfile_data);
#if 0
    /*
     * SMBC_fstatdir_ctx() does nothing. See source/libsmb/libsmb_dir.c in
     * Samba source tree.
     */
    smbc_fstatdir_fn fn = smbc_getFunctionFstatdir(smbfile_data->smbcctx);

    if ((*fn)(smbfile_data->smbcctx, smbfile_data->smbcfile, &data->stat)) {
      rb_sys_fail(smbfile_data->url);
    }
#else
    smbc_stat_fn fn = smbc_getFunctionStat(smbfile_data->smbcctx);
    if ((*fn)(smbfile_data->smbcctx, smbfile_data->url, &data->stat)) {
      rb_sys_fail(smbfile_data->url);
    }
#endif
  }
  else {
    rb_raise(rb_eTypeError, "Net::SMB, Net::SMB::Dir or Net::SMB::File was expected");
  }

  return self;
}

static VALUE rb_smbstat_dev(VALUE self)
{
  RB_SMBSTAT_DATA_FROM_OBJ(self, data);

  return DEVT2NUM(data->stat.st_dev);
}

static VALUE rb_smbstat_ino(VALUE self)
{
  RB_SMBSTAT_DATA_FROM_OBJ(self, data);

  return ULL2NUM(data->stat.st_ino);
}

static VALUE rb_smbstat_mode(VALUE self)
{
  RB_SMBSTAT_DATA_FROM_OBJ(self, data);

  return UINT2NUM(data->stat.st_mode);
}

static VALUE rb_smbstat_nlink(VALUE self)
{
  RB_SMBSTAT_DATA_FROM_OBJ(self, data);

  return ULL2NUM(data->stat.st_nlink);
}

static VALUE rb_smbstat_uid(VALUE self)
{
  RB_SMBSTAT_DATA_FROM_OBJ(self, data);

  return UIDT2NUM(data->stat.st_uid);
}

static VALUE rb_smbstat_gid(VALUE self)
{
  RB_SMBSTAT_DATA_FROM_OBJ(self, data);

  return GIDT2NUM(data->stat.st_gid);
}

static VALUE rb_smbstat_size(VALUE self)
{
  RB_SMBSTAT_DATA_FROM_OBJ(self, data);

  return OFFT2NUM(data->stat.st_size);
}

static VALUE rb_smbstat_atime(VALUE self)
{
  RB_SMBSTAT_DATA_FROM_OBJ(self, data);

  return rb_time_new(data->stat.st_atime, 0);
}

static VALUE rb_smbstat_mtime(VALUE self)
{
  RB_SMBSTAT_DATA_FROM_OBJ(self, data);

  return rb_time_new(data->stat.st_mtime, 0);
}

static VALUE rb_smbstat_ctime(VALUE self)
{
  RB_SMBSTAT_DATA_FROM_OBJ(self, data);

  return rb_time_new(data->stat.st_ctime, 0);
}

/* ====================================================================== */

void Init_net_smbstat(void)
{
  rb_cSMBStat = rb_define_class_under(rb_cSMB, "Stat", rb_cObject);
  rb_define_alloc_func(rb_cSMBStat, rb_smbstat_data_alloc);
  rb_define_method(rb_cSMBStat, "initialize", rb_smbstat_initialize, -1);
  rb_define_method(rb_cSMBStat, "dev", rb_smbstat_dev, 0);
  rb_define_method(rb_cSMBStat, "ino", rb_smbstat_ino, 0);
  rb_define_method(rb_cSMBStat, "nlink", rb_smbstat_nlink, 0);
  rb_define_method(rb_cSMBStat, "mode", rb_smbstat_mode, 0);
  rb_define_method(rb_cSMBStat, "uid", rb_smbstat_uid, 0);
  rb_define_method(rb_cSMBStat, "gid", rb_smbstat_gid, 0);
  rb_define_method(rb_cSMBStat, "size", rb_smbstat_size, 0);
  rb_define_method(rb_cSMBStat, "atime", rb_smbstat_atime, 0);
  rb_define_method(rb_cSMBStat, "mtime", rb_smbstat_mtime, 0);
  rb_define_method(rb_cSMBStat, "ctime", rb_smbstat_ctime, 0);

  id_to_s = rb_intern("to_s");
}

