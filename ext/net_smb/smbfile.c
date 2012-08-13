/*
 * Ruby/Net::SMB - SMB/CIFS client (Samba libsmbclient binding) for Ruby
 * Net::SMB::File class
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

#include <ruby/util.h>
#include <ruby/io.h>
#include <errno.h>

VALUE rb_cSMBFile;

/* ====================================================================== */

static void rb_smbfile_data_gc_mark(RB_SMBFILE_DATA *data)
{
  rb_gc_mark(data->smb_obj);
}

static void rb_smbfile_close_by_data(RB_SMBFILE_DATA *data)
{
  if (data->smbcfile == NULL) {
    rb_raise(rb_eIOError, "Closed file object");
  }

  smbc_close_fn fn = smbc_getFunctionClose(data->smbcctx);

  if ((*fn)(data->smbcctx, data->smbcfile) != 0) {
    rb_sys_fail("SMBC_close_ctx() failed");
  }
}

static void rb_smbfile_close_and_deref_by_data(RB_SMBFILE_DATA *data)
{
  rb_smbfile_close_by_data(data);

  data->smbcctx = NULL;
  data->smbcfile = NULL;

  DLIST_REMOVE(data->smb_data->smbfile_data_list, data);
}

static void rb_smbfile_data_free(RB_SMBFILE_DATA *data)
{
  RB_SMB_DEBUG("smbcctx=%p smbcfile=%p\n", data->smbcctx, data->smbcfile);

  if (data->smbcfile != NULL) {
    rb_smbfile_close_and_deref_by_data(data);
  }

  ruby_xfree(data->url);
  ruby_xfree(data->buffer);
  ruby_xfree(data);
}

static VALUE rb_smbfile_data_alloc(VALUE klass)
{
  RB_SMBFILE_DATA *data = ALLOC(RB_SMBFILE_DATA);

  memset(data, 0, sizeof(*data));

  data->smb_obj = Qnil;

  return Data_Wrap_Struct(klass, rb_smbfile_data_gc_mark, rb_smbfile_data_free, data);
}

static void rb_smbfile_open_by_data(RB_SMBFILE_DATA *data)
{
  smbc_open_fn fn = smbc_getFunctionOpen(data->smbcctx);

  data->smbcfile = (*fn)(data->smbcctx, data->url, data->oflags, 0);
  if (data->smbcfile == NULL) {
    rb_sys_fail("SMBC_open_ctx() failed");
  }

  RB_SMB_DEBUG("smbcctx=%p smbcfile=%p\n", data->smbcctx, data->smbcfile);
}

static void rb_smbfile_seek_by_data(RB_SMBFILE_DATA *data)
{
  smbc_lseek_fn fn = smbc_getFunctionLseek(data->smbcctx);

  if ((*fn)(data->smbcctx, data->smbcfile, data->pos, SEEK_SET) == -1) {
    rb_sys_fail("SMBC_lseek_ctx() failed");
  }

  data->buffer_used_size = 0;
  data->buffer_pos = 0;
  data->eof = 0;
}

static void rb_smbfile_reopen_by_data(RB_SMBFILE_DATA *data)
{
  rb_smbfile_close_by_data(data);
  rb_smbfile_open_by_data(data);
  rb_smbfile_seek_by_data(data);
}

static void rb_smbfile_read_by_data(RB_SMBFILE_DATA *data)
{
  smbc_read_fn fn;
  ssize_t read_size;
  char *buffer = data->buffer + data->buffer_used_size;
  size_t buffer_size = data->buffer_size - data->buffer_used_size;

  if (buffer_size == 0) {
    /* Buffer is full */
    if (data->buffer_pos < data->buffer_used_size) {
      /* But remained data exists */
      return;
    }

    /* Rewind */
    data->buffer_used_size = 0;
    data->buffer_pos = 0;
    buffer = data->buffer;
    buffer_size = data->buffer_size;
  }

  fn = smbc_getFunctionRead(data->smbcctx);

try:
  read_size = (*fn)(data->smbcctx, data->smbcfile, buffer, buffer_size);
  if (read_size < 0) {
    if (errno != EBADF) {
      rb_sys_fail("Bad SMBCFILE");
    }
    else {
      rb_smbfile_reopen_by_data(data);
      goto try;
    }
  }

  data->buffer_used_size += read_size;
  data->eof = (read_size == 0);
}

static VALUE rb_smbfile_close(VALUE self);

static VALUE rb_smbfile_initialize(int argc, VALUE *argv, VALUE self)
{
  RB_SMBFILE_DATA_FROM_OBJ(self, data);
  VALUE smb_obj, url_obj, mode_obj;

  rb_scan_args(argc, argv, "21", &smb_obj, &url_obj, &mode_obj);
  RB_SMB_DATA_FROM_OBJ(smb_obj, smb_data);

  if (NIL_P(mode_obj)) {
    data->fmode = FMODE_READABLE;
    // FIXME data->fmode = FMODE_READABLE | DEFAULT_TEXTMODE;
    data->oflags = O_RDONLY;
  }
  else if (FIXNUM_P(mode_obj)) {
    rb_raise(rb_eArgError, "FIXME");
    data->fmode = 0;
    data->oflags = NUM2INT(mode_obj);
  }
  else {
    const char *mode_str = StringValueCStr(mode_obj);
    data->fmode = rb_io_modestr_fmode(mode_str);
    data->oflags = rb_io_modestr_oflags(mode_str);
  }

  data->smb_obj = smb_obj;
  data->smb_data = smb_data;
  data->smbcctx = smb_data->smbcctx;
  data->url = ruby_strdup(RSTRING_PTR(url_obj));

  data->buffer = ruby_xmalloc(RB_SMBFILE_BUFFER_SIZE);
  data->buffer_size = RB_SMBFILE_BUFFER_SIZE;

  rb_smbfile_open_by_data(data);

  if (rb_block_given_p()) {
    return rb_ensure(rb_yield, self, rb_smbfile_close, self);
  }

  return self;
}

static VALUE rb_smbfile_smb(VALUE self)
{
  RB_SMBFILE_DATA_FROM_OBJ(self, data);

  return data->smb_obj;
}

static VALUE rb_smbfile_url(VALUE self)
{
  RB_SMBFILE_DATA_FROM_OBJ(self, data);

  return rb_str_new2(data->url);
}

static VALUE rb_smbfile_read_buffer_size(VALUE self)
{
  return INT2NUM(RB_SMBFILE_BUFFER_SIZE);
}

static VALUE rb_smbfile_close(VALUE self)
{
  RB_SMBFILE_DATA_FROM_OBJ(self, data);
  RB_SMBFILE_DATA_CLOSED(data);

  RB_SMB_DEBUG("data=%p smbcctx=%p smbcfile=%p\n", data, data->smbcctx, data->smbcfile);

  rb_smbfile_close_and_deref_by_data(data);

  return self;
}

static VALUE rb_smbfile_tell(VALUE self)
{
  RB_SMBFILE_DATA_FROM_OBJ(self, data);
  RB_SMBFILE_DATA_CLOSED(data);

  return SIZET2NUM(data->pos);
}

static VALUE rb_smbfile_seek(VALUE self, VALUE offset_num, VALUE whence_num)
{
  RB_SMBFILE_DATA_FROM_OBJ(self, data);
  RB_SMBFILE_DATA_CLOSED(data);
  off_t offset = NUM2OFFT(offset_num);
  int whence = NUM2INT(whence_num);

  switch (whence) {
  case SEEK_SET:
    data->pos = offset;
    break;
  case SEEK_CUR:
    if (offset < 0 && offset < -data->pos) {
      errno = EINVAL;
      rb_sys_fail("SMBC_lseek_ctx() failed");
    }
    data->pos += offset;
    break;
  case SEEK_END:
    rb_sys_fail("FIXME");
    break;
  default:
    rb_sys_fail("FIXME");
    break;
  }

  rb_smbfile_seek_by_data(data);

  return self;
}

static VALUE rb_smbfile_rewind(VALUE self)
{
  return rb_smbfile_seek(self, OFFT2NUM(0), INT2NUM(SEEK_SET));
}

static VALUE rb_smbfile_eof_p(VALUE self)
{
  RB_SMBFILE_DATA_FROM_OBJ(self, data);
  RB_SMBFILE_DATA_CLOSED(data);

  if (data->buffer_used_size - data->buffer_pos > 0) {
    /* Remained data exist in buffer */
    return Qfalse;
  }

  /* Try to read from file to buffer */
  rb_smbfile_read_by_data(data);

  if (data->buffer_used_size - data->buffer_pos > 0) {
    /* Remained data exist in buffer */
    return Qfalse;
  }

  return Qtrue;
}

static void rb_smbfile_readable_p_by_data(RB_SMBFILE_DATA *data)
{
  if (data->oflags & O_WRONLY) {
    rb_raise(rb_eIOError, "Not opened for reading");
  }
}

static VALUE rb_smbfile_read(int argc, VALUE *argv, VALUE self)
{
  RB_SMBFILE_DATA_FROM_OBJ(self, data);
  RB_SMBFILE_DATA_CLOSED(data);
  ssize_t req_read_size;
  VALUE str = rb_str_new2("");

  rb_smbfile_readable_p_by_data(data);

  if (argc == 0) {
    req_read_size = -1;
  }
  else {
    req_read_size = NUM2SSIZET(argv[0]);
    if (req_read_size == 0) {
      return str; /* Return empty string */
    }
    if (req_read_size < 0) {
      rb_raise(rb_eArgError, "Negative length given: %zd", req_read_size);
    }
  }

  while (req_read_size) {
    ssize_t buffer_read_size = data->buffer_used_size - data->buffer_pos;

    if (buffer_read_size == 0) {
      /* No remained data in buffer */
      rb_smbfile_read_by_data(data);

      if (data->eof) {
	/* No remained data in file */
	return (req_read_size > 0 && RSTRING_LEN(str) == 0) ? Qnil : str;
      }

      buffer_read_size = data->buffer_used_size - data->buffer_pos;
    }

    if (req_read_size > 0 && buffer_read_size > req_read_size) {
      buffer_read_size = req_read_size;
    }

    rb_str_cat(str, data->buffer + data->buffer_pos, buffer_read_size);

    data->pos += buffer_read_size;
    data->buffer_pos += buffer_read_size;
    if (req_read_size > 0) {
      req_read_size -= buffer_read_size;
    }
  }

  return str;
}

/* ====================================================================== */

void Init_net_smbfile(void)
{
  rb_cSMBFile = rb_define_class_under(rb_cSMB, "File", rb_cObject);
  rb_define_alloc_func(rb_cSMBFile, rb_smbfile_data_alloc);
  rb_define_method(rb_cSMBFile, "initialize", rb_smbfile_initialize, -1);
  rb_define_method(rb_cSMBFile, "smb", rb_smbfile_smb, 0);
  rb_define_method(rb_cSMBFile, "url", rb_smbfile_url, 0);
  rb_define_method(rb_cSMBFile, "read_buffer_size", rb_smbfile_read_buffer_size, 0);
  rb_define_method(rb_cSMBFile, "close", rb_smbfile_close, 0);
  rb_define_method(rb_cSMBFile, "tell", rb_smbfile_tell, 0);
  rb_define_alias(rb_cSMBFile, "pos", "tell");
  rb_define_method(rb_cSMBFile, "seek", rb_smbfile_seek, 2);
  rb_define_method(rb_cSMBFile, "rewind", rb_smbfile_rewind, 0);
  rb_define_method(rb_cSMBFile, "eof?", rb_smbfile_eof_p, 0);
  rb_define_method(rb_cSMBFile, "read", rb_smbfile_read, -1);
}

