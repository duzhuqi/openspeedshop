////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2007 William Hachfeld. All Rights Reserved.
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place, Suite 330, Boston, MA  02111-1307  USA
////////////////////////////////////////////////////////////////////////////////

/** @file
 *
 * Definition of frontend/backend communication protocol utility functions.
 *
 */

#include "Address.hxx"
#include "AddressRange.hxx"
#include "Assert.hxx"
#include "Blob.hxx"
#include "Path.hxx"
#include "Time.hxx"
#include "Utility.hxx"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <iomanip>
#include <limits>
#include <sstream>
#include <unistd.h>

using namespace OpenSpeedShop::Framework;



/**
 * Compute the checksum of a file.
 *
 * ...
 *
 * @param path    Full path name of the file for which to calculat the checksum.
 * @return        Checksum of that file.
 */
uint64_t OpenSpeedShop::Framework::computeChecksum(const Path& path)
{
    // TODO: implement!
    return 0;
}



//
// Maximum length of a host name. According to the Linux manual page for the
// gethostname() function, this should be available in a header somewhere. But
// it isn't found on all systems, so define it directly if necessary.
//
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX (256)
#endif

/**
 * Get the local host name.
 *
 * Returns the name of the host on which we are executing. This information is
 * obtained directly from the operating system.
 *
 * @note    This function has intentionally been given an identical name to
 *          the similar method in the framework Experiment class. In the near
 *          future, the experiment database functionality may be split into a
 *          separate library and utilized by the daemon. If this happens, this
 *          function will be removed in favor of the existing framework method.
 *
 * @return    Name of the local host.
 */
std::string OpenSpeedShop::Framework::getLocalHost()
{
    // Obtain the local host name from the operating system
    char buffer[HOST_NAME_MAX];
    Assert(gethostname(buffer, sizeof(buffer)) == 0);
    
    // Return the local host name to the caller
    return buffer;
}



/**
 * Get the canonical name of a host.
 *
 * Returns the canonical name of the specified host. This information is
 * obtained directly from the operating system.
 *
 * @note    This function has intentionally been given an identical name to
 *          the similar method in the framework Experiment class. In the near
 *          future, the experiment database functionality may be split into a
 *          separate library and utilized by the daemon. If this happens, this
 *          function will be removed in favor of the existing framework method.
 *
 * @param host    Name of host for which to get canonical name.
 * @return        Canonical name of that host.
 */
std::string OpenSpeedShop::Framework::getCanonicalName(const std::string& host)
{
    std::string canonical = host;

    // Interested in IPv4 protocol information only (including canonical name)
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_CANONNAME;
    hints.ai_family = AF_INET;
    hints.ai_protocol = PF_INET;
    
    // Request address information for this host
    struct addrinfo* results = NULL;
    getaddrinfo(host.c_str(), NULL, &hints, &results);
    
    // Was the specified name for the loopback device?
    if((results != NULL) && 
       (ntohl(reinterpret_cast<struct sockaddr_in*>
              (results->ai_addr)->sin_addr.s_addr) == INADDR_LOOPBACK)) {

        // Free the address information
        freeaddrinfo(results);

        // Request address information for the local host name
        results = NULL;
        getaddrinfo(getLocalHost().c_str(), NULL, &hints, &results);
        
    }

    // Did we get address information?
    if(results != NULL) {

        // Use the canonical name if one was provided
        if(results->ai_canonname != NULL)
            canonical = results->ai_canonname;

        // Free the address information
        freeaddrinfo(results);

    }

    // Return the canonical name to the caller
    return canonical;
}



/**
 * Conversion from OpenSS_Protocol_AddressRange to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_AddressRange into a std::string.
 *
 * @param range    Address range to be converted.
 * @return         String conversion of that range.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_AddressRange& range
    )
{
    std::stringstream output;
    output << AddressRange(Address(range.begin), Address(range.end));
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_AddressBitmap to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_AddressBitmap into
 * a std::string. Simply returns the string containing the textual
 * representation of the address range and bitmap contents 
 *
 * @param range    Address range to be converted.
 * @return         String conversion of that range.
 */

std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_AddressBitmap& bitmap
    )
{
    std::stringstream output;
    output << toString(bitmap.range) << " ";
    for(int i = 0; i < bitmap.bitmap.bitmap_len; ++i)
	output << (bitmap.bitmap.bitmap_val[i] ? "1" : "0");
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_Blob to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_Blob into a std::string.
 * Simply returns the string containing each data byte, sixteen per line,
 * in both hexadecimal and character representations.
 *
 * @param blob    Blob to be converted.
 * @return        String conversion of that blob.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_Blob& blob
    )
{
    static const unsigned BytesPerLine = 16;
    std::stringstream output;

    // Get the size and contents of the blob
    const unsigned size = blob.data.data_len;
    const char* ptr = (const char*)blob.data.data_val;

    // Iterate over each byte in the blob's contents
    for(unsigned i = 0; i < size; i += BytesPerLine) {

	// Indent each line by four spaces
	output << "    ";

	// Display the hexadecimal representation
	for(unsigned j = 0; j < BytesPerLine; j++) {
	    if((i + j) < size)
		output << std::hex << std::setfill('0') << std::setw(2)
		       << static_cast<unsigned>(
			      static_cast<uint8_t>(ptr[i + j])
			      ) << " ";
	    else
		output << "   ";
	}

	// Indent another two spaces before the character representation
	output << "  ";

	// Display the character representation
	for(unsigned j = 0; j < BytesPerLine; j++) {
	    if((i + j) < size) {
		if(isprint(ptr[i + j]))
		    output << ptr[i + j];
		else
		    output << ".";
	    }
	    else
		output << " ";
	}

	// Complete this line
	output << std::endl;
	
    }

    // Return the string to the caller
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_Collector to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_Collector into a std::string.
 * Simply returns the string containing the textual representation of the
 * experiment and collector's unique identifiers.
 *
 * @param collector    Collector to be converted.
 * @return             String conversion of that collector.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_Collector& collector
    )
{
    std::stringstream output;
    output << collector.experiment << ":" << collector.collector;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_Experiment to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_Experiment into a std::string.
 * Simply returns the string containing the textual representation of the
 * experiment's unique identifier.
 *
 * @param experiment    Experiment to be converted.
 * @return              String conversion of that experiment.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_Experiment& experiment
    )
{
    std::stringstream output;
    output << experiment.experiment;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_ExperimentGroup to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_ExperimentGroup into a std::
 * string. Simply returns the string containing the textual representation of
 * the individual experiments in the experiment group.
 *
 * @param experiments    Experiment group to be converted.
 * @return               String conversion of that experiment group.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_ExperimentGroup& experiments
    )
{
    std::stringstream output;
    output << "    { ";
    for(int i = 0; i < experiments.experiments.experiments_len; ++i) {
	if(i > 0)
	    output << ", ";
	output << toString(experiments.experiments.experiments_val[i]);
    }
    output << " }";
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_FileName to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_FileName into a std::string.
 * ...
 *
 * @param file    File name to be converted.
 * @return        String conversion of that file name.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_FileName& file
    )
{
    // TODO: implement!
}



/**
 * Conversion from OpenSS_Protocol_FunctionEntry to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_FunctionEntry into a
 * std::string. ...
 *
 * @param function    Function entry to be converted.
 * @return            String conversion of that function entry.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_FunctionEntry& function
    )
{
    // TODO: implement!
}



/**
 * Conversion from OpenSS_Protocol_JobEntry to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_JobEntry into a std::string. ...
 * Simply returns the string containing the textual representation of the host
 * name and process identifier.
 *
 * @param entry    Job entry to be converted.
 * @return         String conversion of that job entry.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_JobEntry& entry
    )
{
    std::stringstream output;
    output << entry.host << ":" << entry.pid;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_Job to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_Job into a std::string. Simply
 * returns the string containing the textual representation of the individual
 * entries in the job.
 *
 * @param job    Job to be converted.
 * @return       String conversion of that job.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_Job& job
    )
{
    std::stringstream output;
    output << "    { ";
    for(int i = 0; i < job.entries.entries_len; ++i) {
	if(i > 0)
	    output << ", ";
	output << toString(job.entries.entries_val[i]);
	if(!(i % 4) && (i < (job.entries.entries_len - 1)))
	    output << std::endl << "      ";
    }
    output << " }";
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_StatementEntry to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_StatementEntry into a
 * std::string. ...
 *
 * @param statement    Statement entry to be converted.
 * @return             String conversion of that statement entry.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_StatementEntry& statement
    )
{
    // TODO: implement!
}



/**
 * Conversion from OpenSS_Protocol_ThreadName to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_ThreadName into a std::string.
 * Simply returns the string containing the textual representation of the
 * experiment's unique identifier, host name, the process identifier, and (if
 * present) the posix thread identifier.
 *
 * @param thread    Thread name to be converted.
 * @return          String conversion of that thread name.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_ThreadName& thread
    )
{
    std::stringstream output;
    output << thread.experiment << ":" << thread.host << ":" << thread.pid;
    if(thread.has_posix_tid)
	output << ":" << thread.posix_tid;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_ThreadNameGroup to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_ThreadNameGroup into a std::
 * string. Simply returns the string containing the textual representation of
 * the individual thread names in the thread name group.
 *
 * @param threads    Thread name group to be converted.
 * @return           String conversion of that thread name group.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_ThreadNameGroup& threads
    )
{
    std::stringstream output;
    output << "    { ";
    for(int i = 0; i < threads.names.names_len; ++i) {
	if(i > 0)
	    output << ", ";
	output << toString(threads.names.names_val[i]);
	if(!(i % 4) && (i < (threads.names.names_len - 1)))
	    output << std::endl << "      ";
    }
    output << " }";
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_ThreadState to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_ThreadState into a std::string.
 * Simply returns the string containing the textual representation of the passed
 * thread state enumeration value.
 *
 * @param state    Thread state to be converted.
 * @return         String conversion of that thread state.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_ThreadState& state
    )
{
    switch(state) {
    case Running:
	return "Running";
    case Suspended:
	return "Suspended";
    case Terminated:
	return "Terminated";
    default:
	return "?";
    }
}



/**
 * Conversion from OpenSS_Protocol_AttachToThreads to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_AttachToThreads message
 * into a std::string.
 *
 * @param message    Message to be converted.
 * @return           String conversion of that message.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_AttachToThreads& message
    )
{
    std::stringstream output;
    output << "attachToThreads(" << std::endl 
	   << toString(message.threads) << std::endl
	   << ")" << std::endl;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_AttachedToThreads to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_AttachedToThreads message
 * into a std::string.
 *
 * @param message    Message to be converted.
 * @return           String conversion of that message.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_AttachedToThreads& message
    )
{
    std::stringstream output;
    output << "attachedToThreads(" << std::endl 
	   << toString(message.threads) << std::endl
	   << ")" << std::endl;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_ChangeThreadsState to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_ChangeThreadsState message
 * into a std::string.
 *
 * @param message    Message to be converted.
 * @return           String conversion of that message.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_ChangeThreadsState& message
    )
{
    std::stringstream output;
    output << "changeThreadsState(" << std::endl
	   << toString(message.threads) << "," << std::endl
	   << "    " << toString(message.state) << std::endl
	   << ")" << std::endl;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_CreateProcess to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_CreateProcess message into a
 * std::string.
 *
 * @param message    Message to be converted.
 * @return           String conversion of that message.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_CreateProcess& message
    )
{
    std::stringstream output;
    output << "createProcess(" << std::endl
	   << "    " << toString(message.thread) << "," << std::endl
	   << "    \"" << message.command << "\"," << std::endl
	   << toString(message.environment) << std::endl
	   << ")" << std::endl;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_DetachFromThreads to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_DetachFromThreads message
 * into a std::string.
 *
 * @param message    Message to be converted.
 * @return           String conversion of that message.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_DetachFromThreads& message
    )
{
    std::stringstream output;
    output << "detachFromThreads(" << std::endl 
	   << toString(message.threads) << std::endl 
	   << ")" << std::endl;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_ExecuteNow to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_ExecuteNow message into a
 * std::string.
 *
 * @param message    Message to be converted.
 * @return           String conversion of that message.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_ExecuteNow& message
    )
{
    std::stringstream output;
    output << "executeNow(" << std::endl
	   << toString(message.threads) << "," << std::endl
	   << "    " << toString(message.collector) << "," << std::endl
	   << "    " << (message.disable_save_fpr ? "true" : "false") << "," 
	   << std::endl
	   << "    \"" << message.callee << "\"," << std::endl
	   << toString(message.argument) << std::endl
	   << ")" << std::endl;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_ExecuteAtEntryOrExit to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_ExecuteAtEntryOrExit message
 * into a std::string.
 *
 * @param message    Message to be converted.
 * @return           String conversion of that message.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_ExecuteAtEntryOrExit& message
    )
{
    std::stringstream output;
    output << "executeAtEntryOrExit(" << std::endl
	   << toString(message.threads) << "," << std::endl
	   << "    " << toString(message.collector) << "," << std::endl
	   << "    \"" << message.where << "\"," << std::endl
	   << "    " << (message.at_entry ? "true" : "false") << "," 
	   << std::endl
	   << "    \"" << message.callee << "\"," << std::endl
	   << toString(message.argument) << std::endl
	   << ")" << std::endl;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_ExecuteInPlaceOf to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_ExecuteInPlaceOf message
 * into a std::string.
 *
 * @param message    Message to be converted.
 * @return           String conversion of that message.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_ExecuteInPlaceOf& message
    )
{
    std::stringstream output;
    output << "executeInPlaceOf(" << std::endl
	   << toString(message.threads) << "," << std::endl
	   << "    " << toString(message.collector) << "," << std::endl
	   << "    \"" << message.where << "\"," << std::endl
	   << "    \"" << message.callee << "\"" << std::endl
	   << ")" << std::endl;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_GetGlobalInteger to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_GetGlobalInteger message
 * into a std::string.
 *
 * @param message    Message to be converted.
 * @return           String conversion of that message.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_GetGlobalInteger& message
    )
{
    std::stringstream output;
    output << "getGlobalInteger("
	   << toString(message.thread) << ", "
	   << "\"" << message.global << "\""
	   << ")" << std::endl;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_GetGlobalString to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_GetGlobalString message
 * into a std::string.
 *
 * @param message    Message to be converted.
 * @return           String conversion of that message.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_GetGlobalString& message
    )
{
    std::stringstream output;
    output << "getGlobalString("
	   << toString(message.thread) << ", "
	   << "\"" << message.global << "\""
	   << ")" << std::endl;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_GetMPICHProcTable to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_GetMPICHProcTable message
 * into a std::string.
 *
 * @param message    Message to be converted.
 * @return           String conversion of that message.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_GetMPICHProcTable& message
    )
{
    std::stringstream output;
    output << "getMPICHProcTable("
	   << toString(message.thread)
	   << ")" << std::endl;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_GlobalIntegerValue to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_GlobalIntegerValue message
 * into a std::string.
 *
 * @param message    Message to be converted.
 * @return           String conversion of that message.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_GlobalIntegerValue& message
    )
{
    std::stringstream output;
    output << "globalIntegerValue(" << std::endl
	   << "    " << toString(message.thread) << "," << std::endl
	   << "    \"" << message.global << "\"," << std::endl 
	   << "    " << message.value << std::endl
	   << ")" << std::endl;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_GlobalJobValue to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_GlobalJobValue message into a
 * std::string.
 *
 * @param message    Message to be converted.
 * @return           String conversion of that message.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_GlobalJobValue& message
    )
{
    std::stringstream output;
    output << "globalJobValue(" << std::endl
	   << "    " << toString(message.thread) << "," << std::endl
	   << "    \"" << message.global << "\"," << std::endl
	   << toString(message.value) << std::endl
	   << ")" << std::endl;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_GlobalStringValue to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_GlobalStringValue message
 * into a std::string.
 *
 * @param message    Message to be converted.
 * @return           String conversion of that message.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_GlobalStringValue& message
    )
{
    std::stringstream output;
    output << "globalStringValue(" << std::endl
	   << "    " << toString(message.thread) << "," << std::endl
	   << "    \"" << message.global << "\"," << std::endl 
	   << "    \"" << message.value << "\"" << std::endl
	   << ")" << std::endl;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_LoadedLinkedObject to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_LoadedLinkedObject message
 * into a std::string.
 *
 * @param message    Message to be converted.
 * @return           String conversion of that message.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_LoadedLinkedObject& message
    )
{
    std::stringstream output;
    output << "loadedLinkedObject(" << std::endl
	   << toString(message.threads) << "," << std::endl
	   << "    " << Time(message.time) << "," << std::endl
	   << "    " << toString(message.range) << "," << std::endl
	   << "    " << toString(message.linked_object) << "," << std::endl
	   << "    " << (message.is_executable ? "true" : "false") << std::endl
	   << ")" << std::endl;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_ReportError to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_ReportError message into a
 * std::string.
 *
 * @param message    Message to be converted.
 * @return           String conversion of that message.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_ReportError& message
    )
{
    std::stringstream output;
    output << "reportError(" << std::endl
	   << "    \"" << message.text << "\"" << std::endl
	   << ")" << std::endl;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_SetGlobalInteger to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_SetGlobalInteger message
 * into a std::string.
 *
 * @param message    Message to be converted.
 * @return           String conversion of that message.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_SetGlobalInteger& message
    )
{
    std::stringstream output;
    output << "setGlobalString("
	   << toString(message.thread) << ", "
	   << "\"" << message.global << "\", "
	   << message.value 
	   << ")" << std::endl;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_StopAtEntryOrExit to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_StopAtEntryOrExit message
 * into a std::string.
 *
 * @param message    Message to be converted.
 * @return           String conversion of that message.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_StopAtEntryOrExit& message
    )
{
    std::stringstream output;
    output << "stopAtEntryOrExit(" << std::endl
	   << toString(message.threads) << "," << std::endl
	   << "    \"" << message.where << "\"," << std::endl
	   << (message.at_entry ? "true" : "false") << std::endl
	   << ")" << std::endl;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_SymbolTable to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_SymbolTable message into a
 * std::string.
 *
 * @param message    Message to be converted.
 * @return           String conversion of that message.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_SymbolTable& message
    )
{
    std::stringstream output;
    output << "symbolTable(" << std::endl
	   << toString(message.experiments) << "," << std::endl
	   << toString(message.linked_object) << "," << std::endl;
    
    for(int i = 0; i < message.functions.functions_len; ++i) {
	// TODO: implement!
    }
    
    for(int i = 0; i < message.statements.statements_len; ++i) {
	// TODO: implement!
    }
    
    output << ")" << std::endl;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_ThreadsStateChanged to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_ThreadsStateChanged message
 * into a std::string.
 *
 * @param message    Message to be converted.
 * @return           String conversion of that message.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_ThreadsStateChanged& message
    )
{
    std::stringstream output;
    output << "threadsStateChanged(" << std::endl
	   << toString(message.threads) << "," << std::endl
	   << "    " << toString(message.state) << std::endl
	   << ")" << std::endl;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_Uninstrument to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_Uninstrument message into a
 * std::string.
 *
 * @param message    Message to be converted.
 * @return           String conversion of that message.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_Uninstrument& message
    )
{
    std::stringstream output;
    output << "uninstrument(" << std::endl
	   << toString(message.threads) << "," << std::endl
	   << "    " << toString(message.collector) << std::endl
	   << ")" << std::endl;
    return output.str();
}



/**
 * Conversion from OpenSS_Protocol_UnloadedLinkedObject to std::string.
 *
 * Returns the conversion of an OpenSS_Protocol_UnloadedLinkedObject message
 * into a std::string.
 *
 * @param message    Message to be converted.
 * @return           String conversion of that message.
 */
std::string OpenSpeedShop::Framework::toString(
    const OpenSS_Protocol_UnloadedLinkedObject& message
    )
{
    std::stringstream output;
    output << "unloadedLinkedObject(" << std::endl
	   << toString(message.threads) << "," << std::endl
	   << "    " << Time(message.time) << "," << std::endl
	   << "    " << toString(message.linked_object) << "," << std::endl
	   << ")" << std::endl;
    return output.str();
}