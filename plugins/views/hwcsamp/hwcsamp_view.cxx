/*******************************************************************************
** Copyright (c) 2010 Krell Institute. All Rights Reserved.
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



#include "SS_Input_Manager.hxx"
#include "../../collectors/hwcsamp/HWCSampCollector.hxx"
#include "../../collectors/hwcsamp/HWCSampDetail.hxx"
#include "../../collectors/hwcsamp/HWCSampEvents.h"

// There are 2 reserved locations in the predefined-temporay table.
// Additional items may be defined for individual collectors.

// These are needed to manage hwcsamp collector data.
#define excnt_temp   VMulti_free_temp
#define extime_temp  VMulti_free_temp+1
#define event_temps  VMulti_free_temp+2

#define First_ByThread_Temp VMulti_free_temp+OpenSS_NUMCOUNTERS+3
#define ByThread_use_intervals 2 // "1" => times reported in milliseconds,
                                 // "2" => times reported in seconds,
                                 // otherwise don't add anything.
#include "SS_View_bythread_locations.hxx"
#include "SS_View_bythread_setmetrics.hxx"


#define def_HWCSAMP_values                             \
            uint64_t excnt = 0;                        \
            double extime = 0.0;                       \
            uint64_t exevent[OpenSS_NUMCOUNTERS];      \
            { for(int i=0; i<OpenSS_NUMCOUNTERS;i++) { \
                exevent[i] = 0; } }

#define get_Hwcsamp_invalues(primary, num_calls)

#define get_Hwcsamp_exvalues(secondary, num_calls)       \
              excnt++;                                   \
              extime += secondary.dm_time / num_calls;   \
              { for(int i=0; i<OpenSS_NUMCOUNTERS;i++) { \
                  exevent[i] += secondary.dm_event_values[i]; } }

#define get_inclusive_values(stdv, num_calls)           \
{           int64_t len = stdv.size();                  \
            for (int64_t i = 0; i < len; i++) {         \
             /* Use macro to combine all the values. */ \
              get_Hwcsamp_invalues(stdv[i],num_calls)   \
            }                                           \
}

#define get_exclusive_values(stdv, num_calls)           \
{           int64_t len = stdv.size();                  \
            for (int64_t i = 0; i < len; i++) {         \
             /* Use macro to combine all the values. */ \
              get_Hwcsamp_exvalues(stdv[i],num_calls)   \
            }                                           \
}

#define set_HWCSAMP_values(value_array, sort_excnt)                                          \
              if (num_temps > VMulti_sort_temp) value_array[VMulti_sort_temp] = NULL;        \
              if (num_temps > VMulti_time_temp)  {                                           \
                value_array[VMulti_time_temp] = CRPTR (extime);                              \
              }                                                                              \
              if (num_temps > excnt_temp) value_array[excnt_temp] = CRPTR (excnt);           \
              if (num_temps > extime_temp) value_array[extime_temp] = CRPTR (extime);        \
              if (num_temps > event_temps) {                                                 \
                for(int i=0; i<OpenSS_NUMCOUNTERS;i++) {                                     \
                   if (num_temps <= (event_temps+i)) break;                                                \
                   value_array[event_temps+i] = CRPTR(exevent[i]); } }


#define def_Detail_values def_HWCSAMP_values
#define get_inclusive_trace get_inclusive_values
#define get_exclusive_trace get_exclusive_values
#define set_Detail_values set_HWCSAMP_values
#define Determine_Objects Get_Filtered_Objects
#include "SS_View_detail.txx"


// Hardware Counter Sampling HWCSamp Report
//
static std::string allowed_hwcsamp_V_options[] = {
  "LinkedObject",
  "LinkedObjects",
  "Dso",
  "Dsos",
  "Function",
  "Functions",
  "Statement",
  "Statements",
  "Summary",
  "data",        // Raw data output for scripting
  ""
};



static bool define_hwcsamp_columns (
            CommandObject *cmd,
            ExperimentObject *exp,
            std::vector<Collector>& CV,
            std::vector<std::string>& MV,
            std::vector<ViewInstruction *>& IV,
            std::vector<std::string>& HV,
            View_Form_Category vfc) {
  OpenSpeedShop::cli::ParseResult *p_result = cmd->P_Result();
  vector<ParseRange> *p_slist = p_result->getexpMetricList();
  int64_t last_column = 0;
  int64_t totalIndex  = 0;  // Number of totals needed to perform % calculations.

  bool Generate_Summary = Look_For_KeyWord(cmd, "Summary");
  bool Generate_ButterFly = false;  // Not supported for this view.
  int64_t View_ByThread_Identifier = Determine_ByThread_Id (exp);
  std::string ByThread_Header = Find_Metadata ( CV[0], "time" ).getShortName();

 // Determine the available events for the detail metric.
  int64_t num_events = 0;
  std::string papi_names[OpenSS_NUMCOUNTERS];
  Collector c = CV[0];
  std::string id = "event";
  {
    std::string Value;
    CV[0].getParameterValue("event", Value);

   // Parse the event string.
    int len = Value.length();
    int end = Value.find(",")+1;
    int start = end;
    while (end < len) {
      while ((end < len) && (Value[end] != *(","))) end++;
      if (end < len) {
        std::string N = Value.substr(start,end-start);
        papi_names[num_events++] = N;
        start = end + 1;
        end = start;
      }
    }
  }
  
 // Define combination instructions for predefined temporaries.
  IV.push_back(new ViewInstruction (VIEWINST_Add, VMulti_sort_temp));
  IV.push_back(new ViewInstruction (VIEWINST_Add, VMulti_time_temp));
  IV.push_back(new ViewInstruction (VIEWINST_Add, extime_temp));
  IV.push_back(new ViewInstruction (VIEWINST_Add, excnt_temp));
  for (int i=0; i < OpenSS_NUMCOUNTERS; i++) {
    IV.push_back(new ViewInstruction (VIEWINST_Add, event_temps+i));
  }

  if (Generate_Summary) {
   // Total time is always displayed - also add display of the summary time.
    IV.push_back(new ViewInstruction (VIEWINST_Display_Summary));
  }

  if (p_slist->begin() != p_slist->end()) {
   // Add modifiers to output list.
    vector<ParseRange>::iterator mi;
    for (mi = p_slist->begin(); mi != p_slist->end(); mi++) {
      parse_range_t *m_range = (*mi).getRange();
      std::string C_Name;
      std::string M_Name;
      if (m_range->is_range) {
        C_Name = m_range->start_range.name;
        if (strcasecmp(C_Name.c_str(), "hwcsamp")) {
         // We only know what to do with the hwcsamp collector.
          std::string s("The specified collector, " + C_Name +
                        ", can not be displayed as part of a 'hwcsamp' view.");
          Mark_Cmd_With_Soft_Error(cmd,s);
          return false;
        }
        M_Name = m_range->end_range.name;
      } else {
        M_Name = m_range->start_range.name;
      }

      if (!strcasecmp(M_Name.c_str(), "time") ||
          !strcasecmp(M_Name.c_str(), "times")) {
        IV.push_back(new ViewInstruction (VIEWINST_Display_Tmp, last_column++, extime_temp));
        HV.push_back(Find_Metadata ( CV[0], "time" ).getDescription());
      } else if (!strcasecmp(M_Name.c_str(), "percent") ||
                 !strcmp(M_Name.c_str(), "%")           ||
                 !strcasecmp(M_Name.c_str(), "%time")   ||
                 !strcasecmp(M_Name.c_str(), "%times")) {
       // percent is calculate from 2 temps: time for this row and total time.
       // Sum the extime_temp values.
        IV.push_back(new ViewInstruction (VIEWINST_Define_Total_Tmp, totalIndex, extime_temp));
        IV.push_back(new ViewInstruction (VIEWINST_Display_Percent_Tmp, last_column++, extime_temp, totalIndex++));
        HV.push_back("% of Total Exclusive Time");
      }
#include "SS_View_bythread_recognize.hxx"

      else {
       // Look for Event options.
        bool event_found = false;
        for (int i=0; i < num_events; i++) {
          const char *c = papi_names[i].c_str();
          if (c == NULL) break;
          if (!strcasecmp(M_Name.c_str(), c) ||
              !strcasecmp(M_Name.c_str(), "AllEvents")) {
            event_found = true;
            IV.push_back(new ViewInstruction (VIEWINST_Display_Tmp, last_column++, event_temps+i));
            HV.push_back( papi_names[i] );
          }
        }

        if (!event_found) {
         // Unrecognized '-m' option.
          Mark_Cmd_With_Soft_Error(cmd,"Warning: Unsupported option, '-m " + M_Name + "'");
        }
      }
    }

  } else {
   // If nothing is requested ...
   // Report the exclusive time and the percent.
    IV.push_back(new ViewInstruction (VIEWINST_Display_Tmp, last_column++, extime_temp));  // first column is metric
    HV.push_back( Find_Metadata ( CV[0], "time" ).getDescription() );
   // Percent is calculated from 2 temps: time for this row and total inclusive time.
    if (Filter_Uses_F(cmd)) {
     // Use the metric needed for calculating total time.
      IV.push_back(new ViewInstruction (VIEWINST_Define_Total_Metric, totalIndex, 1));
    } else {
     // Sum the extime_temp values.
      IV.push_back(new ViewInstruction (VIEWINST_Define_Total_Tmp, totalIndex, extime_temp));
    }
    IV.push_back(new ViewInstruction (VIEWINST_Display_Percent_Tmp, last_column++, extime_temp, totalIndex++));
    HV.push_back( std::string("% of ") + Find_Metadata ( CV[0], "time" ).getShortName() );
  }
  return (last_column > 0);
}

static bool hwcsamp_definition ( CommandObject *cmd, ExperimentObject *exp, int64_t topn,
                                 ThreadGroup& tgrp, std::vector<Collector>& CV, std::vector<std::string>& MV,
                                 std::vector<ViewInstruction *>& IV, std::vector<std::string>& HV,
                                 View_Form_Category vfc) {
    Assert (CV.begin() != CV.end());
    CollectorGroup cgrp = exp->FW()->getCollectors();
    Collector C = *CV.begin();
    if (cgrp.find(C) == std::set<Collector>::iterator(cgrp.end())) {
      std::string C_Name = C.getMetadata().getUniqueId();
      std::string s("The required collector, " + C_Name + ", was not used in the experiment.");
      Mark_Cmd_With_Soft_Error(cmd,s);
      return false;
    }
    std::string M_Name = MV[0];
    if (!Collector_Generates_Metric (*CV.begin(), M_Name)) {
      std::string s("The metrics required to generate the view are not available in the experiment.");
      Mark_Cmd_With_Soft_Error(cmd,s);
      return false;
    }

    Validate_V_Options (cmd, allowed_hwcsamp_V_options);
    return define_hwcsamp_columns (cmd, exp, CV, MV, IV, HV, vfc);
}


//TODO: Add more info regarding hwcsamp.
static std::string VIEW_hwcsamp_brief = "HWC (Hardware Counter) report";
static std::string VIEW_hwcsamp_short = "Report hardware counts and the amount and percent of program time spent in a code unit.";
static std::string VIEW_hwcsamp_long  =
                   "The report is sorted in descending order by the amount of time that"
                   " was used in each unit. Also included in the report is the"
                   " percent of total time that each unit uses."
                   " A positive integer can be added to the end of the keyword"
                   " 'hwcsamp' to indicate the maximum number of items in the report."
                   "\n\nThe type of unit displayed can be selected with the '-v'"
                   " option."
                   "\n\t'-v LinkedObjects' will report times by linked object."
                   "\n\t'-v Functions' will report times by function. This is the default."
                   "\n\t'-v Statements' will report times by statement."
                  "\n\nThe information included in the report can be controlled with the"
                  " '-m' option.  More than one item can be selected but only the items"
                  " listed after the option will be printed and they will be printed in"
                  " the order that they are listed."
                  " If no '-m' option is specified, the default is equivalent to"
                  " '-m time, percent'."
                  " Each value pertains to the function, statement or linked object that is"
                  " on that row of the report.  The 'Thread...' selections pertain to the"
                  " process unit that the program was partitioned into: Pid's,"
                  " Posix threads, Mpi threads or Ranks."
                  " \n\t'-m time' reports the total cpu time for all the processes."
                  " \n\t'-m percent' reports the percent of total cpu time for all the processes."
                  " \n\t'-m <event_name>' reports counts associated with the named event for all"
                  " the processes. (Use 'expStatus' to see active event counters.>"
                  " \n\t'-m allEvents' reports all the counter values for all the processes."
// Get the description of the BY-Thread metrics.
#include "SS_View_bythread_help.hxx"
                  "\n";
static std::string VIEW_hwcsamp_example = "\texpView hwcsamp\n"
                                         "\texpView -v statements hwcsamp10\n";
static std::string VIEW_hwcsamp_metrics[] =
  { "time",
    "exclusive_detail",
    ""
  };
static std::string VIEW_hwcsamp_collectors[] =
  { "hwcsamp",
    ""
  };
class hwcsamp_view : public ViewType {

 public: 
  hwcsamp_view() : ViewType ("hwcsamp",
                            VIEW_hwcsamp_brief,
                            VIEW_hwcsamp_short,
                            VIEW_hwcsamp_long,
                            VIEW_hwcsamp_example,
                           &VIEW_hwcsamp_metrics[0],
                           &VIEW_hwcsamp_collectors[0],
                            true) {
  }
  virtual bool GenerateView (CommandObject *cmd, ExperimentObject *exp, int64_t topn,
                             ThreadGroup& tgrp, std::list<CommandResult *>& view_output) {
    std::vector<Collector> CV;
    std::vector<std::string> MV;
    std::vector<ViewInstruction *>IV;
    std::vector<std::string> HV;

   // Initialize the detail metric
    CV.push_back( Get_Collector (exp->FW(), VIEW_hwcsamp_collectors[0]) );
    MV.push_back(VIEW_hwcsamp_metrics[1]);
   // Loadbalance utilities look for second metric, so set it to be just the time metric.
    CV.push_back( Get_Collector (exp->FW(), VIEW_hwcsamp_collectors[0]) );
    MV.push_back(VIEW_hwcsamp_metrics[0]);

    View_Form_Category vfc = Determine_Form_Category(cmd);
    if (hwcsamp_definition (cmd, exp, topn, tgrp, CV, MV, IV, HV, vfc)) {

      if ((CV.size() == 0) ||
          (MV.size() == 0)) {
        Mark_Cmd_With_Soft_Error(cmd, "(There are no metrics specified to report.)");
        return false;   // There is no collector, return.
      }

//      CommandResult *dummyVector;
      std::vector<HWCSampDetail> dummyVector;
// TEST      HWCSampDetail *dummyVector;
      switch (vfc) {
       case VFC_Function:
        Framework::Function *fp;
        return Detail_Base_Report (cmd, exp, topn, tgrp, CV, MV, IV, HV,
                                   false, fp, vfc, &dummyVector, view_output);
       case VFC_LinkedObject:
        LinkedObject *lp;
        return Detail_Base_Report (cmd, exp, topn, tgrp, CV, MV, IV, HV,
                                   false, lp, vfc, &dummyVector, view_output);
       case VFC_Statement:
        Statement *sp;
        return Detail_Base_Report (cmd, exp, topn, tgrp, CV, MV, IV, HV,
                                   false, sp, vfc, &dummyVector, view_output);
      }
      Mark_Cmd_With_Soft_Error(cmd, "(There is no supported view name supplied.)");
      return false;
    }
    Mark_Cmd_With_Soft_Error(cmd, "(There is no requested information to report for 'hwcsamp' view.)");
    return false;
  }
};

// This is the only external entrypoint.
// Calls to the VIEWs needs to be done through the ViewType class objects.
extern "C" void hwcsamp_view_LTX_ViewFactory () {
  Define_New_View (new hwcsamp_view());
}