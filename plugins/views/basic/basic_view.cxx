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


#include "SS_Input_Manager.hxx"

using namespace std;
using namespace OpenSpeedShop::cli;


// pcsamp view

static std::string allowed_pcsamp_V_options[] = {
  "LinkedObject",
  "LinkedObjects",
  "Dso",
  "Dsos",
  "Function", 
  "Functions",
  "Statement",
  "Statements",
  "Summary",
  "data",       // Raw data output for scripting
  ""
};


static bool define_pcsamp_columns (
            CommandObject *cmd,
            ExperimentObject *exp,
            std::vector<Collector>& CV,
            std::vector<std::string>& MV,
            std::vector<ViewInstruction *>& IV,
            std::vector<std::string>& HV) {
  OpenSpeedShop::cli::ParseResult *p_result = cmd->P_Result();
  vector<ParseRange> *p_slist = p_result->getexpMetricList();
  int64_t last_column = 0;

  bool Generate_Summary = Look_For_KeyWord(cmd, "Summary");

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
        if (strcasecmp(C_Name.c_str(), "pcsamp")) {
         // We only know what to do with the pcsamp collector.
          std::string s("The specified collector, " + C_Name +
                        ", can not be displayed as part of a 'pcsamp' view.");
          Mark_Cmd_With_Soft_Error(cmd,s);
          return false;
        }
        M_Name = m_range->end_range.name;
      } else {
        M_Name = m_range->start_range.name;
      }

      if (!strcasecmp(M_Name.c_str(), "time") ||
          !strcasecmp(M_Name.c_str(), "times")) {
        IV.push_back(new ViewInstruction (VIEWINST_Display_Metric, last_column++, 0));
        HV.push_back("CPU Time(ms)");
      } else if (!strcasecmp(M_Name.c_str(), "percent") ||
                 !strcmp(M_Name.c_str(), "%")           ||
                 !strcasecmp(M_Name.c_str(), "%time")   ||
                 !strcasecmp(M_Name.c_str(), "%times")) {
       // percent is calculate from 2 temps: time for this row and total time.
        IV.push_back(new ViewInstruction (VIEWINST_Define_Total, 0));  // total the metric in first column
        IV.push_back(new ViewInstruction (VIEWINST_Display_Percent_Column, last_column++, 0));  // second column is %
        HV.push_back("% of Total CPU Time");
      } else if (!strcasecmp(M_Name.c_str(), "ThreadMean") ||
                 !strcasecmp(M_Name.c_str(), "ThreadAverage")) {
       // Do a By-Thread average.
        IV.push_back(new ViewInstruction (VIEWINST_Display_ByThread_Metric, last_column++, 0, ViewReduction_mean));
        HV.push_back("Average CPU Time(ms) Across Threads");
      } else if (!strcasecmp(M_Name.c_str(), "ThreadMin")) {
       // Find the By-Thread Min.
        IV.push_back(new ViewInstruction (VIEWINST_Display_ByThread_Metric, last_column++, 0, ViewReduction_min));
        HV.push_back("Min CPU Time(ms) Across Threads");
      } else if (!strcasecmp(M_Name.c_str(), "ThreadMax")) {
       // Find the By-Thread Max.
        IV.push_back(new ViewInstruction (VIEWINST_Display_ByThread_Metric, last_column++, 0, ViewReduction_max));
        HV.push_back("Max CPU Time(ms) Across Threads");
      } else {
       // Unrecognized '-m' option.
        Mark_Cmd_With_Soft_Error(cmd,"Warning: Unsupported option, '-m " + M_Name + "'");
      }
    }

  } else {
   // If nothing is requested ...
   // There is only 1 supported metric.  Use it and also generate the percent.
    IV.push_back(new ViewInstruction (VIEWINST_Display_Metric, last_column++, 0));  // first column is metric
    IV.push_back(new ViewInstruction (VIEWINST_Define_Total, 0));  // total the metric in first column
    IV.push_back(new ViewInstruction (VIEWINST_Display_Percent_Column, last_column++, 0));  // second column is %
    HV.push_back("CPU Time(ms)");
    HV.push_back("% of Total CPU Time");
  }
  return (last_column > 0);
}

static std::string VIEW_pcsamp_brief = "PC (Program Counter) report";
static std::string VIEW_pcsamp_short = "Report the amount and percent of program time spent in a code unit.";
static std::string VIEW_pcsamp_long  = "The report is sorted in descending order by the amount of time that"
                                       " was used in each unit. Also included in the report is the"
                                       " percent of total time that each unit uses."
                                       " A positive integer can be added to the end of the keyword"
                                       " 'pcsamp' to indicate the maximum number of items in the report."
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
                                      " \n\t'-m ThreadAverage' reports the average cpu time for a process."
                                      " \n\t'-m ThreadMin' reports the minimum cpu time for a process."
                                      " \n\t'-m ThreadMin' reports the maximum cpu time for a process."
                                      "\n";
static std::string VIEW_pcsamp_example = "\texpView pcsamp\n"
                                         "\texpView -v statements pcsamp10\n";
static std::string VIEW_pcsamp_metrics[] =
  { "time",
    ""
  };
static std::string VIEW_pcsamp_collectors[] =
  { "pcsamp",
    ""
  };
class pcsamp_view : public ViewType {

 public: 
  pcsamp_view() : ViewType ("pcsamp",
                            VIEW_pcsamp_brief,
                            VIEW_pcsamp_short,
                            VIEW_pcsamp_long,
                            VIEW_pcsamp_example,
                           &VIEW_pcsamp_metrics[0],
                           &VIEW_pcsamp_collectors[0],
                            true) {
  }
  virtual bool GenerateView (CommandObject *cmd, ExperimentObject *exp, int64_t topn,
                             ThreadGroup& tgrp, std::list<CommandResult *>& view_output) {
    std::vector<Collector> CV;
    std::vector<std::string> MV;
    std::vector<ViewInstruction *>IV;
    std::vector<std::string> HV;
    MV.push_back(VIEW_pcsamp_metrics[0]);  // Use the Collector with the first metric

   // Warn about misspelled of meaningless options.
    Validate_V_Options (cmd, allowed_pcsamp_V_options);

   // Initialize the one supported metric.
    CV.push_back( Get_Collector (exp->FW(), VIEW_pcsamp_collectors[0]) ); // use pcsamp collector

   // Look for user override of '-m' options.
    if (!define_pcsamp_columns (cmd, exp, CV, MV, IV, HV)) {
      return false;
    }

    return Generic_View (cmd, exp, topn, tgrp, CV, MV, IV, HV, view_output);
  }
};

// Hardware Counter Report

static std::string allowed_hwc_V_options[] = {
  "LinkedObject",
  "LinkedObjects",
  "Dso",
  "Dsos",
  "Function",
  "Functions",
  "Statement",
  "Statements",
  "Summary",
  "data",       // Raw data output for scripting
  ""
};


static bool define_hwc_columns (
            CommandObject *cmd,
            ExperimentObject *exp,
            std::vector<Collector>& CV,
            std::vector<std::string>& MV,
            std::vector<ViewInstruction *>& IV,
            std::vector<std::string>& HV) {
  OpenSpeedShop::cli::ParseResult *p_result = cmd->P_Result();
  vector<ParseRange> *p_slist = p_result->getexpMetricList();
  int64_t last_column = 0;

  bool Generate_Summary = Look_For_KeyWord(cmd, "Summary");
  
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
        if (strcasecmp(C_Name.c_str(), "hwc")) {
         // We only know what to do with the hwc collector.
          std::string s("The specified collector, " + C_Name +
                        ", can not be displayed as part of a 'hwc' view.");
          Mark_Cmd_With_Soft_Error(cmd,s);
          return false;
        }
        M_Name = m_range->end_range.name;
      } else {
        M_Name = m_range->start_range.name;
      }

      if (!strcasecmp(M_Name.c_str(), "overflow") ||
          !strcasecmp(M_Name.c_str(), "overflows")) {
        Collector C = Get_Collector (exp->FW(), "hwc");
        IV.push_back(new ViewInstruction (VIEWINST_Display_Metric, last_column++, 0));
       // Get the name of the event that we were collecting.
       // Use this for the column header in the report rather then the name of the metric.
        std::string H;
        C.getParameterValue ("event", H);
        HV.push_back(H);
      } else if (!strcasecmp(M_Name.c_str(), "percent") ||
                 !strcmp(M_Name.c_str(), "%")           ||
                 !strcasecmp(M_Name.c_str(), "%overflow")   ||
                 !strcasecmp(M_Name.c_str(), "%overflows")) {
       // percent is calculate from 2 temps: time for this row and total time.
        IV.push_back(new ViewInstruction (VIEWINST_Define_Total, 0));  // total the metric in first column
        IV.push_back(new ViewInstruction (VIEWINST_Display_Percent_Column, last_column++, 0));  // second column is %
        HV.push_back("% of Total Overflows");
      } else if (!strcasecmp(M_Name.c_str(), "ThreadMean") ||
                 !strcasecmp(M_Name.c_str(), "ThreadAverage")) {
       // Do a By-Thread average.
        IV.push_back(new ViewInstruction (VIEWINST_Display_ByThread_Metric, last_column++, 0, ViewReduction_mean));
        HV.push_back("Average Overflows Across Threads");
      } else if (!strcasecmp(M_Name.c_str(), "ThreadMin")) {
       // Find the By-Thread Min.
        IV.push_back(new ViewInstruction (VIEWINST_Display_ByThread_Metric, last_column++, 0, ViewReduction_min));
        HV.push_back("Minimum Overflows Across Threads");
      } else if (!strcasecmp(M_Name.c_str(), "ThreadMax")) {
       // Find the By-Thread Max.
        IV.push_back(new ViewInstruction (VIEWINST_Display_ByThread_Metric, last_column++, 0, ViewReduction_max));
        HV.push_back("Maximum Overflows Across Threads");
      } else {
       // Unrecognized '-m' option.
        Mark_Cmd_With_Soft_Error(cmd,"Warning: Unsupported option, '-m " + M_Name + "'");
      }
    }

  } else {
   // If nothing is requested ...
   // There is only 1 supported metric.  Use it and also generate the percent.
    IV.push_back(new ViewInstruction (VIEWINST_Display_Metric, last_column++, 0));  // first column is metric
    IV.push_back(new ViewInstruction (VIEWINST_Define_Total, 0));  // metric is total
    IV.push_back(new ViewInstruction (VIEWINST_Display_Percent_Column, last_column++, 0));  // second column is % of first

   // Get the name of the event that we were collecting.
   // Use this for the column header in the report rather then the name of the metric.
    std::string H;
    CV[0].getParameterValue ("event", H);
    HV.push_back(H);
    HV.push_back("% of Total Overflows");
  }
  return (last_column > 0);
}

static std::string VIEW_hwc_brief = "Hardware counter report";
static std::string VIEW_hwc_short = "Report the hardware counts spent in a code unit.";
static std::string VIEW_hwc_long  = "The report is sorted in descending order by the number of counts that"
                                    " were accumulated in each unit.  The reported counter is the one"
                                    " set in the 'event' parameter when the experiment was run."
                                    " Also included in the report is the percent of total cycles"
                                    " that each unit uses."
                                    " A positive integer can be added to the end of the keyword 'hwc'"
                                    " to indicate the maximum number of items in the report."
                                    "\n\nThe type of unit displayed can be selected with the '-v'"
                                    " option."
                                    "\n\t'-v LinkedObjects' will report counts by linked object."
                                    "\n\t'-v Functions' will report counts by function. This is the default."
                                    "\n\t'-v Statements' will report counts by statement."
                                    "\n\nThe information included in the report can be controlled with the"
                                    " '-m' option.  More than one item can be selected but only the items"
                                    " listed after the option will be printed and they will be printed in"
                                    " the order that they are listed."
                                    " If no '-m' option is specified, the default is equivalent to"
                                    " '-m overflows, percent'."
                                    " Each value pertains to the function, statement or linked object that is"
                                    " on that row of the report.  The 'Thread...' selections pertain to the"
                                    " process unit that the program was partitioned into: Pid's,"
                                    " Posix threads, Mpi threads or Ranks."
                                    " \n\t'-m overflows' reports the total counts for all the processes."
                                    " \n\t'-m percent' reports the percent of total counts for all the processes."
                                    " \n\t'-m ThreadAverage' reports the average counts for a process."
                                    " \n\t'-m ThreadMin' reports the minimum number of counts for a process."
                                    " \n\t'-m ThreadMin' reports the maximum counts for a process."
                                    "\n";
static std::string VIEW_hwc_example = "\texpView hwc\n"
                                      "\texpView -v Functions hwc10\n";
static std::string VIEW_hwc_metrics[] =
  { "overflows",
    ""
  };
static std::string VIEW_hwc_collectors[] =
  { "hwc",
    ""
  };
class hwc_view : public ViewType {

 public: 
  hwc_view() : ViewType ("hwc",
                         VIEW_hwc_brief,
                         VIEW_hwc_short,
                         VIEW_hwc_long,
                         VIEW_hwc_example,
                        &VIEW_hwc_metrics[0],
                        &VIEW_hwc_collectors[0],
                         true) {
  }
  virtual bool GenerateView (CommandObject *cmd, ExperimentObject *exp, int64_t topn,
                             ThreadGroup& tgrp, std::list<CommandResult *>& view_output) {
    std::vector<Collector> CV;
    std::vector<std::string> MV;
    std::vector<ViewInstruction *>IV;
    std::vector<std::string> HV;

   // Warn about misspelled of meaningless options.
    Validate_V_Options (cmd, allowed_hwc_V_options);
  
   // Initialize the one supported metric.
    CV.push_back( Get_Collector (exp->FW(), VIEW_hwc_collectors[0]) ); // use hwc collector
    MV.push_back("overflows");
    
   // Look for user override of '-m' options.
    if (!define_hwc_columns (cmd, exp, CV, MV, IV, HV)) {
      return false;
    }

    return Generic_View (cmd, exp, topn, tgrp, CV, MV, IV, HV, view_output);
  }
};


// This is the only external entrypoint.
// Calls to the VIEWs needs to be done through the ViewType class objects.
extern "C" void basic_view_LTX_ViewFactory () {
  Define_New_View (new pcsamp_view());
  Define_New_View (new hwc_view());
}
