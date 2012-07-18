/*
 * Ruby/Net::SMB - SMB/CIFS client (Samba libsmbclient binding) for Ruby
 * Net::SMB::DirEntry class
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

VALUE rb_cSMBDirEntry;

VALUE sym_name;
VALUE sym_type;
VALUE sym_url;
VALUE sym_comment;

/* ====================================================================== */

static VALUE rb_smbdirentry_initialize(VALUE self,
    VALUE name_obj, VALUE type_obj, VALUE url_obj, VALUE comment_obj)
{
  rb_call_super(0, 0);

  rb_hash_aset(self, sym_name, name_obj);
  rb_hash_aset(self, sym_type, type_obj);
  rb_hash_aset(self, sym_url, comment_obj);
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

/* ====================================================================== */

void Init_smbdirentry(void)
{
  rb_cSMBDirEntry = rb_define_class_under(rb_cSMB, "DirEntry", rb_cHash);
  rb_define_method(rb_cSMBDirEntry, "initialize", rb_smbdirentry_initialize, 4);
  rb_define_method(rb_cSMBDirEntry, "name", rb_smbdirentry_name, 0);
  rb_define_alias(rb_cSMBDirEntry, "to_s", "name");
  rb_define_method(rb_cSMBDirEntry, "type", rb_smbdirentry_type, 0);
  rb_define_method(rb_cSMBDirEntry, "url", rb_smbdirentry_url, 0);
  rb_define_method(rb_cSMBDirEntry, "comment", rb_smbdirentry_comment, 0);

  sym_name = ID2SYM(rb_intern("name"));
  sym_type = ID2SYM(rb_intern("type"));
  sym_url = ID2SYM(rb_intern("url"));
  sym_comment = ID2SYM(rb_intern("comment"));
}

