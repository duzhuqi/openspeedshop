################################################################################
# Copyright (c) 2007 Krell Institute  All Rights Reserved.
# Copyright (c) 2007 William Hachfeld. All Rights Reserved.
#
# This library is free software; you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation; either version 2.1 of the License, or (at your option)
# any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
################################################################################

# Note: See note in "libopenss-runtime/Makefile.am" for why "-O0" is used here.

CFLAGS = -g -O0

BUILT_SOURCES = \
	blobs.h blobs.c	

#noinst_LIBRARIES = libvt.mpi.a

if HAVE_LAMPI
libvt-lampi-mpi-a :
	cp -r @VT_LIBSDIR@/libvt.mpi.lampi.a libvt.mpi.lampi.a
	ar d libvt.mpi.lampi.a libvt_mpi_lampi_a-vt_mpiwrap.o
endif

if HAVE_OPENMPI
libvt-openmpi-mpi-a:
	cp -r @VT_LIBSDIR@/libvt.mpi.openmpi.a libvt.mpi.openmpi.a
	ar d libvt.mpi.openmpi.a libvt_mpi_openmpi_a-vt_mpiwrap.o
endif

if HAVE_MPICH2
libvt-mpich2-mpi-a:
	cp -r @VT_LIBSDIR@/libvt.mpi.mpich2.a libvt.mpi.mpich2.a
	ar d libvt.mpi.mpich2.a libvt_mpi_mpich2_a-vt_mpiwrap.o
endif

if HAVE_MPICH
libvt-mpich-mpi-a:
	cp -r @VT_LIBSDIR@/libvt.mpi.mpich.a libvt.mpi.mpich.a
	ar d libvt.mpi.mpich.a libvt_mpi_mpich_a-vt_mpiwrap.o
endif

if HAVE_LAM
libvt-lam-mpi-a:
	cp -r @VT_LIBSDIR@/libvt.mpi.lam.a libvt.mpi.lam.a
	ar d libvt.mpi.lam.a libvt_mpi_lam_a-vt_mpiwrap.o
endif

if HAVE_MPT
libvt-mpt-mpi-a:
	cp -r @VT_LIBSDIR@/libvt.mpi.mpt.a libvt.mpi.mpt.a
	ar d libvt.mpi.mpt.a libvt_mpi_mpt_a-vt_mpiwrap.o
endif

pkglib_LTLIBRARIES = mpiotf.la
if HAVE_LAM
pkglib_LTLIBRARIES += mpiotf-rt-lam.la
endif
if HAVE_LAMPI
pkglib_LTLIBRARIES += mpiotf-rt-lampi.la
endif
if HAVE_MPICH
pkglib_LTLIBRARIES += mpiotf-rt-mpich.la
endif
if HAVE_MPICH2
pkglib_LTLIBRARIES += mpiotf-rt-mpich2.la
endif
if HAVE_MPT
pkglib_LTLIBRARIES += mpiotf-rt-mpt.la
endif
if HAVE_OPENMPI
pkglib_LTLIBRARIES += mpiotf-rt-openmpi.la
endif

mpiotf_la_CXXFLAGS = \
	-I$(top_srcdir)/libopenss-framework

mpiotf_la_LDFLAGS = \
	-L$(top_srcdir)/libopenss-framework \
	-no-undefined -module -avoid-version

mpiotf_la_LIBADD = \
	-lopenss-framework

mpiotf_la_SOURCES = \
	blobs.x $(BUILT_SOURCES) \
	MPIOTFCollector.hxx MPIOTFCollector.cxx \
	MPIOTFDetail.hxx

# Common definitions for all MPI implementations
#
# See comments in plugins/collectors/mpi/Makefile.am.  Remarks there
# concerning the generated wrapper sources also apply to wrappers.c
# here.

rt_CFLAGS_generate = \
	-I$(top_srcdir)/libopenss-runtime \
	@VT_CPPFLAGS@ \
	$(1) \
        @OTF_CPPFLAGS@ \
	@LIBUNWIND_CPPFLAGS@

rt_LDFLAGS_generate = \
	-L$(top_srcdir)/libopenss-runtime \
	$(1) \
	@LIBUNWIND_LDFLAGS@ \
	-no-undefined -avoid-version

rt_LIBADD_generate = \
	-lopenss-runtime \
	$(1) \
	@LIBUNWIND_LIBS@ \
	-lpthread \
	-L$(top_srcdir)/plugins/collectors/mpiotf \
        @OTF_LDFLAGS@ \
        @OTF_LIBS@ \
        @OTF_LIBZ_LDFLAGS@ \
	@OTF_LIBZ_LIBS@ \
	-lrt 

rt_sources = \
	blobs.x $(BUILT_SOURCES) \
	runtime.h runtime.c  \
	vt_mpiwrap.c 


rt_openmpi_LIBADD_generate = \
	-lopenss-runtime \
	$(1) \
	@LIBUNWIND_LIBS@ \
	-lpthread \
	@OPENMPI_LDFLAGS@ @OPENMPI_LIBS@ \
	-L./ -lvt.mpi.openmpi  \
	@OPENMPI_LDFLAGS@ @OPENMPI_LIBS@ \
	@PAPI_LDFLAGS@ @PAPI_LIBS@ \
        @OTF_LDFLAGS@ \
        @OTF_LIBS@ \
        @OTF_LIBZ_LDFLAGS@ \
	@OTF_LIBZ_LIBS@ \
	-lrt 


rt_lam_LIBADD_generate = \
	-lopenss-runtime \
	$(1) \
	@LIBUNWIND_LIBS@ \
	-lpthread \
	@LAM_LDFLAGS@ @LAM_LIBS@ \
	@PAPI_LDFLAGS@ @PAPI_LIBS@ \
	-L./ -lvt.mpi.lam  \
	@PAPI_LDFLAGS@ @PAPI_LIBS@ \
	@LAM_LDFLAGS@ @LAM_LIBS@ \
        @OTF_LDFLAGS@ \
        @OTF_LIBS@ \
        @OTF_LIBZ_LDFLAGS@ \
	@OTF_LIBZ_LIBS@ \
	-lrt 

rt_lampi_LIBADD_generate = \
	-lopenss-runtime \
	$(1) \
	@LIBUNWIND_LIBS@ \
	-lpthread \
	-L./ -lvt.mpi.lampi  \
	@PAPI_LDFLAGS@ @PAPI_LIBS@ \
	@LAMPI_LDFLAGS@ @LAMPI_LIBS@ \
        @OTF_LDFLAGS@ \
        @OTF_LIBS@ \
        @OTF_LIBZ_LDFLAGS@ \
	@OTF_LIBZ_LIBS@ \
	-lrt 


rt_mpich_LIBADD_generate = \
	-lopenss-runtime \
	$(1) \
	@LIBUNWIND_LIBS@ \
	-lpthread \
	-L./ -lvt.mpi.mpich  \
        @OTF_LDFLAGS@ \
        @OTF_LIBS@ \
        @OTF_LIBZ_LDFLAGS@ \
	@OTF_LIBZ_LIBS@ \
	-lrt 

rt_mpich2_LIBADD_generate = \
	-lopenss-runtime \
	$(1) \
	@LIBUNWIND_LIBS@ \
	-lpthread \
	-L./ -lvt.mpi.mpich2  \
        @OTF_LDFLAGS@ \
        @OTF_LIBS@ \
        @OTF_LIBZ_LDFLAGS@ \
	@OTF_LIBZ_LIBS@ \
	-lrt 

rt_mpt_LIBADD_generate = \
	-lopenss-runtime \
	$(1) \
	@LIBUNWIND_LIBS@ \
	-lpthread \
	-L./ -lvt.mpi.mpt  \
        @OTF_LDFLAGS@ \
        @OTF_LIBS@ \
        @OTF_LIBZ_LDFLAGS@ \
	@OTF_LIBZ_LIBS@ \
	-lrt 

if HAVE_LAM
mpiotf_rt_lam_la_CFLAGS = $(call rt_CFLAGS_generate,@LAM_CPPFLAGS@)
mpiotf_rt_lam_la_LDFLAGS = -module $(call rt_LDFLAGS_generate,@LAM_LDFLAGS@)
mpiotf_rt_lam_la_LIBADD = $(call rt_lam_LIBADD_generate,@LAM_LIBS@)
mpiotf_rt_lam_la_DEPENDENCIES = libvt-lam-mpi-a
mpiotf_rt_lam_la_SOURCES = $(rt_sources)
endif

if HAVE_LAMPI
mpiotf_rt_lampi_la_CFLAGS = $(call rt_CFLAGS_generate,@LAMPI_CPPFLAGS@)
mpiotf_rt_lampi_la_LDFLAGS = -module $(call rt_LDFLAGS_generate,@LAMPI_LDFLAGS@)
mpiotf_rt_lampi_la_LIBADD = $(call rt_lampi_LIBADD_generate,@LAMPI_LIBS@)
mpiotf_rt_lampi_la_DEPENDENCIES = libvt-lampi-mpi-a
mpiotf_rt_lampi_la_SOURCES = $(rt_sources)
endif

if HAVE_MPICH
mpiotf_rt_mpich_la_CFLAGS = $(call rt_CFLAGS_generate,@MPICH_CPPFLAGS@)
mpiotf_rt_mpich_la_LDFLAGS = -module $(call rt_LDFLAGS_generate,@MPICH_LDFLAGS@)
mpiotf_rt_mpich_la_LIBADD = $(call rt_mpich_LIBADD_generate,@MPICH_LIBS@)
mpiotf_rt_mpich_la_DEPENDENCIES = libvt-mpich-mpi-a
mpiotf_rt_mpich_la_SOURCES = $(rt_sources)
endif

if HAVE_MPICH2
mpiotf_rt_mpich2_la_CFLAGS = $(call rt_CFLAGS_generate,@MPICH2_CPPFLAGS@)
mpiotf_rt_mpich2_la_LDFLAGS = -module $(call rt_LDFLAGS_generate,@MPICH2_LDFLAGS@)
mpiotf_rt_mpich2_la_LIBADD = $(call rt_mpich2_LIBADD_generate,@MPICH2_LIBS@)
mpiotf_rt_mpich2_la_DEPENDENCIES = libvt-mpich2-mpi-a
mpiotf_rt_mpich2_la_SOURCES = $(rt_sources)
endif

if HAVE_MPT
mpiotf_rt_mpt_la_CFLAGS = $(call rt_CFLAGS_generate,@MPT_CPPFLAGS@)
mpiotf_rt_mpt_la_LDFLAGS = -module $(call rt_LDFLAGS_generate,@MPT_LDFLAGS@)
mpiotf_rt_mpt_la_LIBADD = $(call rt_mpt_LIBADD_generate,@MPT_LIBS@)
mpiotf_rt_mpt_la_DEPENDENCIES = libvt-mpt-mpi-a
mpiotf_rt_mpt_la_SOURCES = $(rt_sources)
endif

if HAVE_OPENMPI
mpiotf_rt_openmpi_la_CFLAGS = $(call rt_CFLAGS_generate,@OPENMPI_CPPFLAGS@)
mpiotf_rt_openmpi_la_LDFLAGS = -module $(call rt_LDFLAGS_generate,@OPENMPI_LDFLAGS@)
mpiotf_rt_openmpi_la_LIBADD = $(call rt_openmpi_LIBADD_generate,@OPENMPI_LIBS@)
mpiotf_rt_openmpi_la_DEPENDENCIES = libvt-openmpi-mpi-a
mpiotf_rt_openmpi_la_SOURCES = $(rt_sources)
endif


SUFFIXES = .x 
CLEANFILES = $(BUILT_SOURCES) 
if HAVE_LAM
CLEANFILES += libvt.mpi.lam.a
endif
if HAVE_LAMPI
CLEANFILES += libvt.mpi.lampi.a
endif
if HAVE_MPICH
CLEANFILES += libvt.mpi.mpich.a
endif
if HAVE_MPICH2
CLEANFILES += libvt.mpi.mpich2.a
endif
if HAVE_MPT
CLEANFILES += libvt.mpi.mpt.a
endif
if HAVE_OPENMPI
CLEANFILES += libvt.mpi.openmpi.a
endif

blobs.h blobs.c: blobs.x 
	rm -f  $(patsubst %.x, %.h, $<) $(patsubst %.x, %.c, $<)
	$(RPCGEN) -h -o $(patsubst %.x, %.h, $<) $<
	$(RPCGEN) -c -o $(patsubst %.x, %.c, $<) $<

EXTRA_DIST = vt_openss.h
