typedef int64_t EXPID;
typedef int64_t CMDID;
typedef int64_t CMDWID;

// ResultObject
enum RStatus { SUCCESS,
               PARTIAL_SUCCESS,
               FAILURE,
               EXIT };
class ResultObject
{
 private:
  RStatus status;    // Did the command complete or not...
  std::string result_type;
  std::string msg_string;
  int64_t num_results;
  void *result;      // A pointer to the data in the FrameWork.    The
                     // gui or cli should know how to interpret the data
                     // base on the associated commandObject and the
                     // result_type.

 public:
  void SetResultObject_StatusError (RStatus s, std::string e)
    { status = s;
      msg_string = e;
    }
  void SetResultObject_TypeValue (std::string t, void *r)
    { result_type = t;
      result = r;
    }

  // Constructors
  ResultObject ()
    { status = FAILURE;
      result_type = std::string("");
      result = NULL;
      msg_string = std::string("");
    }
  ResultObject (enum RStatus s)
    { *this = ResultObject ();
      status = s;
    }
  ResultObject (enum RStatus s, std::string t, void *r, std::string e)
    { *this = ResultObject ();
      SetResultObject_StatusError (s, e);
      SetResultObject_TypeValue (t, r);
    }
  ResultObject (enum RStatus s, char *t, void *r, char * e)
    { *this = ResultObject ( s, std::string(t ? t : ""), r, std::string(e ? e : ""));
    }
  ResultObject (RStatus s, std::string e)
    { *this = ResultObject ();
      SetResultObject_StatusError (s, e);
    }
  ResultObject (RStatus s, char * e)
    { *this = ResultObject ( s, std::string(e ? e : ""));
    }

  RStatus Status () { return status; }
  std::string Type () { return result_type; }
  void *Result () { return result; }
  std:: string ResultMsg () { return msg_string; }
};

class CodeObjectLocator
{
 private:
  std::string Host_Name;
  std::string File_Name;
  std::string PID_Name;
  std::string Rank_Name;
  std::string Thread_Name;

 public:
  CodeObjectLocator () {
    Host_Name = File_Name = PID_Name = Rank_Name = Thread_Name = std::string("");
  }
  CodeObjectLocator (std::string H, std::string F, std::string P, std::string R, std::string T) {
    Host_Name = H;
    File_Name = F;
    PID_Name  = P;
    Rank_Name = R;
    Thread_Name = T;
  }

  void Dump (FILE *TFile) {
    if (Host_Name.length()) fprintf(TFile,"-h %s ",Host_Name.c_str());
    if (File_Name.length()) fprintf(TFile,"-f %s ",File_Name.c_str());
    if (PID_Name.length()) fprintf(TFile,"-p %s ",PID_Name.c_str());
    if (Rank_Name.length()) fprintf(TFile,"-r %s ",Rank_Name.c_str());
    if (Thread_Name.length()) fprintf(TFile,"-t %s ",Thread_Name.c_str());
  }
  void dump (FILE *TFile) {
    this->Dump(TFile);
  }
};

// Forward class definitions:
class CommandObject; // defined in CommandObject.hxx
class InputLineObject; // defined in Clip.hxx
class CommandWindowID; // defined in Commander.cxx

// Command WIndows provide a way to get textual commands into the OpendSpeedShop tool.
CMDWID Commander_Initialization (char *my_name, char *my_host, pid_t my_pid, int64_t my_panel, bool Input_is_Async);
CMDWID Default_Window (char *my_name, char *my_host, pid_t my_pid, int64_t my_panel, bool Input_is_Async);
CMDWID TLI_Window     (char *my_name, char *my_host, pid_t my_pid, int64_t my_panel, bool Input_is_Async);
CMDWID GUI_Window     (char *my_name, char *my_host, pid_t my_pid, int64_t my_panel, bool Input_is_Async);
CMDWID RLI_Window     (char *my_name, char *my_host, pid_t my_pid, int64_t my_panel, bool Input_is_Async);

void Default_TLI_Line_Output (InputLineObject *clip);
void Default_TLI_Command_Output (CommandObject *C);

extern CMDWID command_line_window;
extern CMDWID tli_window;
extern CMDWID gui_window;
extern char *Current_OpenSpeedShop_Prompt;
extern char *Alternate_Current_OpenSpeedShop_Prompt;
inline void SS_Issue_Prompt (FILE *TFile) {
  fprintf(TFile,"%s",Current_OpenSpeedShop_Prompt);
  fflush(TFile);
}

void Commander_Termination (CMDWID my_window);

// Selection of items in the trace file are controlled throught his enum.
// Except for raw data dumps, the record identifier is stripped from output.
enum Trace_Entry_Type
  { CMDW_TRACE_ALL,                  // Dump raw data
    CMDW_TRACE_COMMANDS,             // Dump command line records with added information - "C "
    CMDW_TRACE_ORIGINAL_COMMANDS,    // Dump command line recoreds as entered by user - "C "
    CMDW_TRACE_RESULTS               // Dump result records  - "R "
  };

// Main readline interface
InputLineObject *SpeedShop_ReadLine (int is_more);

/* Global Data for tracking the current command line. */
extern InputLineObject *Current_ILO;
extern CommandObject   *Current_CO;

// Attach a new input source that will be read AFTER all the previous ones
bool Append_Input_File (CMDWID issuedbywindow, std::string fromfname,
                                      void (*CallBackLine) (InputLineObject *b) = NULL,
                                      void (*CallBackCmd) (CommandObject *b) = NULL);
InputLineObject *Append_Input_String (CMDWID issuedbywindow, char *b_ptr,
                                      void *LocalCmdId = NULL,
                                      void (*CallBackLine) (InputLineObject *b) = NULL,
                                      void (*CallBackCmd) (CommandObject *b) = NULL);

// Attach a new input source that will be read BEFORE all the previous ones
bool Push_Input_File (CMDWID issuedbywindow, std::string fromfname,
                                      void (*CallBackLine) (InputLineObject *b) = NULL,
                                      void (*CallBackCmd) (CommandObject *b) = NULL);

// Manipulate tracing options
bool Command_Trace (CommandObject *cmd, enum Trace_Entry_Type trace_type,
                    CMDWID cmdwinid, std::string tofname);
bool Command_Trace_OFF (CMDWID WindowID);
bool Command_Trace_ON (CMDWID WindowID, std::string tofname);

// Focus is a property of the Command Window that issued the command.
EXPID Experiment_Focus (CMDWID WindowID);
EXPID Experiment_Focus (CMDWID WindowID, EXPID ExperimentID);
void List_CommandWindows ( FILE *TFile );

// Communicate command information the window manager
extern void Link_Cmd_Obj_to_Input (InputLineObject *I, CommandObject *);
extern void Clip_Complete (InputLineObject *clip);
extern void Cmd_Obj_Complete (CommandObject *C);
