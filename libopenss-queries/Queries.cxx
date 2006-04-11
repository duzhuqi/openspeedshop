////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 Silicon Graphics, Inc. All Rights Reserved.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2.1 of the License, or (at your option)
// any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
////////////////////////////////////////////////////////////////////////////////

/** @file
 *
 * Definition of the Queries namespace and related code.
 *
 */

#include "Queries.hxx"
#include "ToolAPI.hxx"

using namespace OpenSpeedShop;



/**
 * Make a thread group from a thread.
 *
 * Constructs a thread group containing the one specified thread.
 *
 * @param thread    Thread from which to construct the thread group.
 * @return          Thread group containing that thread.
 */
Framework::ThreadGroup
Queries::MakeThreadGroup(const Framework::Thread& thread)
{
    Framework::ThreadGroup threads;
    threads.insert(thread);
    return threads;
}



/**
 * Strict weak ordering predicate evaluator for linked objects.
 *
 * Implements a strict weak ordering predicate for linked objects that works
 * properly even when the two linked objects are in different experiment
 * databases.
 *
 * @param lhs    Linked object on left hand side of comparison.
 * @param rhs    Linked object on right hand side of comparison.
 * @return       Boolean "true" if (lhs < rhs), "false" otherwise.
 */
bool Queries::CompareLinkedObjects::operator()(
    const Framework::LinkedObject& lhs,
    const Framework::LinkedObject& rhs) const
{
    return lhs.getPath() < rhs.getPath();
}



/**
 * Strict weak ordering predicate evaluator for functions.
 *
 * Implements a strict weak ordering predicate for functions that works
 * properly even when the two functions are in different experiment databases.
 *
 * @param lhs    Function on left hand side of comparison.
 * @param rhs    Function on right hand side of comparison.
 * @return       Boolean "true" if (lhs < rhs), "false" otherwise.
 */
bool Queries::CompareFunctions::operator()(
    const Framework::Function& lhs,
    const Framework::Function& rhs) const
{
    if(lhs.getMangledName() < rhs.getMangledName())
	return true;
    else if(lhs.getMangledName() > rhs.getMangledName())
	return false;
    return lhs.getLinkedObject().getPath() < rhs.getLinkedObject().getPath();
}



/**
 * Strict weak ordering predicate evaluator for statements.
 *
 * Implements a strict weak ordering predicate for statements that works
 * properly even when the two statements are in different experiment databases.
 *
 * @param lhs    Statement on left hand side of comparison.
 * @param rhs    Statement on right hand side of comparison.
 * @return       Boolean "true" if (lhs < rhs), "false" otherwise.
 */
bool Queries::CompareStatements::operator()(
    const Framework::Statement& lhs,
    const Framework::Statement& rhs) const
{
    if(lhs.getPath() < rhs.getPath())
	return true;
    else if(lhs.getPath() > rhs.getPath())
	return false;
    else if(lhs.getLine() < rhs.getLine())
	return true;
    else if(lhs.getLine() > rhs.getLine())
	return false;
    else if(lhs.getColumn() < rhs.getColumn())
	return true;
    else if(lhs.getColumn() > rhs.getColumn())
	return false;
    return lhs.getLinkedObject().getPath() < rhs.getLinkedObject().getPath();
}
