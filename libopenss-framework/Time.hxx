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
 * Declaration and definition of the Time class.
 *
 */

#ifndef _OpenSpeedShop_Framework_Time_
#define _OpenSpeedShop_Framework_Time_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Assert.hxx"
#include "TotallyOrdered.hxx"

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#include <limits>
#include <stdexcept>
#include <time.h>



namespace OpenSpeedShop { namespace Framework {

    /**
     * Time.
     *
     * All time values are represented using a single 64-bit unsigned integer.
     * These integers are interpreted as the number of nanoseconds that have
     * passed since midnight (00:00) Coordinated Universal Time (UTC), on
     * Janaury 1, 1970. This system gives nanosecond resolution for representing
     * times while not running out the clock until sometime in the year 2054.
     *
     * @ingroup Utility
     */
    class Time :
	public TotallyOrdered<Time>
    {

    public:

	/** Create the earliest possible time value. */
	static Time TheBeginning()
	{
	    return Time(std::numeric_limits<value_type>::min());
	}

	/** Create the current time value. */
	static Time Now()
	{
	    struct timespec now;
	    Assert(clock_gettime(CLOCK_REALTIME, &now) == 0);
	    return Time((static_cast<value_type>(now.tv_sec) *
			 static_cast<value_type>(1000000000)) +
			static_cast<value_type>(now.tv_nsec));
	}

	/** Create the last possible time value. */
	static Time TheEnd()
	{
	    return Time(std::numeric_limits<value_type>::max());
	}
	
    public:

	/** Type representing a time. */
	typedef uint64_t value_type;
	
	/** Type representing the difference between two times. */
	typedef int64_t difference_type;
	
	/** Default constructor. */
	Time() :
	    dm_value(0x0)
	{
	}

	/** Constructor from a time value. */
	Time(const value_type& value) :
	    dm_value(value)
	{
	}

	/** Operator "<" defined for two Time objects. */
	bool operator<(const Time& other) const
	{
	    return dm_value < other.dm_value;
	}

	/** Operator "+=" defined for an Time object and a signed offset. */
	Time& operator+=(const difference_type& other)
	{
	    value_type result = dm_value + other;
	    if((other < 0) && (result >= dm_value))
		throw std::range_error("Adding a negative offset to a time "
				       "resulted in a time less than "
				       "UINT64_MIN.");
	    if((other > 0) && (result <= dm_value))
		throw std::range_error("Adding a positive offset to a time "
				       "resulted in a time greater than "
				       "UINT64_MAX.");
	    dm_value = result;
	    return *this;
	}
	
	/** Operator "+" defined in terms of "+=". */
	Time operator+(const difference_type& other) const
	{
	    return Time(*this) += other;
	}

	/** Operator "-" defined for two Time objects. */
	difference_type operator-(const Time& other) const
	{
	    difference_type result = dm_value - other.dm_value;
	    if((*this < other) && (result >= 0))
		throw std::range_error("Subtracting two times resulted in "
				       "a time less than INT64_MIN.");
	    if((*this > other) && (result <= 0))
		throw std::range_error("Subtracting two times resulted in "
				       "a time greater than INT64_MAX.");
	    return result;
	}

	/** Operator "<<" defined for std::ostream. */
	friend std::ostream& operator<<(std::ostream& stream,
					const Time& object)
	{
	    time_t value = object.dm_value / 1000000000;
	    char buffer[32];
	    Assert(ctime_r(&value, buffer) != NULL);
	    stream << std::string(buffer, strlen(buffer) - 1);
	    return stream;
	}
	
	/** Read-only data member accessor function. */
	const value_type& getValue() const
	{
	    return dm_value;
	}

    private:

	/** Time value. */
	value_type dm_value;
	
    };

} }



#endif
