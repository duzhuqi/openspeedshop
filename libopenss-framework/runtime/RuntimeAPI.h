/*******************************************************************************
** Copyright (c) 2005 Silicon Graphics, Inc. All Rights Reserved.
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

#ifndef _OpenSpeedShop_Framework_RuntimeAPI_
#define _OpenSpeedShop_Framework_RuntimeAPI_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "OpenSS_DataHeader.h"

#include <rpc/rpc.h>



bool_t OpenSS_DecodeParameters(const char*, const xdrproc_t, void*);
bool_t OpenSS_Send(const OpenSS_DataHeader*, const xdrproc_t, const void*);



#endif
