/*
 * Ruby/Net::SMB - SMB/CIFS client (Samba libsmbclient binding) for Ruby
 * Net::SMB::DirEntry class
 * Copyright (C) 2012-2013 SATOH Fumiyasu @ OSS Technology Corp., Japan
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

VALUE rb_cSMBDirEntry;

static VALUE sym_name;
static VALUE sym_type;
static VALUE sym_url;
static VALUE sym_comment;

/* ====================================================================== */

static VALUE rb_smbdirentry_initialize(VALUE self,
    VALUE name_obj, VALUE type_obj, VALUE url_obj, VALUE comment_obj)
{
  rb_call_super(0, 0);

  rb_hash_aset(self, sym_name, name_obj);
  rb_hash_aset(self, sym_type, type_obj);
  rb_hash_aset(self, sym_url, url_obj);
  rb_hash_aset(self, sym_comment, comment_obj);

  return self;
}

static VALUE rb_smbdirentry_name(VALUE self)
{
  return rb_hash_lookup(self, sym_name);
}

static VALUE rb_smbdirentry_type(VALUE self)
{
  return rb_hash_lookup(self, sym_type);
}

static VALUE rb_smbdirentry_url(VALUE self)
{
  return rb_hash_lookup(self, sym_url);
}

static VALUE rb_smbdirentry_comment(VALUE self)
{
  return rb_hash_lookup(self, sym_comment);
}

static VALUE rb_smbdirentry_workgroup_p(VALUE self)
{
  return rb_hash_lookup(self, sym_type) == INT2FIX(SMBC_WORKGROUP) ?
      Qtrue : Qfalse;
}

static VALUE rb_smbdirentry_server_p(VALUE self)
{
  return rb_hash_lookup(self, sym_type) == INT2FIX(SMBC_SERVER) ?
      Qtrue : Qfalse;
}

static VALUE rb_smbdirentry_file_share_p(VALUE self)
{
  return rb_hash_lookup(self, sym_type) == INT2FIX(SMBC_FILE_SHARE) ?
      Qtrue : Qfalse;
}

static VALUE rb_smbdirentry_printer_share_p(VALUE self)
{
  return rb_hash_lookup(self, sym_type) == INT2FIX(SMBC_PRINTER_SHARE) ?
      Qtrue : Qfalse;
}

static VALUE rb_smbdirentry_comms_share_p(VALUE self)
{
  return rb_hash_lookup(self, sym_type) == INT2FIX(SMBC_COMMS_SHARE) ?
      Qtrue : Qfalse;
}

static VALUE rb_smbdirentry_ipc_share_p(VALUE self)
{
  return rb_hash_lookup(self, sym_type) == INT2FIX(SMBC_IPC_SHARE) ?
      Qtrue : Qfalse;
}

static VALUE rb_smbdirentry_dir_p(VALUE self)
{
  return rb_hash_lookup(self, sym_type) == INT2FIX(SMBC_DIR) ?
      Qtrue : Qfalse;
}

static VALUE rb_smbdirentry_file_p(VALUE self)
{
  return rb_hash_lookup(self, sym_type) == INT2FIX(SMBC_FILE) ?
      Qtrue : Qfalse;
}

static VALUE rb_smbdirentry_link_p(VALUE self)
{
  return rb_hash_lookup(self, sym_type) == INT2FIX(SMBC_LINK) ?
      Qtrue : Qfalse;
}

/* ====================================================================== */

void Init_net_smbdirentry(void)
{
  rb_cSMBDirEntry = rb_define_class_under(rb_cSMB, "DirEntry", rb_cHash);
  rb_define_method(rb_cSMBDirEntry, "initialize", rb_smbdirentry_initialize, 4);
  rb_define_method(rb_cSMBDirEntry, "name", rb_smbdirentry_name, 0);
  rb_define_alias(rb_cSMBDirEntry, "to_s", "name");
  rb_define_method(rb_cSMBDirEntry, "type", rb_smbdirentry_type, 0);
  rb_define_method(rb_cSMBDirEntry, "url", rb_smbdirentry_url, 0);
  rb_define_method(rb_cSMBDirEntry, "comment", rb_smbdirentry_comment, 0);
  rb_define_method(rb_cSMBDirEntry, "workgroup?", rb_smbdirentry_workgroup_p, 0);
  rb_define_method(rb_cSMBDirEntry, "server?", rb_smbdirentry_server_p, 0);
  rb_define_method(rb_cSMBDirEntry, "file_share?", rb_smbdirentry_file_share_p, 0);
  rb_define_method(rb_cSMBDirEntry, "printer_share?", rb_smbdirentry_printer_share_p, 0);
  rb_define_method(rb_cSMBDirEntry, "comms_share?", rb_smbdirentry_comms_share_p, 0);
  rb_define_method(rb_cSMBDirEntry, "ipc_share?", rb_smbdirentry_ipc_share_p, 0);
  rb_define_method(rb_cSMBDirEntry, "directory?", rb_smbdirentry_dir_p, 0);
  rb_define_alias(rb_cSMBDirEntry, "dir?", "directory?");
  rb_define_method(rb_cSMBDirEntry, "file?", rb_smbdirentry_file_p, 0);
  rb_define_method(rb_cSMBDirEntry, "link?", rb_smbdirentry_link_p, 0);

  sym_name = ID2SYM(rb_intern("name"));
  sym_type = ID2SYM(rb_intern("type"));
  sym_url = ID2SYM(rb_intern("url"));
  sym_comment = ID2SYM(rb_intern("comment"));
}

