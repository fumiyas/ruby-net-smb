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
#include <libsmbclient.h>

#define TRUE_P(value)   ((value) == Qtrue)
#define FALSE_P(value)  (NIL_P(value) || (value) == Qfalse)

#define BOOL2VALUE(b)   ((b) ? Qtrue : Qfalse)
#define VALUE2BOOL(v)   (TRUE_P(v) ? 1 : 0)

#define SMBCCTX_TRUE	((smbc_bool)1)
#define SMBCCTX_FALSE	((smbc_bool)0)

#define _RB_SMB_H_

#endif /* _RB_SMB_H_ */

