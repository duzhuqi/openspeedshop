################################################################################
# Copyright (c) 2006-2013 Krell Institute All Rights Reserved.
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA
################################################################################

noinst_PROGRAMS = mutatee
noinst_SHARED_LIBRARIES = libwork
noinst_SHAREDdir=./

mutatee_CFLAGS = \
	-g -gdwarf-2 -O0

mutatee_LDADD = \
	-ldl

mutatee_SOURCES = \
	mutatee.c 

libwork_CFLAGS = \
	-g -gdwarf-2 -shared -O0 -fPIC

libwork_LDFLAGS = \
        -shared -o libwork.so

libwork_SOURCES = \
	libwork.c 
