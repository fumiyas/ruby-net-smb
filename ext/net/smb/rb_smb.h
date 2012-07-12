/*
 * Ruby/Net::SMB - SMB/CIFS client (Samba libsmbclient binding) for Ruby
 * Common header for Net::SMB
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

#ifndef _RB_SMB_H_

#include <ruby.h>
#include <ruby/encoding.h>
#include <libsmbclient.h>

#ifdef RB_SMB_DEBUG
#  undef RB_SMB_DEBUG
#  define RB_SMB_DEBUG(fmt, ...) \
    do { \
      fprintf(stdout, "%s:%s:%d ", __FILE__, __func__, __LINE__); \
      fprintf(stdout, fmt, __VA_ARGS__); \
      fflush(stdout); \
    } while (0)
#else
#  define RB_SMB_DEBUG(fmt, ...)
#endif

#define SMBC_TRUE	((smbc_bool)1)
#define SMBC_FALSE	((smbc_bool)0)

#define TRUE_P(value)   ((value) == Qtrue)
#define FALSE_P(value)  (NIL_P(value) || (value) == Qfalse)

#define SMBC2RB_BOOL(b)   ((b) ? Qtrue : Qfalse)
#define RB2SMBC_BOOL(v)   (TRUE_P(v) ? SMBC_TRUE : SMBC_FALSE)

#define RB_SMB_NAME	"Net::SMB"
#define RB_SMBDIR_NAME	"Net::SMB::Dir"
#define RB_SMBFILE_NAME	"Net::SMB::File"

#define RB_SMBFILE_BUFFER_SIZE 8192

typedef struct rb_smb_data	RB_SMB_DATA;
typedef struct rb_smbfile_data	RB_SMBFILE_DATA;

struct rb_smbfile_data {
  rb_encoding	*enc;
  VALUE         smb_obj;	/* Net::SMB object */
  RB_SMB_DATA	*smb_data;
  SMBCCTX	*smbcctx;
  SMBCFILE	*smbcfile;
  char		*url;
  int		fmode;
  int		oflags;
  int		eof;
  off_t		pos;
  char		*buffer;
  size_t	buffer_size;
  size_t	buffer_used_size;
  size_t	buffer_pos;
  RB_SMBFILE_DATA *next, *prev;
};

struct rb_smb_data {
  rb_encoding	*enc;
  SMBCCTX	*smbcctx;
  VALUE		auth_callback;
  RB_SMBFILE_DATA *smbfile_data_list;
};

#define RB_SMB_DATA_FROM_OBJ(obj, data) \
  RB_SMB_DATA *data; \
  Data_Get_Struct(obj, RB_SMB_DATA, data);

#define RB_SMBFILE_DATA_FROM_OBJ(obj, data) \
  RB_SMBFILE_DATA *data; \
  Data_Get_Struct(obj, RB_SMBFILE_DATA, data);

extern VALUE rb_cSMB;
extern VALUE rb_eSMBError;
extern VALUE rb_cSMBDir;
extern VALUE rb_cSMBFile;

void Init_smbdir(void);
void Init_smbfile(void);

#define _RB_SMB_H_

#endif /* _RB_SMB_H_ */

