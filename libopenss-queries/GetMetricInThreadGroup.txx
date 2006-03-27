////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2006 Silicon Graphics, Inc. All Rights Reserved.
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
 * Definition of the GetMetricInThreadGroup template function.
 *
 */

#ifndef _OpenSpeedShop_Queries_GetMetricInThreadGroup_
#define _OpenSpeedShop_Queries_GetMetricInThreadGroup_

#include "Queries.hxx"
#include "ToolAPI.hxx"



//
// To avoid requiring fully qualified names be used everywhere, something like:
//
//     using namespace OpenSpeedShop;
//     using namespace OpenSpeedShop::Framework;
//
// would normally be placed near the top of this source file. Here, however,
// that particular formulation doesn't work well. This file contains template
// definitions that are usually included directly into other source files. By
// using the above formulation, those source files would be required to have
// the same "using" clauses. And doing so would pretty much negate the whole
// reason for using namespaces in the first place. The solution is to place
// the definitions directly inside the OpenSpeedShop namespace and only
// qualify names with Framework:: where necessary.
//
namespace OpenSpeedShop {



/**
 * Get metric values in a thread group.
 *
 * Evalutes the specified collector's metric, over the specified time interval,
 * for the specified source objects, totaled over the specified thread group.
 * Results are provided in a map of source objects to metric values. An
 * initially-empty map is allocated if one is not provided. Source objects with
 * non-zero metric values will then be added into the (new or existing) map.
 *
 * @pre    The specified collector and all threads in the thread group must be
 *         in the same experiment. An assertion failure occurs if more than one
 *         experiment is implied.
 *
 * @pre    All the specified source objects must be from the same experiment as
 *         the specified collector. An assertion failure occurs if more than one
 *         experiment is implied.
 *
 * @param collector    Collector for which to get a metric.
 * @param metric       Unique identifier of the metric.
 * @param interval     Time interval over which to get the metric values. 
 * @param threads      Thread group for which to get metric values.
 * @param objects      Source objects for which to get metric values.
 * @retval result      Smart pointer to a map of the source objects to their
 *                     metric values. A new map is allocated if a null smart
 *                     pointer is passed.
 */
template <typename TS, typename TM>
void Queries::GetMetricInThreadGroup(
    const Framework::Collector& collector,
    const std::string& metric,
    const Framework::TimeInterval& interval,
    const Framework::ThreadGroup& threads,
    const std::set<TS >& objects,
    Framework::SmartPtr<std::map<TS, TM > >& result)
{
    // Check preconditions
    for(Framework::ThreadGroup::const_iterator
	    i = threads.begin(); i != threads.end(); ++i)
	Assert(collector.inSameDatabase(*i));
    for(typename std::set<TS >::const_iterator
	    i = objects.begin(); i != objects.end(); ++i)
	Assert(collector.inSameDatabase(*i));
    
    // Allocate (if necessary) a new map of source objects to values
    if(result.isNull())
        result = Framework::SmartPtr<std::map<TS, TM > >(
            new std::map<TS, TM >()
            );
    Assert(!result.isNull());

    // Construct extent restricting evaluation to requested time interval
    Framework::Extent restriction(
        interval,
        Framework::AddressRange(Framework::Address::TheLowest(),
                                Framework::Address::TheHighest())
        );
    
    // Lock the appropriate database
    collector.lockDatabase();

    // Get the extent table for the source objects in the thread group
    Framework::ExtentTable<TS > extent_table = 
	threads.getExtentsOf(objects, restriction);
    
    // Iterate over each thread in the thread group
    for(Framework::ThreadGroup::const_iterator
	    i = threads.begin(); i != threads.end(); ++i) {

	// Get the extents for the source objects in this thread
	Framework::ExtentGroup& extents = extent_table.getExtents(*i);
	
	// Evaluate the metric values for the necessary extents
	std::vector<TM > values;
	collector.getMetricValues(metric, *i, extents, values);
	
	// Iterate over each evaluated extent
	for(Framework::ExtentGroup::size_type j = 0; j < extents.size(); ++j) {
	    
	    // Was this subextent's evaluation a non-empty value?
	    if(values[j] != TM()) {

		// Get the source object corresponding to this evaluated extent
		const TS& object = extent_table.getObject(*i, j);
		
		// Add this value into the results
		if(result->find(object) == result->end())
		    result->insert(std::make_pair(object, values[j]));
		else
		    (*result)[object] += values[j];
		
	    }
	    
	}

    }

    // Unlock the appropriate database
    collector.unlockDatabase();
}



}  // namespace OpenSpeedShop



#endif
