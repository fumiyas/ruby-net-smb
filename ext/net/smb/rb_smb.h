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

#define TRUE_P(value)   ((value) == Qtrue)
#define FALSE_P(value)  (NIL_P(value) || (value) == Qfalse)

#define BOOL2VALUE(b)   ((b) ? Qtrue : Qfalse)
#define VALUE2BOOL(v)   (TRUE_P(v) ? 1 : 0)

#define SMBCCTX_TRUE	((smbc_bool)1)
#define SMBCCTX_FALSE	((smbc_bool)0)

#define RB_SMB_NAME	"Net::SMB"
#define RB_SMBDIR_NAME	"Net::SMB::Dir"
#define RB_SMBFILE_NAME	"Net::SMB::File"

typedef struct rb_smb_data	RB_SMB_DATA;
typedef struct rb_smbfile_data	RB_SMBFILE_DATA;

struct rb_smbfile_data {
  rb_encoding	*enc;
  VALUE         smb_obj;	/* Net::SMB object */
  SMBCCTX	*smbcctx;
  SMBCFILE	*smbcfile;
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
extern VALUE rb_cSMBFile;
extern VALUE rb_cSMBDir;
extern VALUE rb_eSMBError;

void Init_smbfile(void);
void Init_smbdir(void);

#define _RB_SMB_H_

#endif /* _RB_SMB_H_ */

