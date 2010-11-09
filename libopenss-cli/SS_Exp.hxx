/******************************************************************************
** Copyright (c) 2005 Silicon Graphics, Inc. All Rights Reserved.
** Copyright (c) 2007-2010 Krell Institute  All Rights Reserved.
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

//#define DEBUG_SYNC_SIGNAL 1
//#define DEBUG_CLI 1

/** @file
 *
 * Definition of the ExperimentObject class.
 *
 */



// ExperimentObject
// Note: ALL the instrumentation in the experiment's list is applied
//       to ALL of the executables in the experiment's list.
class ExperimentObject;
extern EXPID Experiment_Sequence_Number;
extern pthread_mutex_t Experiment_List_Lock;
extern std::list<ExperimentObject *> ExperimentObject_list;
extern void openss_error(char *);

#define ExpStatus_NonExistent 0
#define ExpStatus_Paused      1
#define ExpStatus_Running     2
#define ExpStatus_Terminated  3
#define ExpStatus_InError     4

class ExperimentObject
{
 private:
  EXPID Exp_ID;
  int ExpStatus;
  bool Data_File_Has_A_Generated_Name;
  OpenSpeedShop::Framework::Experiment *FW_Experiment;
  pthread_mutex_t Experiment_Lock;
  bool exp_reserved;
  bool expRunAtLeastOnceAlreadyFlag;
  std::list<CommandObject *> waiting_cmds;
  bool isBatch;
  bool isInstrumentorOffline;
  std::string offlineAppCommand;

 public:

  std::list<std::string> offlineCollectorList;

  ExperimentObject (std::string data_base_name = std::string(""), EXPID preferred_ID = 0) {

    Assert(pthread_mutex_lock(&Experiment_List_Lock) == 0);

#ifdef DEBUG_CLI
    std::cerr << "Enter ExperimentObject, preferred_ID=" << preferred_ID << "Experiment_Sequence_Number=" << Experiment_Sequence_Number << "\n";
#endif

    // preferred_ID indicates that this is an offline experiment that wants to write over the previous ExperimentObject
    if (preferred_ID != 0) {

#ifdef DEBUG_CLI
      std::cerr << "ExperimentObject, preferred_ID=" << preferred_ID << "Experiment_Sequence_Number=" << Experiment_Sequence_Number << "\n";
#endif

      Assert (Experiment_Sequence_Number <= preferred_ID);

      Exp_ID = preferred_ID;
      Experiment_Sequence_Number = preferred_ID;

    } else {

      Exp_ID = ++Experiment_Sequence_Number;

#ifdef DEBUG_CLI
      std::cerr << "ExperimentObject, preferred_ID=" << preferred_ID << " after updating Experiment_Sequence_Number=" << Experiment_Sequence_Number << " Exp_ID=" << Exp_ID << "\n";
#endif
    } 
//    Exp_ID = ++Experiment_Sequence_Number;
    ExpStatus = ExpStatus_Paused;
    exp_reserved = false;
    expRunAtLeastOnceAlreadyFlag = false;
    exp_rerun_count = 0;

    // Determine real value for this from the expCreate command (-i offline)
    setIsInstrumentorOffline(false);
    offlineCollectorList.clear();

    Assert(pthread_mutex_init(&Experiment_Lock, NULL) == 0); // dynamic initialization
    Assert(pthread_mutex_lock(&Experiment_Lock) == 0);       // Lock it!

   // Allocate a data base file for the information connected with the experiment.
    std::string Data_File_Name;
    if (data_base_name.length() == 0) {
      bool database_not_allocated = true;
      Data_File_Has_A_Generated_Name = !OPENSS_SAVE_EXPERIMENT_DATABASE;
      if (OPENSS_SAVE_EXPERIMENT_DATABASE) {
       // Try to create a file in the current directory
       // of the form "X<exp_id>.XXXX.openss".
       // If the generated name already exists, increment
       // the "XXXX" field and try again with the new name.
       // If we try 1000 times and can't generate a unique name,
       // fall into logic to use 'tempnam' to generate a name.
        char base[256];
        int64_t cnt = 0;
        for (cnt = 0; cnt < 1000; cnt++) {
          char *database_directory = getenv("OPENSS_DB_DIR");
          if (database_directory) {
             snprintf(base, 256, "%s/X%lld.%lld.openss",database_directory, Exp_ID,cnt);
           } else {
             snprintf(base, 256, "./X%lld.%lld.openss",Exp_ID,cnt);
           } 
      
          int fd;
          if ((fd = open(base, O_RDONLY)) != -1) {
           // File name is already used!
            Assert(close(fd) == 0);
            continue;
          }
          Data_File_Name = std::string(base);
          database_not_allocated = false;
          break;
        }
      }
      if (database_not_allocated) {   
       // Create a file in /tmp, for fastest access,
       // of the form "X<exp_id>.XXXXXX.openss".
//        char base[L_tmpnam];
//        snprintf(base, L_tmpnam, "X%lld.",Exp_ID);
//        char *tName = tempnam ( (Data_File_Has_A_Generated_Name ? NULL : "./"), base );
//        Assert(tName != NULL);
//        Data_File_Name = std::string(tName) + ".openss";
//        free (tName);
        Data_File_Name = createName(Exp_ID, Data_File_Has_A_Generated_Name);
      }
      try {
        OpenSpeedShop::Framework::Experiment::create (Data_File_Name);
      }
      catch(const Exception& error) {
       // Don't really care why.
       // Calling routine must handle the problem.
        Exp_ID = 0;
        Data_File_Has_A_Generated_Name = false;
        FW_Experiment = NULL;
      }
    } else {
      Data_File_Name = data_base_name;
      Data_File_Has_A_Generated_Name = false;
    }

   // Create and open an experiment
    try {

#ifdef DEBUG_CLI
      std::cerr << "try create/open experiment preferred_ID=" << preferred_ID << " Experiment_Sequence_Number=" << Experiment_Sequence_Number << "\n";
#endif
      FW_Experiment = new OpenSpeedShop::Framework::Experiment (Data_File_Name);
      if (preferred_ID == 0) {

#ifdef DEBUG_CLI
        std::cerr << "Creating new experiment ID, pushing this to ExperimentObject_list, Experiment_Sequence_Number=" << Experiment_Sequence_Number << "\n";
#endif

        ExperimentObject_list.push_front(this);

      } else {

#ifdef DEBUG_CLI
        std::cerr << "NOT CREATING A NEW FRAMEWORK EXPERIMENT because preferred_ID=" << preferred_ID << "\n";
        std::cerr << "POP original experiment object and push this to ExperimentObject_list because preferred_ID=" << preferred_ID << "\n";
#endif
        ExperimentObject_list.pop_back();
        ExperimentObject_list.push_front(this);
      }
    }
    catch(const Exception& error) {
     // Don't really care why.
     // Calling routine must handle the problem.
      Exp_ID = 0;
      Data_File_Has_A_Generated_Name = false;
      FW_Experiment = NULL;
    }

    Assert(pthread_mutex_unlock(&Experiment_Lock) == 0);       // Unlock new experiment
    Assert(pthread_mutex_unlock(&Experiment_List_Lock) == 0);

#ifdef DEBUG_CLI
    std::cerr << "Exit ExperimentObject, Experiment_Sequence_Number=" << Experiment_Sequence_Number << "\n";
#endif

  }


  ~ExperimentObject () {
    Assert(pthread_mutex_lock(&Experiment_List_Lock) == 0);
    ExperimentObject_list.remove (this);
    Assert(pthread_mutex_unlock(&Experiment_List_Lock) == 0);

    if (FW_Experiment != NULL) {
      try {
        std::string Data_File_Name;
        if (Data_File_Has_A_Generated_Name) {
         // Delete the file AFTER deleting the experiment object.
          Data_File_Name = FW_Experiment->getName();
        }
        delete FW_Experiment;
        if (Data_File_Has_A_Generated_Name) {
          OpenSpeedShop::Framework::Experiment::remove (Data_File_Name);
        }
      }
      catch(const Exception& error) {
       // Don't really care why.
      }
      Data_File_Has_A_Generated_Name = false;
      FW_Experiment = NULL;
    }
    Exp_ID = 0;
    ExpStatus = ExpStatus_NonExistent;
    pthread_mutex_destroy(&Experiment_Lock);
  }

  EXPID ExperimentObject_ID() {return Exp_ID;}
  Experiment *FW() {return FW_Experiment;}
  bool expRunAtLeastOnceAlready () {return expRunAtLeastOnceAlreadyFlag;}
  void setExpRunAtLeastOnceAlready (bool flag) {expRunAtLeastOnceAlreadyFlag = flag;}
  bool expIsBatch () {return isBatch;}
  void setExpIsBatch (bool flag) {isBatch = flag;}
  bool getIsInstrumentorOffline () {return isInstrumentorOffline;}
  void setIsInstrumentorOffline (bool flag) {isInstrumentorOffline = flag;}

  void setOfflineAppCommand(std::string appCmd) {
         offlineAppCommand = appCmd;
  }

  std::string getOfflineAppCommand() {
     return offlineAppCommand;
  }

  uint32_t exp_rerun_count;

  std::string createName (EXPID LocalExpId, bool LocalDataFileHasAGeneratedName) {
     std::string LocalDataFileName;
     // Create a file in /tmp, for fastest access,
     // of the form "X<exp_id>.XXXXXX.openss".
     char base[L_tmpnam];
     snprintf(base, L_tmpnam, "X%lld.",LocalExpId);
     char *database_directory = getenv("OPENSS_DB_DIR");
     char *tName = NULL;
     char tmp_tName[256];
     if (database_directory) {
        int64_t cnt = 0;
        for (cnt = 0; cnt < 1000; cnt++) {
         snprintf(tmp_tName, 256, "%s/X%lld.%lld.openss",database_directory, LocalExpId, cnt);
         Assert(tmp_tName != NULL);

         int fd;
         if ((fd = open(tmp_tName, O_RDONLY)) != -1) {
          // File name is already used!
           Assert(close(fd) == 0);
           continue;
         }
         LocalDataFileName = std::string(tmp_tName);
         break;
        }
     } else {
       tName = tempnam ( (LocalDataFileHasAGeneratedName ? NULL : "./"), base );
       Assert(tName != NULL);
       LocalDataFileName = std::string(tName) + ".openss";
       free (tName);
     }

     return LocalDataFileName;
  }

  std::string createRerunNameFromCurrentName (EXPID LocalExpId, uint32_t rerun_count, const char* current_exp_name) {
     std::string LocalDataFileName;
     // Create a file in /tmp, for fastest access,
     // of the form "X<exp_id>.XXXXXX.openss".
     char base[256];
     char newName[256];

#if DEBUG_CLI
     printf("[TID=%ld], createRerunNameFromCurrentName, newname=%s-%d.\n", pthread_self(), current_exp_name, rerun_count);
#endif

     int nameLen = strlen(current_exp_name);
     nameLen = nameLen - 7; // .openss has seven characters and we want to eliminate them
     strncpy (newName, current_exp_name, nameLen);
     newName[nameLen] = '\0';
     snprintf(base, 256, "%s-%d", newName, rerun_count);

#if DEBUG_CLI
     printf("[TID=%ld], createRerunNameFromCurrentName, base=%s, rerun_count=%ld\n", pthread_self(), base, rerun_count);
#endif

     Assert(base != NULL);
     LocalDataFileName = std::string(base) + ".openss";

#if DEBUG_CLI
     printf("[TID=%ld], createRerunNameFromCurrentName, LocalDataFileName.c_str()=%s\n", pthread_self(), LocalDataFileName.c_str());
#endif

     return LocalDataFileName;
  }

  bool Data_Base_Is_Tmp () {return Data_File_Has_A_Generated_Name;}
  std::string Data_Base_Name () {
    if (FW() == NULL) {
      return "";
    } else {
      try {
        return FW()->getName();
      }
      catch(const Exception& error) {
       // Don't really care why.
        return "(Unable to determine.)";
      }
    }
  }
  void RenameDB (std::string New_DB) {
   // Rename the data base file.
    if (FW_Experiment != NULL) {
      FW_Experiment->renameTo(New_DB);
      Data_File_Has_A_Generated_Name = false;
    }
  }
  void CopyDB (std::string New_DB) {
   // Make a copy of the data base file.
    if (FW_Experiment != NULL) {
      FW_Experiment->copyTo(New_DB);
    }
  }

  bool Has_Ranks() {
    ThreadGroup tgrp = FW()->getThreads();
    ThreadGroup::iterator ti = tgrp.begin();
    if (ti != tgrp.end()) {
      Thread t = *ti;
      std::pair<bool, int> prank = t.getMPIRank();
      return prank.first;
    }
    return false;
  }

  void Q_Lock (CommandObject *cmd, bool start_next_cmd = false) {

#ifdef DEBUG_SYNC_SIGNAL
    printf("[TID=%ld], Q_Lock, before calling pthread_mutex_lock(&Experiment_Lock=%ld), start_next_cmd=%ld\n", pthread_self(), Experiment_Lock, start_next_cmd);
#endif

    Assert(pthread_mutex_lock(&Experiment_Lock) == 0);

#ifdef DEBUG_SYNC_SIGNAL
    printf("[TID=%ld], Q_Lock, after calling pthread_mutex_lock(&Experiment_Lock=%ld), start_next_cmd=%ld\n", pthread_self(), Experiment_Lock, start_next_cmd);
#endif

    if (start_next_cmd) {

#ifdef DEBUG_SYNC_SIGNAL
     printf("[TID=%ld], Q_Lock, before calling SafeToDoNextCmd(), start_next_cmd=%ld\n", pthread_self(), start_next_cmd);
#endif
      
      SafeToDoNextCmd ();

#ifdef DEBUG_SYNC_SIGNAL
     printf("[TID=%ld], Q_Lock, after calling SafeToDoNextCmd(), start_next_cmd=%ld\n", pthread_self(), start_next_cmd);
#endif

    }

#ifdef DEBUG_SYNC_SIGNAL
    printf("[TID=%ld], Q_Lock, exp_reserved=%ld\n", pthread_self(), exp_reserved);
#endif

    if (exp_reserved) {
     // Queue myself up and wait until the previous
     // commands have completed execution.
      waiting_cmds.push_back(cmd);
      cmd->Wait_On_Dependency(Experiment_Lock);
     // When we return, the lock has been reset.
    }
    exp_reserved = true;

#ifdef DEBUG_SYNC_SIGNAL
    printf("[TID=%ld], Q_Lock, before calling pthread_mutex_unlock(&Experiment_Lock=%ld)\n", pthread_self(), Experiment_Lock);
#endif

    Assert(pthread_mutex_unlock(&Experiment_Lock) == 0);

#ifdef DEBUG_SYNC_SIGNAL
    printf("[TID=%ld], Q_Lock, after calling pthread_mutex_unlock(&Experiment_Lock=%ld)\n", pthread_self(), Experiment_Lock);
#endif

    return;
  }

  void Q_UnLock () {

#ifdef DEBUG_SYNC_SIGNAL
    printf("[TID=%ld], Q_UnLock, before calling pthread_mutex_lock(&Experiment_Lock=%ld), exp_reserved=%ld\n", pthread_self(), Experiment_Lock, exp_reserved);
#endif

    Assert(pthread_mutex_lock(&Experiment_Lock) == 0);

#ifdef DEBUG_SYNC_SIGNAL
    printf("[TID=%ld], Q_UnLock, after calling pthread_mutex_lock(&Experiment_Lock=%ld), exp_reserved=%ld\n", pthread_self(), Experiment_Lock, exp_reserved);
#endif

    if (waiting_cmds.begin() != waiting_cmds.end()) {
      CommandObject *cmd = *waiting_cmds.begin();
      waiting_cmds.pop_front();
      cmd->All_Clear ();
    }
    exp_reserved = false;

#ifdef DEBUG_SYNC_SIGNAL
    printf("[TID=%ld], Q_UnLock, before calling pthread_mutex_unlock(&Experiment_Lock=%ld), exp_reserved=%ld\n", pthread_self(), Experiment_Lock, exp_reserved);
#endif

    Assert(pthread_mutex_unlock(&Experiment_Lock) == 0);

#ifdef DEBUG_SYNC_SIGNAL
    printf("[TID=%ld], Q_UnLock, after calling pthread_mutex_unlock(&Experiment_Lock=%ld), exp_reserved=%ld\n", pthread_self(), Experiment_Lock, exp_reserved);
#endif

  }

  bool TS_Lock () {

#ifdef DEBUG_SYNC_SIGNAL
    printf("[TID=%ld], TS_LOCK, before calling pthread_mutex_lock(&Experiment_Lock=%ld), exp_reserved=%ld\n", pthread_self(), Experiment_Lock, exp_reserved);
#endif

    Assert(pthread_mutex_lock(&Experiment_Lock) == 0);

#ifdef DEBUG_SYNC_SIGNAL
    printf("[TID=%ld], TS_LOCK, before calling pthread_mutex_lock(&Experiment_Lock=%ld), exp_reserved=%ld\n", pthread_self(), Experiment_Lock, exp_reserved);
#endif

    bool already_in_use = exp_reserved;
    exp_reserved = true;

#ifdef DEBUG_SYNC_SIGNAL
    printf("[TID=%ld], TS_LOCK, before calling pthread_mutex_unlock(&Experiment_Lock=%ld), already_in_use=%ld, exp_reserved=%ld\n", 
            pthread_self(), Experiment_Lock, already_in_use, exp_reserved);
#endif

    Assert(pthread_mutex_unlock(&Experiment_Lock) == 0);

#ifdef DEBUG_SYNC_SIGNAL
    printf("[TID=%ld], TS_LOCK, after calling pthread_mutex_unlock(&Experiment_Lock=%ld), already_in_use=%ld, exp_reserved=%ld\n", 
            pthread_self(), Experiment_Lock, already_in_use, exp_reserved);
#endif

    return !already_in_use;
  }

  bool CanExecute () {
    bool itcan = false;
    if (FW() != NULL) {
      if (TS_Lock()) {
        try {
          ThreadGroup tgrp = FW()->getThreads();
          if (!tgrp.empty()) itcan = true;
        }
        catch(const Exception& error) {
         // Don't care
        }
        Q_UnLock();
      }
    }
    return itcan;

  }

  int Status() { return ExpStatus; }
  void setStatus (int S) {ExpStatus = S;}
  int Determine_Status();
  std::string ExpStatus_Name ();

  void Print_Waiting (std::ostream &mystream);
  void Print(std::ostream &mystream);
};

// Make sure all experiments are closed and associated files freed.
void Experiment_Termination ();

// Experiment Utilities
ExperimentObject *Find_Experiment_Object (EXPID ExperimentID);

bool Collector_Used_In_Experiment (OpenSpeedShop::Framework::Experiment *fexp, std::string myname);
Collector Get_Collector (OpenSpeedShop::Framework::Experiment *fexp, std::string myname);
bool Filter_Uses_F (CommandObject *cmd);
void Filter_ThreadGroup (OpenSpeedShop::cli::ParseResult *p_result, ThreadGroup& tgrp);

// Error reporting and stopping the presses
inline void Mark_Cmd_With_Std_Error (CommandObject *cmd, const Exception& error) {
   cmd->Result_String ( error.getDescription() );
   cmd->set_Status(CMD_ERROR);
   
   // Put in python exception
   openss_error((char *)error.getDescription().c_str());
   return;
}

// Error reporting and stopping the presses only 
// for scripting
inline void 
Mark_Cmd_With_Soft_Error (CommandObject *cmd, const std::string S) {
   cmd->Result_String ( S );
   cmd->set_Status(CMD_ERROR);
   
   // Put in python exception
   openss_error((char *)S.c_str());
   return;
}

//  Just put out the message with out error reporting
inline void 
Mark_Cmd_With_Message (CommandObject *cmd, const std::string S) {
   cmd->Result_String ( S );
   return;
}
