/*******************************************************************************
** Copyright (c) 2005 Silicon Graphics, Inc. All Rights Reserved.
** Copyright (c) 2007 William Hachfeld. All Rights Reserved.
**
** This library is free software; you can redistribute it and/or modify it under
** the terms of the GNU Lesser General Public License as published by the Free
** Software Foundation; either version 2.1 of the License, or (at your option)
** any later version.
**
** This library is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
** details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this library; if not, write to the Free Software Foundation, Inc.,
** 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*******************************************************************************/

/** @file
 *
 * Declaration of the Runtime API.
 *
 */

#ifndef _OpenSpeedShop_Runtime_RuntimeAPI_
#define _OpenSpeedShop_Runtime_RuntimeAPI_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Assert.h"
#include "OpenSS_DataHeader.h"

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <rpc/rpc.h>
#include <ucontext.h>



/** dpcl limits blobs to about 16k. All collectors are tuned to */
/** create blobs that attempt to maximize usage of the 16k blob sisze */
/** fileio does not have a 16k restriction on the size of a blob */
/** TODO: Since OpenSS_Send use alloca for allocation of blobs we */
/** are still limited by what safely can be allocated on the stack. */
/** For OPENSS_USE_FILEIO we could consider using malloc/free */
#if defined (OPENSS_OFFLINE)
#define OpenSS_BlobSizeFactor 15
#else
#define OpenSS_BlobSizeFactor 1
#endif

/** Number of entries in the sample buffer. */
#define OpenSS_PCBufferSize (1024 * OpenSS_BlobSizeFactor)

/** Number of entries in the hash table. */
#define OpenSS_PCHashTableSize (OpenSS_PCBufferSize + (OpenSS_PCBufferSize / 4))

/** Type representing PC sampling data (PCs and their respective counts). */
typedef struct {

    uint64_t addr_begin;  /**< Beginning of gathered data's address range. */
    uint64_t addr_end;    /**< End of gathered data's address range. */
    
    uint16_t length;  /**< Actual used length of the PC and count arrays. */
    
    uint64_t pc[OpenSS_PCBufferSize];    /**< Program counter (PC) addresses. */
    uint8_t count[OpenSS_PCBufferSize];  /**< Sample count at each address. */
    
    /** Hash table mapping PC addresses to their array index. */
    unsigned hash_table[OpenSS_PCHashTableSize];
    
} OpenSS_PCData;



/**
 * Type representing different floating-point exception (FPE) types.
 *
 * @note    Some systems fail to properly fill in the "si_code" when calling
 *          the FPE handler. Thus we must have an Unknown FPE to handle these
 *          cases.
 */
typedef enum {
    DivideByZero,        /**< Divide by zero. */
    Overflow,            /**< Overflow. */
    Underflow,           /**< Underflow. */
    InexactResult,       /**< Inexact result. */
    InvalidOperation,    /**< Invalid operation. */
    SubscriptOutOfRange, /**< Subscript out of range. */
    Unknown,             /**< Unknown FPE (no "si_code" provided). */
    AllFPE               /**< Toggle on all FPE's. */
} OpenSS_FPEType; 

/** Type representing a function pointer to a fpe event handler. */
typedef void (*OpenSS_FPEEventHandler)(const OpenSS_FPEType, const ucontext_t*);



/** Type representing a function pointer to a timer event handler. */
typedef void (*OpenSS_TimerEventHandler)(const ucontext_t*);



void OpenSS_DecodeParameters(const char*, const xdrproc_t, void*);
void OpenSS_FPEHandler(const OpenSS_FPEType, const OpenSS_FPEEventHandler);
uint64_t OpenSS_GetAddressOfFunction(const void*);
uint64_t OpenSS_GetPCFromContext(const ucontext_t*);
void OpenSS_InitializeDataHeader(int, int, OpenSS_DataHeader*);
void OpenSS_SetPCInContext(uint64_t, ucontext_t*);
uint64_t OpenSS_GetTime();
void OpenSS_Send(const OpenSS_DataHeader*, const xdrproc_t, const void*);
void OpenSS_Timer(uint64_t, const OpenSS_TimerEventHandler);
bool_t OpenSS_UpdatePCData(uint64_t, OpenSS_PCData*);
bool_t OpenSS_Path_From_Pid(char *);

#ifdef HAVE_LIBUNWIND
void OpenSS_GetStackTraceFromContext(const ucontext_t*,
				     bool_t, unsigned, unsigned,
				     unsigned*, uint64_t*);
#endif

#ifdef HAVE_PAPI
void OpenSS_Create_Eventset(int*);
void OpenSS_AddEvent(int, int);
void OpenSS_Overflow(int, int, int, void*);
void OpenSS_Start(int);
void OpenSS_Stop(int);
#endif

#ifdef HAVE_BINUTILS
int OpenSS_GetInstrLength(uint64_t);
#endif



/**
 * Get the current context.
 *
 * Returns the current context of the calling thread.
 *
 * @retval context    Current context.
 *
 * @ingroup RuntimeAPI
 */
#define OpenSS_GetContext(context)    \
    Assert(getcontext(context) == 0);



#endif
