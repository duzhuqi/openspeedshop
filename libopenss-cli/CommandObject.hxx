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

// The CommandObject

// types of results that can be returned in a CommandObject
enum cmd_result_type_enum {
  CMD_RESULT_NULL,
  CMD_RESULT_UINT,
  CMD_RESULT_INT,
  CMD_RESULT_FLOAT,
  CMD_RESULT_STRING,
  CMD_RESULT_RAWSTRING,
  CMD_RESULT_FUNCTION,
  CMD_RESULT_STATEMENT,
  CMD_RESULT_LINKEDOBJECT,
  CMD_RESULT_CALLTRACE,
  CMD_RESULT_TIME,
  CMD_RESULT_DURATION,
  CMD_RESULT_INTERVAL,
  CMD_RESULT_TITLE,
  CMD_RESULT_COLUMN_HEADER,
  CMD_RESULT_COLUMN_VALUES,
  CMD_RESULT_COLUMN_ENDER,
  CMD_RESULT_EXTENSION,
};

class CommandResult {
  bool NullValue;
  cmd_result_type_enum Result_Type;

 private:
  CommandResult () {
    NullValue = false;
    Result_Type = CMD_RESULT_NULL; }

 public:
  CommandResult (cmd_result_type_enum T) {
    NullValue = false;
    Result_Type = T; }
  bool setNullValue () {
    NullValue = true;
  }
  bool isNullValue () {
    return NullValue;
  }
  cmd_result_type_enum Type () {
    return Result_Type;
  }
  virtual ~CommandResult () { }

  void Value (char *&C) {
    C = NULL;
  }

  virtual std::string Form () {
    return std::string("");
  }
  virtual PyObject * pyValue () {
    return Py_BuildValue("");
  }
  virtual void Print (ostream& to, int64_t fieldsize=20, bool leftjustified=false) {
    to << (leftjustified ? std::setiosflags(std::ios::left) : std::setiosflags(std::ios::right))
       << std::setw(fieldsize) << "(none)";
  }
};

// Reclaim CommandResult objects within other structures.
inline void Reclaim_CR_Space ( CommandResult *R) {
  if (R != NULL) delete R;
}

template <typename TL>
void Reclaim_CR_Space ( std::list<TL>& L) {
  typename std::list<TL>::iterator li;
  for ( li = L.begin(); li != L.end(); li++) {
    Reclaim_CR_Space (*li);
  }
}

template <typename TE>
void Reclaim_CR_Space ( std::map<TE, CommandResult *>& V) {
  typename std::map<TE, CommandResult *>::iterator vi;
  for ( vi = V.begin(); vi != V.end(); vi++) {
    Reclaim_CR_Space ((*vi).second);
  }
}

template <typename TV>
void Reclaim_CR_Space ( std::vector<TV>& V) {
  typename std::vector<TV>::iterator vi;
  for ( vi = V.begin(); vi != V.end(); vi++) {
    Reclaim_CR_Space (*vi);
  }
}

template  <typename TS>
void Reclaim_CR_Space ( std::pair<Function, TS>& P ) {
  Reclaim_CR_Space (P.second);
}

template  <typename TS>
void Reclaim_CR_Space ( std::pair<Statement, TS>& P ) {
  Reclaim_CR_Space (P.second);
}

template  <typename TS>
void Reclaim_CR_Space ( std::pair<LinkedObject, TS>& P ) {
  Reclaim_CR_Space (P.second);
}

template  <typename TF, typename TS>
void Reclaim_CR_Space ( std::pair<TF, TS>& P ) {
  Reclaim_CR_Space (P.first);
  Reclaim_CR_Space (P.second);
}

template  <typename TE>
void Reclaim_CR_Space ( SmartPtr<TE>& P) {
  Reclaim_CR_Space (*P);
}

template <typename TV>
void Reclaim_CR_Space (
              int64_t starting_with,
              std::vector<TV>& V) {
  typename std::vector<TV>::iterator vi;
  for (vi = (V.begin() + starting_with); vi != V.end(); vi++) {
    Reclaim_CR_Space (*vi);
  }
}

class CommandResult_Uint :
     public CommandResult {
  uint64_t uint_value;

 public:
  CommandResult_Uint () : CommandResult(CMD_RESULT_UINT) {
    uint_value = 0;
  }
  CommandResult_Uint (uint64_t U) : CommandResult(CMD_RESULT_UINT) {
    uint_value = U;
  }
  CommandResult_Uint (CommandResult_Uint *C) :
       CommandResult(CMD_RESULT_UINT) {
    uint_value = C->uint_value;
  }
  virtual ~CommandResult_Uint () { }

  void Min_Uint (CommandResult_Uint *B) {
    uint_value = min (uint_value, B->uint_value);
  }
  void Max_Uint (CommandResult_Uint *B) {
    uint_value = max (uint_value, B->uint_value);
  }
  void Accumulate_Uint (CommandResult_Uint *B) {
    uint_value += B->uint_value;
  }
  void Value (uint64_t& U) {
    U = uint_value;
  }

  virtual std::string Form () {
    char s[OPENSS_VIEW_FIELD_SIZE];
    sprintf ( s, "%lld", uint_value);
    return std::string (s);
  }
  virtual PyObject * pyValue () {
    return Py_BuildValue("l", uint_value);
  }
  virtual void Print (ostream& to, int64_t fieldsize, bool leftjustified) {
    to << (leftjustified ? std::setiosflags(std::ios::left) : std::setiosflags(std::ios::right))
       << std::setw(fieldsize) << uint_value;
  }
};

class CommandResult_Int :
     public CommandResult {
  int64_t int_value;

 public:
  CommandResult_Int () : CommandResult(CMD_RESULT_INT) {
    int_value = 0;
  }
  CommandResult_Int (CommandResult_Uint *U) : CommandResult(CMD_RESULT_INT) {
    uint64_t Uvalue;
    U->Value(Uvalue);
    int_value = Uvalue;
  }
  CommandResult_Int (int64_t I) : CommandResult(CMD_RESULT_INT) {
    int_value = I;
  }
  CommandResult_Int (CommandResult_Int *C) :
       CommandResult(CMD_RESULT_INT) {
    int_value = C->int_value;
  }
  virtual ~CommandResult_Int () { }

  void Min_Int (CommandResult_Int *B) {
    int_value = min (int_value, B->int_value);
  }
  void Max_Int (CommandResult_Int *B) {
    int_value = max (int_value, B->int_value);
  }
  void Accumulate_Int (CommandResult_Int *B) {
    int_value += B->int_value;
  }
  void Value (int64_t& I) {
    I = int_value;
  }

  virtual std::string Form () {
    char s[OPENSS_VIEW_FIELD_SIZE];
    sprintf ( s, "%lld", int_value);
    return std::string (s);
  }
  virtual PyObject * pyValue () {
    return Py_BuildValue("l", int_value);
  }
  virtual void Print (ostream& to, int64_t fieldsize, bool leftjustified) {
    to << (leftjustified ? std::setiosflags(std::ios::left) : std::setiosflags(std::ios::right))
       << std::setw(fieldsize) << int_value;
  }
};

class CommandResult_Float :
     public CommandResult {
  double float_value;

 public:
  CommandResult_Float () : CommandResult(CMD_RESULT_FLOAT) {
    float_value = 0.0;
  }
  CommandResult_Float (CommandResult_Uint *U) : CommandResult(CMD_RESULT_FLOAT) {
    uint64_t Uvalue;
    U->Value(Uvalue);
    float_value = Uvalue;
  }
  CommandResult_Float (CommandResult_Int *I) : CommandResult(CMD_RESULT_FLOAT) {
    int64_t Ivalue;
    I->Value(Ivalue);
    float_value = Ivalue;
  }
  CommandResult_Float (double f) : CommandResult(CMD_RESULT_FLOAT) {
    float_value = f;
  }
  CommandResult_Float (CommandResult_Float *C) :
       CommandResult(CMD_RESULT_FLOAT) {
    float_value = C->float_value;
  }
  virtual ~CommandResult_Float () { }

  void Min_Float (CommandResult_Float *B) {
    float_value = min (float_value, B->float_value);
  }
  void Max_Float (CommandResult_Float *B) {
    float_value = max (float_value, B->float_value);
  }
  void Accumulate_Float (CommandResult_Float *B) {
    float_value += B->float_value;
  }
  void Value (double& F) {
    F = float_value;
  }

  virtual std::string Form () {
    char F[20];
    F[0] = *("%"); // left justify in field
    sprintf(&F[1], "%lld.%lldf\0", OPENSS_VIEW_FIELD_SIZE, OPENSS_VIEW_PRECISION);

    char s[OPENSS_VIEW_FIELD_SIZE];
    sprintf ( s, &F[0], float_value);
    return std::string (s);
  }
  virtual PyObject * pyValue () {
    return Py_BuildValue("d", float_value);
  }
  virtual void Print (ostream& to, int64_t fieldsize, bool leftjustified) {
    to << (leftjustified ? std::setiosflags(std::ios::left) : std::setiosflags(std::ios::right))
       << std::setw(fieldsize) << Form ();
  }
};

class CommandResult_String :
     public CommandResult {
  std::string string_value;

 public:
  CommandResult_String (std::string S) : CommandResult(CMD_RESULT_STRING) {
    string_value = S;
  }
  CommandResult_String (char *S) : CommandResult(CMD_RESULT_STRING) {
    string_value = std::string(S);
  }
  CommandResult_String (CommandResult_String *C) :
       CommandResult(CMD_RESULT_STRING) {
    string_value = C->string_value;
  }
  virtual ~CommandResult_String () { }

  void Accumulate_String (CommandResult_String *B) {
    string_value += B->string_value;
  }
  void Value (std::string& S) {
    S = string_value;
  }

  virtual std::string Form () {
    return string_value;
  }
  virtual PyObject * pyValue () {
    return Py_BuildValue("s",string_value.c_str());
  }
  virtual void Print (ostream& to, int64_t fieldsize, bool leftjustified) {
    if (leftjustified) {
     // Left justification is only done on the last column of a report.
     // Don't truncate the string if it is bigger than the field size.
     // This is done to make sure everything gets printed.

      to << std::setiosflags(std::ios::left) << string_value;

     // If there is unused space in the field, pad with blanks.
      if ((string_value.length() < fieldsize) &&
          (string_value[string_value.length()-1] != *("\n"))) {
        for (int64_t i = string_value.length(); i < fieldsize; i++) to << " ";
      }

    } else {
     // Right justify the string in the field.
     // Don't let it exceed the size of the field.
     // Also, limit the size based on our internal buffer size.
      to << std::setiosflags(std::ios::right) << std::setw(fieldsize)
         << ((string_value.length() <= fieldsize) ? string_value : string_value.substr(0, fieldsize));
    }
  }
};

class CommandResult_RawString :
     public CommandResult {
  std::string string_value;

 public:
  CommandResult_RawString (std::string S) : CommandResult(CMD_RESULT_RAWSTRING) {
    string_value = S;
  }
  CommandResult_RawString (char *S) : CommandResult(CMD_RESULT_RAWSTRING) {
    string_value = std::string(S);
  }
  CommandResult_RawString (CommandResult_RawString *C) :
       CommandResult(CMD_RESULT_RAWSTRING) {
    string_value = C->string_value;
  }
  virtual ~CommandResult_RawString () { }

  void Accumulate_RawString (CommandResult_RawString *B) {
    string_value += B->string_value;
  }
  void Value (std::string& S) {
    S = string_value;
  }

  virtual std::string Form () {
    return string_value;
  }
  virtual PyObject * pyValue () {
    return Py_BuildValue("s",string_value.c_str());
  }
  virtual void Print (ostream& to, int64_t fieldsize, bool leftjustified) {
    // Ignore fieldsize and leftjustified specifications and just dump the
    // the raw string to the output stream.
    to << string_value;
  }
};

class CommandResult_Function :
     public CommandResult,
     public Function {
  std::set<Statement> ST;

 public:

  CommandResult_Function (Function F) :
       Framework::Function(F),
       CommandResult(CMD_RESULT_FUNCTION) {
  }
  CommandResult_Function  (Function F, std::set<Statement>& st) :
       Framework::Function(F),
       CommandResult(CMD_RESULT_FUNCTION) {
    ST = st;
  }
  CommandResult_Function  (CommandResult_Function *C) :
       Framework::Function(*C),
       CommandResult(CMD_RESULT_FUNCTION) {
    ST = C->ST;
  }
  virtual ~CommandResult_Function () { }

  void Value (std::string& F) {
    F = OPENSS_VIEW_MANGLED_NAME ? getMangledName() : getDemangledName();
  }
  void Value (std::set<Statement>& st) {
    st = ST;
  }

  virtual std::string Form () {
    std::string S = OPENSS_VIEW_MANGLED_NAME ? getMangledName() : getDemangledName();
    LinkedObject L = getLinkedObject();
    std::set<Statement> T = getDefinitions();

    if (OPENSS_VIEW_DEFINING_LOCATION) {
      S = S + "(" + ((OPENSS_VIEW_FULLPATH)
                        ? L.getPath()
                        : L.getPath().getBaseName());

      if (T.size() > 0) {
        std::set<Statement>::const_iterator ti;
        for (ti = T.begin(); ti != T.end(); ti++) {
          if (ti != T.begin()) {
            S += "  &...";
            break;
          }
          Statement s = *ti;
          char l[50];
          sprintf( &l[0], "%lld", (int64_t)s.getLine());
          S = S + ": "
                + ((OPENSS_VIEW_FULLPATH)
                        ? s.getPath()
                        : s.getPath().getBaseName())
                + "," + &l[0];
        }
      }
      S += ")";
    }

    return S;
  }
  virtual PyObject * pyValue () {
    std::string F = Form ();
    return Py_BuildValue("s",F.c_str());
  }

  virtual void Print (ostream& to, int64_t fieldsize, bool leftjustified) {
    std::string string_value = Form ();
    if (leftjustified) {
     // Left justification is only done on the last column of a report.
     // Don't truncate the string if it is bigger than the field size.
     // This is done to make sure everything gets printed.

      to << std::setiosflags(std::ios::left) << string_value;

     // If there is unused space in the field, pad with blanks.
      if ((string_value.length() < fieldsize) &&
          (string_value[string_value.length()-1] != *("\n"))) {
        for (int64_t i = string_value.length(); i < fieldsize; i++) to << " ";
      }

    } else {
     // Right justify the string in the field.
     // Don't let it exceed the size of the field.
     // Also, limit the size based on our internal buffer size.
      to << std::setiosflags(std::ios::right) << std::setw(fieldsize)
         << ((string_value.length() <= fieldsize) ? string_value : string_value.substr(0, fieldsize));
    }
  }
};

class CommandResult_Statement :
     public CommandResult,
     public Statement {

 public:
  CommandResult_Statement (Statement S) :
       Framework::Statement(S),
       CommandResult(CMD_RESULT_STATEMENT) {
  }
  CommandResult_Statement (CommandResult_Statement *C) :
       Framework::Statement(*C),
       CommandResult(CMD_RESULT_STATEMENT) {
  }
  virtual ~CommandResult_Statement () { }

  void Value (int64_t& L) {
    L = (int64_t)getLine();
  }

  virtual std::string Form () {
    char l[20];
    sprintf( &l[0], "%lld", (int64_t)getLine());
    std::string S = (OPENSS_VIEW_FULLPATH)
                        ? getPath()
                        : getPath().getBaseName();
    S = S + "(" + std::string(l) + ")";
    return S;
  }
  virtual PyObject * pyValue () {
    std::string S = Form ();
    return Py_BuildValue("s",S.c_str());
  }
  virtual void Print (ostream& to, int64_t fieldsize, bool leftjustified) {
    std::string string_value = Form ();
    if (leftjustified) {
     // Left justification is only done on the last column of a report.
     // Don't truncate the string if it is bigger than the field size.
     // This is done to make sure everything gets printed.

      to << std::setiosflags(std::ios::left) << string_value;

     // If there is unused space in the field, pad with blanks.
      if ((string_value.length() < fieldsize) &&
          (string_value[string_value.length()-1] != *("\n"))) {
        for (int64_t i = string_value.length(); i < fieldsize; i++) to << " ";
      }

    } else {
     // Right justify the string in the field.
     // Don't let it exceed the size of the field.
     // Also, limit the size based on our internal buffer size.
      to << std::setiosflags(std::ios::right) << std::setw(fieldsize)
         << ((string_value.length() <= fieldsize) ? string_value : string_value.substr(0, fieldsize));
    }
  }
};

class CommandResult_LinkedObject :
     public CommandResult,
     public LinkedObject {
  uint64_t Offset;

 public:

  CommandResult_LinkedObject (LinkedObject F) :
       Framework::LinkedObject(F),
       CommandResult(CMD_RESULT_LINKEDOBJECT) {
  }
  CommandResult_LinkedObject (LinkedObject F, uint64_t off) :
       Framework::LinkedObject(F),
       CommandResult(CMD_RESULT_LINKEDOBJECT) {
    Offset = off;
  }
  CommandResult_LinkedObject  (CommandResult_LinkedObject *C) :
       Framework::LinkedObject(*C),
       CommandResult(CMD_RESULT_LINKEDOBJECT) {
    Offset = C->Offset;
  }
  virtual ~CommandResult_LinkedObject () { }

  void Value (std::string& L) {
    L = (OPENSS_VIEW_FULLPATH)
                      ? getPath()
                      : getPath().getBaseName();
  }
  void Value (uint64_t& off) {
    off = Offset;
  }

  virtual std::string Form () {
    std::string S = (OPENSS_VIEW_FULLPATH)
                      ? getPath()
                      : getPath().getBaseName();
    return S;
  }
  virtual PyObject * pyValue () {
    std::string F = Form ();
    return Py_BuildValue("s",F.c_str());
  }
  virtual void Print (ostream& to, int64_t fieldsize, bool leftjustified) {
    std::string string_value = Form ();
    if (leftjustified) {
     // Left justification is only done on the last column of a report.
     // Don't truncate the string if it is bigger than the field size.
     // This is done to make sure everything gets printed.

      to << std::setiosflags(std::ios::left) << string_value;

     // If there is unused space in the field, pad with blanks.
      if ((string_value.length() < fieldsize) &&
          (string_value[string_value.length()-1] != *("\n"))) {
        for (int64_t i = string_value.length(); i < fieldsize; i++) to << " ";
      }

    } else {
     // Right justify the string in the field.
     // Don't let it exceed the size of the field.
     // Also, limit the size based on our internal buffer size.
      to << std::setiosflags(std::ios::right) << std::setw(fieldsize)
         << ((string_value.length() <= fieldsize) ? string_value : string_value.substr(0, fieldsize));
    }
  }
};

CommandResult *Dup_CommandResult (CommandResult *C);  // forward definition
class CommandResult_CallStackEntry : public CommandResult {
 private:

  class CallStackEntry {
   public:
    std::vector<CommandResult *> *CSV;
    CallStackEntry () {
    }
    CallStackEntry (std::vector<CommandResult *> *call_vector) {
      CSV = call_vector;
    }
    ~CallStackEntry () {
      Reclaim_CR_Space (*CSV);
    }
  };

  bool Bottom_up;
  SmartPtr<CallStackEntry> CallStack;

  CommandResult_CallStackEntry () : CommandResult(CMD_RESULT_CALLTRACE) {
    Bottom_up = false;
  }

 public:
  CommandResult_CallStackEntry (std::vector<CommandResult *> *call_vector,
                                bool Reverse=false)
      : CommandResult(CMD_RESULT_CALLTRACE) {
    Bottom_up = Reverse;
    if (CallStack.isNull()) {
      CallStack = SmartPtr<CallStackEntry>(new CallStackEntry(call_vector));
    } else {
      CallStack->CSV = call_vector;
    }
  }
  CommandResult_CallStackEntry (CommandResult_CallStackEntry *CSE)
      : CommandResult(CMD_RESULT_CALLTRACE) {
    int64_t len = CSE->CallStack->CSV->size();
    Bottom_up = CSE->Bottom_up;
    CallStack = CSE->CallStack;
  }
  virtual ~CommandResult_CallStackEntry () {
   // Only delete std::vector<CommandResult *> when last SmartPtr is released.
  }

  std::vector<CommandResult *> *Value () {
    return (CallStack->CSV);
  };

  virtual std::string Form () {
    int64_t sz = CallStack->CSV->size();
    if (sz <= 0) return std::string("");
    std::vector<CommandResult *> *C1 = CallStack->CSV;
    CommandResult *CE = (*C1)[sz - 1];
    std::string Name;
   // Add indentation.
    for (int64_t i = 1; i < sz; i++) {
      Name += ((Bottom_up) ? "<" : ">");
    }
   // Add line number.
    if (sz > 1) {
      if (CE->Type() == CMD_RESULT_FUNCTION) {
        std::set<Statement> T;
        ((CommandResult_Function *)CE)->Value(T);
        if (T.begin() != T.end()) {
          std::set<Statement>::const_iterator sti = T.begin();;
          Statement S = *sti;
          char l[50];
          sprintf( &l[0], "%lld", (int64_t)(S.getLine()));
          Name = Name + " @ " + l + " in ";
        }
      } else if (CE->Type() == CMD_RESULT_LINKEDOBJECT) {
        uint64_t V;
        ((CommandResult_LinkedObject *)CE)->Value(V);
        char l[50];
        sprintf( &l[0], "+0x%llx", V);
        std::string l_name;
        ((CommandResult_LinkedObject *)CE)->Value(l_name);
        Name = Name + " @ " + l_name + l + " in ";
      } else if (CE->Type() == CMD_RESULT_UINT) {
        Name += " @ ";
      }
    }
   // Add function name and location information.
    Name += CE->Form();
    return Name;
  }
  virtual PyObject * pyValue () {
    std::string F = Form ();
    return Py_BuildValue("s",F.c_str());
  }
  virtual void Print (ostream &to, int64_t fieldsize, bool leftjustified) {
    std::string string_value = Form ();
    if (leftjustified) {
     // Left justification is only done on the last column of a report.
     // Don't truncate the string if it is bigger than the field size.
     // This is done to make sure everything gets printed.

      to << std::setiosflags(std::ios::left) << string_value;

     // If there is unused space in the field, pad with blanks.
      if ((string_value.length() < fieldsize) &&
          (string_value[string_value.length()-1] != *("\n"))) {
        for (int64_t i = string_value.length(); i < fieldsize; i++) to << " ";      }

    } else {
     // Right justify the string in the field.
     // Don't let it exceed the size of the field.
     // Also, limit the size based on our internal buffer size.
      to << std::setiosflags(std::ios::right) << std::setw(fieldsize)
         << ((string_value.length() <= fieldsize) ? string_value : string_value.substr(0, fieldsize));
    }
  }
};

// A Time is nano-second time that is displayed in date-time format.
class CommandResult_Time : public CommandResult {
 private:
  Time time_value;

 public:
  CommandResult_Time () : CommandResult(CMD_RESULT_TIME) {
  }
  CommandResult_Time (Time t)
      : CommandResult(CMD_RESULT_TIME) {
    time_value = t;
  }
  CommandResult_Time (CommandResult_Time *T)
      : CommandResult(CMD_RESULT_TIME) {
    time_value = T->time_value;
  }
  virtual ~CommandResult_Time () {
  }

  void Min_Time (CommandResult_Time *B) {
    time_value = min (time_value, B->time_value);
  }
  void Max_Time (CommandResult_Time *B) {
    time_value = max (time_value, B->time_value);
  }
  Time& Value () {
    return time_value;
  };
  void Value (Time& t) {
    t = time_value;
  };

  virtual std::string Form () {
    std::ostringstream form(ios::out);
    form << time_value;
    return form.ostringstream::str();
  }
  virtual PyObject * pyValue () {
    std::string F = Form ();
    return Py_BuildValue("s",F.c_str());
  }
  virtual void Print (ostream &to, int64_t fieldsize, bool leftjustified) {
    std::string string_value = Form ();
    if (leftjustified) {
     // Left justification is only done on the last column of a report.
     // Don't truncate the string if it is bigger than the field size.
     // This is done to make sure everything gets printed.

      to << std::setiosflags(std::ios::left) << string_value;

     // If there is unused space in the field, pad with blanks.
      if ((string_value.length() < fieldsize) &&
          (string_value[string_value.length()-1] != *("\n"))) {
        for (int64_t i = string_value.length(); i < fieldsize; i++) to << " ";      }

    } else {
     // Right justify the string in the field.
     // Don't let it exceed the size of the field.
     // Also, limit the size based on our internal buffer size.
      to << std::setiosflags(std::ios::right) << std::setw(fieldsize)
         << ((string_value.length() <= fieldsize) ? string_value : string_value.substr(0, fieldsize));
    }
  }
};

// A Duration is nano-second time that is displayed as days:hours:minutes:seconds.
class CommandResult_Duration : public CommandResult {
 private:
  int64_t duration_value;

 public:
  CommandResult_Duration () : CommandResult(CMD_RESULT_DURATION) {
  }
  CommandResult_Duration (int64_t d)
      : CommandResult(CMD_RESULT_DURATION) {
    duration_value = d;
  }
  CommandResult_Duration (CommandResult_Duration *D)
      : CommandResult(CMD_RESULT_DURATION) {
    duration_value = D->duration_value;
  }
  virtual ~CommandResult_Duration () {
  }

  void Min_Duration (CommandResult_Duration *B) {
    duration_value = min (duration_value, B->duration_value);
  }
  void Max_Duration (CommandResult_Duration *B) {
    duration_value = max (duration_value, B->duration_value);
  }
  void Accumulate_Duration (CommandResult_Duration *B) {
    duration_value += B->duration_value;
  }
  int64_t& Value () {
    return duration_value;
  };
  void Value (int64_t& d) {
    d = duration_value;
  };

  virtual std::string Form () {
    std::ostringstream form(ios::ate);
    int64_t time_scale = 1000000000;
    int64_t remaining = duration_value;
    int64_t next_threshold = (time_scale * 60 * 60 * 24);
    bool output_started = false;
    if (remaining >= next_threshold) {
     // Output days
      form << (remaining / next_threshold) << ":";
      remaining = (remaining % next_threshold);
      output_started = true;
    }
    next_threshold = (time_scale * 60 * 60);
    if (remaining >= next_threshold) {
     // Output hours
      form << (remaining / next_threshold) << ":";
      remaining = (remaining % next_threshold);
      output_started = true;
    }
    next_threshold = (time_scale * 60);
    if (output_started ||
        (remaining >= next_threshold)) {
     // output minutes
      form << (remaining / next_threshold) << ":";
      remaining = (remaining % next_threshold);
      output_started = true;
    }
    if (output_started ||
        (remaining >= time_scale)) {
     // output seconds
      form << (remaining / time_scale) << ".";
      remaining = (remaining % time_scale);
      output_started = true;
    }
    if (!output_started) {
      form << "0.";
    }
    if (remaining == 0) {
      form << "0";
    } else if (!output_started &&
               (remaining < 1000000)) {
     // number is less than 1 milli-second
     // ouput to nano-second precision
      form << remaining;
    } else {
     // output normal precision
      std::ostringstream s(ios::ate);
      s << remaining;
      if (s.ostringstream::str().size() < OPENSS_VIEW_PRECISION) {
        form << s.ostringstream::str();
      } else {
        form << s.ostringstream::str().substr(0,OPENSS_VIEW_PRECISION);
      }
    }
    return form.ostringstream::str();
  }
  virtual PyObject * pyValue () {
    std::string F = Form ();
    return Py_BuildValue("s",F.c_str());
  }
  virtual void Print (ostream &to, int64_t fieldsize, bool leftjustified) {
    std::string string_value = Form ();
    if (leftjustified) {
     // Left justification is only done on the last column of a report.
     // Don't truncate the string if it is bigger than the field size.
     // This is done to make sure everything gets printed.

      to << std::setiosflags(std::ios::left) << string_value;

     // If there is unused space in the field, pad with blanks.
      if ((string_value.length() < fieldsize) &&
          (string_value[string_value.length()-1] != *("\n"))) {
        for (int64_t i = string_value.length(); i < fieldsize; i++) to << " ";      }

    } else {
     // Right justify the string in the field.
     // Don't let it exceed the size of the field.
     // Also, limit the size based on our internal buffer size.
      to << std::setiosflags(std::ios::right) << std::setw(fieldsize)
         << ((string_value.length() <= fieldsize) ? string_value : string_value.substr(0, fieldsize));
    }
  }
};

// An Interval is a double precision value of seconds that is displayed in milli-seconds.
class CommandResult_Interval : public CommandResult {
 private:
  double interval_value;

 public:
  CommandResult_Interval () : CommandResult(CMD_RESULT_INTERVAL) {
  }
  CommandResult_Interval (double d)
      : CommandResult(CMD_RESULT_INTERVAL) {
    interval_value = d;
  }
  CommandResult_Interval (CommandResult_Interval *D)
      : CommandResult(CMD_RESULT_INTERVAL) {
    interval_value = D->interval_value;
  }
  virtual ~CommandResult_Interval () {
  }

  void Min_Interval (CommandResult_Interval *B) {
    interval_value = min (interval_value, B->interval_value);
  }
  void Max_Interval (CommandResult_Interval *B) {
    interval_value = max (interval_value, B->interval_value);
  }
  void Accumulate_Interval (CommandResult_Interval *B) {
    interval_value += B->interval_value;
  }
  double& Value () {
    return interval_value;
  };
  void Value (double& d) {
    d = interval_value;
  };

  virtual std::string Form () {
    std::ostringstream form(ios::ate);
    double float_value = interval_value * 1000; // Convert to milli-seconds

    char F[20];
    F[0] = *("%"); // left justify in field
    sprintf(&F[1], "%lld.%lldf\0", OPENSS_VIEW_FIELD_SIZE, OPENSS_VIEW_PRECISION);

    char s[OPENSS_VIEW_FIELD_SIZE];
    sprintf ( s, &F[0], float_value);
    return std::string (s);
  }
  virtual PyObject * pyValue () {
    std::string F = Form ();
    return Py_BuildValue("s",F.c_str());
  }
  virtual void Print (ostream &to, int64_t fieldsize, bool leftjustified) {
    std::string string_value = Form ();
    if (leftjustified) {
     // Left justification is only done on the last column of a report.
     // Don't truncate the string if it is bigger than the field size.
     // This is done to make sure everything gets printed.

      to << std::setiosflags(std::ios::left) << string_value;

     // If there is unused space in the field, pad with blanks.
      if ((string_value.length() < fieldsize) &&
          (string_value[string_value.length()-1] != *("\n"))) {
        for (int64_t i = string_value.length(); i < fieldsize; i++) to << " ";      }

    } else {
     // Right justify the string in the field.
     // Don't let it exceed the size of the field.
     // Also, limit the size based on our internal buffer size.
      to << std::setiosflags(std::ios::right) << std::setw(fieldsize)
         << ((string_value.length() <= fieldsize) ? string_value : string_value.substr(0, fieldsize));
    }
  }
};

class CommandResult_Title :
     public CommandResult {
  std::string string_value;

 public:
  CommandResult_Title (std::string S) : CommandResult(CMD_RESULT_TITLE) {
    string_value = S;
  }
  virtual ~CommandResult_Title () { }

  void Value (std::string& S) {
    S = string_value;
  }

  virtual std::string Form () {
    return string_value;
  }
  virtual PyObject * pyValue () {
    return Py_BuildValue("s",string_value.c_str());
  }
  virtual void Print (ostream& to, int64_t fieldsize, bool leftjustified) {
    to << string_value;
  }
};

class CommandResult_Headers :
     public CommandResult {

 int64_t number_of_columns;
 std::list<CommandResult *> Headers;

 public:
  CommandResult_Headers () : CommandResult(CMD_RESULT_COLUMN_HEADER) {
    number_of_columns = 0;
  }
  virtual ~CommandResult_Headers () {
    Reclaim_CR_Space (Headers);
  }

  void Add_Header (CommandResult *R) {
    number_of_columns++;
    Headers.push_back(R);
  }
  void Value (std::list<CommandResult *>& R) {
    R = Headers;
  }
  void Value (int64_t& NC) {
    NC = number_of_columns;
  }

  virtual std::string Form () {
    std::string S;
    std::list<CommandResult *>::iterator cri = Headers.begin();
    int64_t num_results = 0;
    for (cri = Headers.begin(); cri != Headers.end(); cri++) {
      if (num_results++ != 0) S += "  "; // 2 spaces between strings
      std::string R = (*cri)->Form ();
      if (num_results < number_of_columns) {
       // Except for the last column ...
        if (R.size () > OPENSS_VIEW_FIELD_SIZE) {
         // Shorten the original string.
          R.resize (OPENSS_VIEW_FIELD_SIZE);
        } else if (R.size () < OPENSS_VIEW_FIELD_SIZE) {
         // Lengthen the original string.
          std::string T;
          T.resize ((OPENSS_VIEW_FIELD_SIZE - R.size ()), *" ");
          R = T + R;
        }
      }
      S += R;
    }
    return S;
  }
  virtual PyObject * pyValue () {
    PyObject *p_object = PyList_New(0);
    std::list<CommandResult *>::iterator cri = Headers.begin();
    for (cri = Headers.begin(); cri != Headers.end(); cri++) {
      PyObject *p_item = (*cri)->pyValue ();
      PyList_Append(p_object,p_item);
    }
    return p_object;
  }
  virtual void Print (ostream& to, int64_t fieldsize, bool leftjustified) {
    std::list<CommandResult *>::iterator coi;
    int64_t num_results = 0;
    for (coi = Headers.begin(); coi != Headers.end(); coi++) {
      if (num_results++ != 0) to << "  ";
      (*coi)->Print (to, fieldsize, (num_results >= number_of_columns) ? true : false);
    }

  }
};

class CommandResult_Enders :
     public CommandResult {

 int64_t number_of_columns;
 std::list<CommandResult *> Enders;

 public:
  CommandResult_Enders () : CommandResult(CMD_RESULT_COLUMN_ENDER) {
    number_of_columns = 0;
  }
  virtual ~CommandResult_Enders () {
    Reclaim_CR_Space (Enders);
  }

  void Add_Ender (CommandResult *R) {
    number_of_columns++;
    Enders.push_back(R);
  }
  void Value (std::list<CommandResult *>& R) {
    R = Enders;
  }

  virtual std::string Form () {
    std::string S;
    std::list<CommandResult *>::iterator cri = Enders.begin();
    int64_t num_results = 0;
    for (cri = Enders.begin(); cri != Enders.end(); cri++) {
      if (num_results++ != 0) S += "  "; // 2 spaces between strings
      std::string R = (*cri)->Form ();
      if (num_results < number_of_columns) {
       // Except for the last column ...
        if (R.size () > OPENSS_VIEW_FIELD_SIZE) {
         // shorten the original string.
          R.resize (OPENSS_VIEW_FIELD_SIZE);
        } else if (R.size () < OPENSS_VIEW_FIELD_SIZE) {
         // lengthen the original string.
          std::string T;
          T.resize ((OPENSS_VIEW_FIELD_SIZE - R.size ()), *" ");
          R = T + R;
        }
      }
      S += R;
    }
    return S;
  }
  virtual PyObject * pyValue () {
    PyObject *p_object = PyList_New(0);
    std::list<CommandResult *>::iterator cri = Enders.begin();
    for (cri = Enders.begin(); cri != Enders.end(); cri++) {
      PyObject *p_item = (*cri)->pyValue ();
      PyList_Append(p_object,p_item);
    }
    return p_object;
  }
  virtual void Print (ostream& to, int64_t fieldsize, bool leftjustified) {
    std::list<CommandResult *>::iterator coi;
    int64_t num_results = 0;
    for (coi = Enders.begin(); coi != Enders.end(); coi++) {
      if (num_results++ != 0) to << "  ";
      (*coi)->Print (to, fieldsize, (num_results >= number_of_columns) ? true : false);
    }

  }
};

class CommandResult_Columns :
     public CommandResult {

 int64_t number_of_columns;
 std::list<CommandResult *> Columns;

 public:
  CommandResult_Columns (int64_t C = 0) : CommandResult(CMD_RESULT_COLUMN_VALUES) {
    number_of_columns = 0;
  }
  virtual ~CommandResult_Columns () {
    Reclaim_CR_Space (Columns);
  }

  void Add_Column (CommandResult *R) {
    number_of_columns++;
    Columns.push_back(R);
  }
  void Value (std::list<CommandResult *>& R) {
    R = Columns;
  }

  virtual std::string Form () {
    std::string S;
    std::list<CommandResult *>::iterator cri = Columns.begin();
    int64_t num_results = 0;
    for (cri = Columns.begin(); cri != Columns.end(); cri++) {
      if (num_results++ != 0) S += "  "; // 2 spaces between strings
      std::string R = (*cri)->Form ();
      if (num_results < number_of_columns) {
       // Except for the last column ...
        if (R.size () > OPENSS_VIEW_FIELD_SIZE) {
         // Shorten the original string.
          R.resize (OPENSS_VIEW_FIELD_SIZE);
        } else if (R.size () < OPENSS_VIEW_FIELD_SIZE) {
         // Lengthen the original string.
          std::string T;
          T.resize ((OPENSS_VIEW_FIELD_SIZE - R.size ()), *" ");
          R = T + R;
        }
      }
      S += R;
    }
    return S;
  }
  virtual PyObject * pyValue () {
    PyObject *p_object = PyList_New(0);
    std::list<CommandResult *>::iterator cri = Columns.begin();
    for (cri = Columns.begin(); cri != Columns.end(); cri++) {
      PyObject *p_item = (*cri)->pyValue ();
      PyList_Append(p_object,p_item);
    }
    return p_object;
  }
  virtual void Print (ostream& to, int64_t fieldsize, bool leftjustified) {
    std::list<CommandResult *>::iterator coi;
    int64_t num_results = 0;
    for (coi = Columns.begin(); coi != Columns.end(); coi++) {
      if (num_results++ != 0) to << "  ";
      if ((*coi)->isNullValue()) {
       // Avoid print lots of meaningless "0" values - blank fill the field.
        to << (leftjustified ? std::setiosflags(std::ios::left) : std::setiosflags(std::ios::right))
           << std::setw(fieldsize) << " ";
      } else {
        (*coi)->Print (to, fieldsize, (num_results >= number_of_columns) ? true : false);
      }
    }

  }
};

inline CommandResult *Dup_CommandResult (CommandResult *C) {
  if (C == NULL) return NULL;
  switch (C->Type()) {
   case CMD_RESULT_UINT:
     return new CommandResult_Uint((CommandResult_Uint *)C);
   case CMD_RESULT_INT:
     return new CommandResult_Int((CommandResult_Int *)C);
   case CMD_RESULT_FLOAT:
     return new CommandResult_Float((CommandResult_Float *)C);
   case CMD_RESULT_STRING:
     return new CommandResult_String((CommandResult_String *)C);
   case CMD_RESULT_RAWSTRING:
     return new CommandResult_RawString((CommandResult_RawString *)C);
   case CMD_RESULT_FUNCTION:
     return new CommandResult_Function((CommandResult_Function *)C);
   case CMD_RESULT_STATEMENT:
     return new CommandResult_Statement((CommandResult_Statement *)C);
   case CMD_RESULT_LINKEDOBJECT:
     return new CommandResult_LinkedObject((CommandResult_LinkedObject *)C);
   case CMD_RESULT_CALLTRACE:
     return new CommandResult_CallStackEntry((CommandResult_CallStackEntry *)C);
   case CMD_RESULT_TIME:
     return new CommandResult_Time((CommandResult_Time *)C);
   case CMD_RESULT_DURATION:
     return new CommandResult_Duration((CommandResult_Duration *)C);
   case CMD_RESULT_INTERVAL:
     return new CommandResult_Interval((CommandResult_Interval *)C);
   default:
    Assert (C->Type() == CMD_RESULT_NULL);
  }
  return NULL;
}

// Create a new CommandResult object, with an initial default value,
// that has the same type as another CommandResult object.
inline CommandResult *New_CommandResult (CommandResult *C) {
  CommandResult *v = NULL;
  switch (C->Type()) {
   case CMD_RESULT_UINT:
    v = new CommandResult_Uint ();
    break;
   case CMD_RESULT_INT:
    v = new CommandResult_Int ();
    break;
   case CMD_RESULT_FLOAT:
    v = new CommandResult_Float ();
    break;
   case CMD_RESULT_STRING:
     v = new CommandResult_String ("");
     break;
   case CMD_RESULT_RAWSTRING:
     v = new CommandResult_RawString ("");
     break;
   case CMD_RESULT_TIME:
     v = new CommandResult_Time ();
     break;
   case CMD_RESULT_DURATION:
     v = new CommandResult_Duration ();
     break;
   case CMD_RESULT_INTERVAL:
     v = new CommandResult_Interval ();
     break;
   default:
    Assert (C->Type() == CMD_RESULT_NULL);
  }
  return v;
}

inline bool CommandResult_lt (CommandResult *lhs, CommandResult *rhs) {
  if ((lhs->Type() == CMD_RESULT_CALLTRACE) &&
      (rhs->Type() != CMD_RESULT_CALLTRACE)) {
    std::vector<CommandResult *> *ls;
    ls = ((CommandResult_CallStackEntry *)lhs)->Value ();
    if(ls->size() == 1)
      return CommandResult_lt((*ls)[0], rhs);
  }
  if ((lhs->Type() != CMD_RESULT_CALLTRACE) &&
      (rhs->Type() == CMD_RESULT_CALLTRACE)) {
    std::vector<CommandResult *> *rs;
    rs = ((CommandResult_CallStackEntry *)rhs)->Value ();
    if(rs->size() == 1)
      return CommandResult_lt(lhs, (*rs)[0]);
  }
  if (lhs->Type() != rhs->Type()) {
    return (lhs->Type() < rhs->Type());
  }
  switch (lhs->Type()) {
   case CMD_RESULT_UINT:
    uint64_t Uvalue1, Uvalue2;
    ((CommandResult_Uint *)lhs)->Value(Uvalue1);
    ((CommandResult_Uint *)rhs)->Value(Uvalue2);
    return Uvalue1 < Uvalue2;
   case CMD_RESULT_INT:
    int64_t Ivalue1, Ivalue2;
    ((CommandResult_Int *)lhs)->Value(Ivalue1);
    ((CommandResult_Int *)rhs)->Value(Ivalue2);
    return Ivalue1 < Ivalue2;
   case CMD_RESULT_FLOAT:
    double Fvalue1, Fvalue2;
    ((CommandResult_Float *)lhs)->Value(Fvalue1);
    ((CommandResult_Float *)rhs)->Value(Fvalue2);
    return Fvalue1 < Fvalue2;
   case CMD_RESULT_LINKEDOBJECT:
    return OpenSpeedShop::Queries::CompareLinkedObjects()(
	*((CommandResult_LinkedObject *)lhs),
	*((CommandResult_LinkedObject *)rhs)
	);
   case CMD_RESULT_FUNCTION:
    return OpenSpeedShop::Queries::CompareFunctions()(
	*((CommandResult_Function *)lhs),
	*((CommandResult_Function *)rhs)
	);
   case CMD_RESULT_STATEMENT:
    return OpenSpeedShop::Queries::CompareStatements()(
	*((CommandResult_Statement *)lhs),
	*((CommandResult_Statement *)rhs)
	);
   case CMD_RESULT_CALLTRACE:
   {
    std::vector<CommandResult *> *ls;
    std::vector<CommandResult *> *rs;
    ls = ((CommandResult_CallStackEntry *)lhs)->Value ();
    rs = ((CommandResult_CallStackEntry *)rhs)->Value ();
    int64_t ll = ls->size();
    int64_t rl = rs->size();
    int64_t lm = min(ll,rl);
    for (int64_t i = 0; i < lm; i++) {
      if (CommandResult_lt ((*ls)[i], (*rs)[i])) {
        return true;
      }
    }
    return (ll < rl);
   }
   case CMD_RESULT_STRING:
   {
     std::string s1, s2;
     ((CommandResult_String *)lhs)->Value(s1);
     ((CommandResult_String *)rhs)->Value(s2);
     return (s1 < s2);
   }
   case CMD_RESULT_RAWSTRING:
   {
     std::string r1, r2;
     ((CommandResult_RawString *)lhs)->Value(r1);
     ((CommandResult_RawString *)rhs)->Value(r2);
     return (r1 < r2);
   }
   case CMD_RESULT_TIME:
   {
    Time Tvalue1, Tvalue2;
    ((CommandResult_Time *)lhs)->Value(Tvalue1);
    ((CommandResult_Time *)rhs)->Value(Tvalue2);
    return Tvalue1 < Tvalue2;
   }
   case CMD_RESULT_DURATION:
   {
    int64_t Dvalue1, Dvalue2;
    ((CommandResult_Duration *)lhs)->Value(Dvalue1);
    ((CommandResult_Duration *)rhs)->Value(Dvalue2);
    return Dvalue1 < Dvalue2;
   }
   case CMD_RESULT_INTERVAL:
   {
    double Dvalue1, Dvalue2;
    ((CommandResult_Interval *)lhs)->Value(Dvalue1);
    ((CommandResult_Interval *)rhs)->Value(Dvalue2);
    return Dvalue1 < Dvalue2;
   }
   default:
    Assert (lhs->Type() == CMD_RESULT_NULL);
  }
  return false;
}

inline bool CommandResult_gt (CommandResult *lhs, CommandResult *rhs) {
Assert (lhs != NULL);
Assert (rhs != NULL);
  if ((lhs->Type() == CMD_RESULT_CALLTRACE) &&
      (rhs->Type() != CMD_RESULT_CALLTRACE)) {
    std::vector<CommandResult *> *ls;
    ls = ((CommandResult_CallStackEntry *)lhs)->Value ();
    if (ls->size() == 1)
      return CommandResult_gt((*ls)[0], rhs);
  }
  if ((lhs->Type() != CMD_RESULT_CALLTRACE) &&
      (rhs->Type() == CMD_RESULT_CALLTRACE)) {
    std::vector<CommandResult *> *rs;
    rs = ((CommandResult_CallStackEntry *)rhs)->Value ();
    if (rs->size() == 1)
      return CommandResult_gt(lhs, (*rs)[0]);
  }
  if (lhs->Type() != rhs->Type()) {
    return (lhs->Type() > rhs->Type());
  }
  switch (lhs->Type()) {
   case CMD_RESULT_UINT:
    uint64_t Uvalue1, Uvalue2;
    ((CommandResult_Uint *)lhs)->Value(Uvalue1);
    ((CommandResult_Uint *)rhs)->Value(Uvalue2);
    return Uvalue1 > Uvalue2;
   case CMD_RESULT_INT:
    int64_t Ivalue1, Ivalue2;
    ((CommandResult_Int *)lhs)->Value(Ivalue1);
    ((CommandResult_Int *)rhs)->Value(Ivalue2);
    return Ivalue1 > Ivalue2;
   case CMD_RESULT_FLOAT:
    double Fvalue1, Fvalue2;
    ((CommandResult_Float *)lhs)->Value(Fvalue1);
    ((CommandResult_Float *)rhs)->Value(Fvalue2);
    return Fvalue1 > Fvalue2;
   case CMD_RESULT_LINKEDOBJECT:
    return OpenSpeedShop::Queries::CompareLinkedObjects()(*((CommandResult_LinkedObject *)rhs),
                                                          *((CommandResult_LinkedObject *)lhs));
   case CMD_RESULT_FUNCTION:
    if (OpenSpeedShop::Queries::CompareFunctions()(*((CommandResult_Function *)rhs),
                                                   *((CommandResult_Function *)lhs))) {
      return true;
    }
    if (OpenSpeedShop::Queries::CompareFunctions()(*((CommandResult_Function *)lhs),
                                                   *((CommandResult_Function *)rhs))) {
      return false;
    } else {
      std::set<Statement> L;
      std::set<Statement> R;
      ((CommandResult_Function *)lhs)->Value(L);
      ((CommandResult_Function *)rhs)->Value(R);
      int64_t Ls = 0;
      int64_t Rs = 0;
      if (L.begin() != L.end()) {
        Ls = (*L.begin()).getLine();
      }
      if (R.begin() != R.end()) {
        Rs = (*R.begin()).getLine();
      }
      return (Ls > Rs);
    }
   case CMD_RESULT_STATEMENT:
    return OpenSpeedShop::Queries::CompareStatements()(*((CommandResult_Statement *)rhs),
                                                       *((CommandResult_Statement *)lhs));
   case CMD_RESULT_CALLTRACE:
   {
    std::vector<CommandResult *> *ls;
    std::vector<CommandResult *> *rs;
    ls = ((CommandResult_CallStackEntry *)lhs)->Value ();
    rs = ((CommandResult_CallStackEntry *)rhs)->Value ();
    int64_t ll = ls->size();
    int64_t rl = rs->size();
    int64_t lm = min(ll,rl);
    for (int64_t i = 0; i < lm; i++) {
      if (CommandResult_gt ((*ls)[i], (*rs)[i])) {
        return true;
      }
    }
    return (ll < rl);
   }
   case CMD_RESULT_STRING:
   {
     std::string s1, s2;
     ((CommandResult_String *)lhs)->Value(s1);
     ((CommandResult_String *)rhs)->Value(s2);
     return (s1 > s2);
   }
   case CMD_RESULT_RAWSTRING:
   {
     std::string r1, r2;
     ((CommandResult_RawString *)lhs)->Value(r1);
     ((CommandResult_RawString *)rhs)->Value(r2);
     return (r1 > r2);
   }
   case CMD_RESULT_TIME:
   {
    Time Tvalue1, Tvalue2;
    ((CommandResult_Time *)lhs)->Value(Tvalue1);
    ((CommandResult_Time *)rhs)->Value(Tvalue2);
    return Tvalue1 > Tvalue2;
   }
   case CMD_RESULT_DURATION:
   {
    int64_t Dvalue1, Dvalue2;
    ((CommandResult_Duration *)lhs)->Value(Dvalue1);
    ((CommandResult_Duration *)rhs)->Value(Dvalue2);
    return Dvalue1 > Dvalue2;
   }
   case CMD_RESULT_INTERVAL:
   {
    double Dvalue1, Dvalue2;
    ((CommandResult_Interval *)lhs)->Value(Dvalue1);
    ((CommandResult_Interval *)rhs)->Value(Dvalue2);
    return Dvalue1 > Dvalue2;
   }
   default:
    Assert (lhs->Type() == CMD_RESULT_NULL);
  }
  return false;
}

inline void Accumulate_CommandResult (CommandResult *A, CommandResult *B) {
  if (!A || !B) return;
  Assert (A->Type() == B->Type());
  switch (A->Type()) {
  case CMD_RESULT_UINT:
    ((CommandResult_Uint *)A)->Accumulate_Uint ((CommandResult_Uint *)B);
    break;
  case CMD_RESULT_INT:
    ((CommandResult_Int *)A)->Accumulate_Int ((CommandResult_Int *)B);
    break;
  case CMD_RESULT_FLOAT:
    ((CommandResult_Float *)A)->Accumulate_Float ((CommandResult_Float *)B);
    break;
  case CMD_RESULT_STRING:
    ((CommandResult_String *)A)->Accumulate_String ((CommandResult_String *)B);
    break;
  case CMD_RESULT_RAWSTRING:
    ((CommandResult_RawString *)A)->Accumulate_RawString ((CommandResult_RawString *)B);
    break;
  case CMD_RESULT_DURATION:
    ((CommandResult_Duration *)A)->Accumulate_Duration ((CommandResult_Duration *)B);
    break;
  case CMD_RESULT_INTERVAL:
    ((CommandResult_Interval *)A)->Accumulate_Interval ((CommandResult_Interval *)B);
    break;
  default:
    Assert (A->Type() == CMD_RESULT_NULL);
  }
}

inline void Accumulate_Min_CommandResult (CommandResult *A, CommandResult *B) {
  if (!A || !B) return;
  Assert (A->Type() == B->Type());
  switch (A->Type()) {
  case CMD_RESULT_UINT:
    ((CommandResult_Uint *)A)->Min_Uint ((CommandResult_Uint *)B);
    break;
  case CMD_RESULT_INT:
    ((CommandResult_Int *)A)->Min_Int ((CommandResult_Int *)B);
    break;
  case CMD_RESULT_FLOAT:
    ((CommandResult_Float *)A)->Min_Float ((CommandResult_Float *)B);
    break;
  case CMD_RESULT_TIME:
    ((CommandResult_Time *)A)->Min_Time ((CommandResult_Time *)B);
    break;
  case CMD_RESULT_DURATION:
    ((CommandResult_Duration *)A)->Min_Duration ((CommandResult_Duration *)B);
    break;
  case CMD_RESULT_INTERVAL:
    ((CommandResult_Interval *)A)->Min_Interval ((CommandResult_Interval *)B);
    break;
  }
}

inline void Accumulate_Max_CommandResult (CommandResult *A, CommandResult *B) {
  if (!A || !B) return;
  Assert (A->Type() == B->Type());
  switch (A->Type()) {
  case CMD_RESULT_UINT:
    ((CommandResult_Uint *)A)->Max_Uint ((CommandResult_Uint *)B);
    break;
  case CMD_RESULT_INT:
    ((CommandResult_Int *)A)->Max_Int ((CommandResult_Int *)B);
    break;
  case CMD_RESULT_FLOAT:
    ((CommandResult_Float *)A)->Max_Float ((CommandResult_Float *)B);
    break;
  case CMD_RESULT_TIME:
    ((CommandResult_Time *)A)->Max_Time ((CommandResult_Time *)B);
    break;
  case CMD_RESULT_DURATION:
    ((CommandResult_Duration *)A)->Max_Duration ((CommandResult_Duration *)B);
    break;
  case CMD_RESULT_INTERVAL:
    ((CommandResult_Interval *)A)->Max_Interval ((CommandResult_Interval *)B);
    break;
  }
}


inline CommandResult *Calculate_Average (CommandResult *A, CommandResult *B) {

  if (B == NULL) {
    return NULL;
  }
  if (A == NULL) {
    return new CommandResult_Float (0.0);
  }

  double Avalue;
  double Bvalue;
  int64_t Ivalue;

  switch (A->Type()) {
   case CMD_RESULT_UINT:
    uint64_t Uvalue;
    ((CommandResult_Uint *)A)->Value(Uvalue);
    Avalue = Uvalue;
    break;
   case CMD_RESULT_INT:
    ((CommandResult_Int *)A)->Value(Ivalue);
    Avalue = Ivalue;
    break;
   case CMD_RESULT_FLOAT:
    ((CommandResult_Float *)A)->Value(Avalue);
    break;
   case CMD_RESULT_DURATION:
    ((CommandResult_Duration *)A)->Value(Ivalue);
    Avalue = Ivalue;
    break;
   case CMD_RESULT_INTERVAL:
    ((CommandResult_Interval *)A)->Value(Avalue);
    break;
  }
  switch (B->Type()) {
   case CMD_RESULT_UINT:
    uint64_t Uvalue;
    ((CommandResult_Uint *)B)->Value(Uvalue);
    Bvalue = Uvalue;
    break;
   case CMD_RESULT_INT:
    ((CommandResult_Int *)B)->Value(Ivalue);
    Bvalue = Ivalue;
    break;
   case CMD_RESULT_FLOAT:
    ((CommandResult_Float *)B)->Value(Bvalue);
    break;
   case CMD_RESULT_DURATION:
    ((CommandResult_Duration *)B)->Value(Ivalue);
    Bvalue = Ivalue;
    break;
   case CMD_RESULT_INTERVAL:
    ((CommandResult_Interval *)B)->Value(Bvalue);
    break;
  }

  double average = Avalue / Bvalue;
  return new CommandResult_Float (average);
}

inline CommandResult *Calculate_StdDev  (CommandResult *A,
                                         CommandResult *B,
                                         CommandResult *C) {

  if ((A == NULL) ||
      (B == NULL) ||
      (C == NULL)) {
    return NULL;
  }

  double Avalue;
  double Bvalue;
  double Cvalue;
  int64_t Ivalue;

  switch (A->Type()) {
   case CMD_RESULT_UINT:
    uint64_t Uvalue;
    ((CommandResult_Uint *)A)->Value(Uvalue);
    Avalue = Uvalue;
    break;
   case CMD_RESULT_INT:
    ((CommandResult_Int *)A)->Value(Ivalue);
    Avalue = Ivalue;
    break;
   case CMD_RESULT_FLOAT:
    ((CommandResult_Float *)A)->Value(Avalue);
    break;
  case CMD_RESULT_DURATION:
    ((CommandResult_Duration *)A)->Value(Ivalue);
    Avalue = Ivalue;
    break;
  case CMD_RESULT_INTERVAL:
    ((CommandResult_Interval *)A)->Value(Avalue);
    break;
  }
  switch (B->Type()) {
   case CMD_RESULT_UINT:
    uint64_t Uvalue;
    ((CommandResult_Uint *)B)->Value(Uvalue);
    Bvalue = Uvalue;
    break;
   case CMD_RESULT_INT:
    ((CommandResult_Int *)B)->Value(Ivalue);
    Bvalue = Ivalue;
    break;
   case CMD_RESULT_FLOAT:
    ((CommandResult_Float *)B)->Value(Bvalue);
    break;
  case CMD_RESULT_DURATION:
    ((CommandResult_Duration *)B)->Value(Ivalue);
    Bvalue = Ivalue;
    break;
  case CMD_RESULT_INTERVAL:
    ((CommandResult_Interval *)B)->Value(Bvalue);
    break;
  }
  switch (C->Type()) {
   case CMD_RESULT_UINT:
    uint64_t Uvalue;
    ((CommandResult_Uint *)C)->Value(Uvalue);
    Cvalue = Uvalue;
    break;
   case CMD_RESULT_INT:
    ((CommandResult_Int *)C)->Value(Ivalue);
    Cvalue = Ivalue;
    break;
   case CMD_RESULT_FLOAT:
    ((CommandResult_Float *)C)->Value(Cvalue);
    break;
  case CMD_RESULT_DURATION:
    ((CommandResult_Duration *)C)->Value(Ivalue);
    Cvalue = Ivalue;
    break;
  case CMD_RESULT_INTERVAL:
    ((CommandResult_Interval *)C)->Value(Cvalue);
    break;
  }
  if (Cvalue <= 1.0) {
    return NULL;
  }

  double result = sqrt (((Cvalue * Bvalue) - (Avalue * Avalue)) / (Cvalue * (Cvalue -1.0)));
  return isnan(result) ? NULL : new CommandResult_Float (result);
}

inline CommandResult *Calculate_Percent (CommandResult *A, CommandResult *B) {

  if (B == NULL) {
    return NULL;
  }
  if (A == NULL) {
    return new CommandResult_Float (0.0);
  }

  double Avalue;
  double Bvalue;
  int64_t Ivalue;

  switch (A->Type()) {
   case CMD_RESULT_UINT:
    uint64_t Uvalue;
    ((CommandResult_Uint *)A)->Value(Uvalue);
    Avalue = Uvalue;
    break;
   case CMD_RESULT_INT:
    ((CommandResult_Int *)A)->Value(Ivalue);
    Avalue = Ivalue;
    break;
   case CMD_RESULT_FLOAT:
    ((CommandResult_Float *)A)->Value(Avalue);
    break;
  case CMD_RESULT_DURATION:
    ((CommandResult_Duration *)A)->Value(Ivalue);
    Avalue = Ivalue;
    break;
  case CMD_RESULT_INTERVAL:
    ((CommandResult_Interval *)A)->Value(Avalue);
    break;
  }
  switch (B->Type()) {
   case CMD_RESULT_UINT:
    uint64_t Uvalue;
    ((CommandResult_Uint *)B)->Value(Uvalue);
    Bvalue = Uvalue;
    break;
   case CMD_RESULT_INT:
    ((CommandResult_Int *)B)->Value(Ivalue);
    Bvalue = Ivalue;
    break;
   case CMD_RESULT_FLOAT:
    ((CommandResult_Float *)B)->Value(Bvalue);
    break;
  case CMD_RESULT_DURATION:
    ((CommandResult_Duration *)B)->Value(Ivalue);
    Bvalue = Ivalue;
    break;
  case CMD_RESULT_INTERVAL:
    ((CommandResult_Interval *)B)->Value(Bvalue);
    break;
  }

  double percent = 0.0;
  if (Bvalue > 0.0) {
    percent = (Avalue *100) / Bvalue;
    // if (percent > 100.0) percent = 100.0;
    if (percent < 0.0) percent = 0.0;
  }
  return new CommandResult_Float (percent);
}

// Overloaded utility that will generate the right CommandResult object.
inline CommandResult *CRPTR (uint V) { return new CommandResult_Uint (V); }
inline CommandResult *CRPTR (uint64_t V) { return new CommandResult_Uint (V); }
inline CommandResult *CRPTR (int V) { return new CommandResult_Int (V); }
inline CommandResult *CRPTR (int64_t V) { return new CommandResult_Int (V); }
inline CommandResult *CRPTR (float V) { return new CommandResult_Float (V); }
inline CommandResult *CRPTR (double V) { return new CommandResult_Float (V); }
inline CommandResult *CRPTR (char *V) { return new CommandResult_String (std::string(V)); }
inline CommandResult *CRPTR (std::string& V) { return new CommandResult_String (V); }
inline CommandResult *CRPTR (Function& V) { return new CommandResult_Function (V); }
inline CommandResult *CRPTR (Statement& V) { return new CommandResult_Statement (V); }
inline CommandResult *CRPTR (LinkedObject& V) { return new CommandResult_LinkedObject (V); }
inline CommandResult* CRPTR (Time& V) { return new CommandResult_Time (V); }


enum Command_Status
{
  CMD_UNKNOWN,
  CMD_PARSED,
  CMD_EXECUTING,
  CMD_COMPLETE,
  CMD_ERROR,
  CMD_ABORTED 
};

class CommandObject
{
  InputLineObject *Associated_Clip; // The input line that caused generation of this object.
  int64_t Seq_Num; // The order this object was generated in from the input line.
  Command_Status Cmd_Status;
  oss_cmd_enum Cmd_Type; // A copy of information in the Parse_Result.
  OpenSpeedShop::cli::ParseResult *PR;
  bool result_needed_in_python;  // Don't Print to ostream if this is set!
  bool results_used; // Once used, this object can be deleted!
  std::list<CommandResult *> CMD_Result;
  std::list<CommandResult_RawString *> CMD_Annotation;
  pthread_cond_t wait_on_dependency;

  void Associate_Input ()
  {
    Associated_Clip = Current_ILO;
    Link_Cmd_Obj_to_Input (Associated_Clip, this);
  }

  void Attach_Result (CommandResult *R) {
    CMD_Result.push_back(R);
  }

public:
  CommandObject()
  {
    Associated_Clip = NULL;
    Seq_Num = 0;
    Cmd_Status = CMD_ERROR;
    Cmd_Type =  CMD_HEAD_ERROR;
    PR = NULL;
    result_needed_in_python = false;
    results_used = false;
    pthread_cond_init(&wait_on_dependency, (pthread_condattr_t *)NULL);
  }
  CommandObject(OpenSpeedShop::cli::ParseResult *pr, bool use_by_python)
  {
    this->Associate_Input ();
    Cmd_Status = CMD_PARSED;
    Cmd_Type =  pr->getCommandType();
    PR = pr;
    result_needed_in_python = use_by_python;
    results_used = false;
    pthread_cond_init(&wait_on_dependency, (pthread_condattr_t *)NULL);
  }
  ~CommandObject() {
   // Destroy ParseResult object
    if (PR != NULL) delete PR;

   // Reclaim results
    Reclaim_CR_Space (CMD_Result);

   // Reclaim annotations.
    Reclaim_CR_Space (CMD_Annotation);

   // Safety check.
    pthread_cond_destroy (&wait_on_dependency);
  }

  InputLineObject *Clip () { return Associated_Clip; }
  Command_Status Status () { return Cmd_Status; }
  oss_cmd_enum Type () { return Cmd_Type; }
  bool Needed_By_Python () { return result_needed_in_python; }
  bool Results_Used () { return results_used; }
  OpenSpeedShop::cli::ParseResult *P_Result () { return PR; }
  void clearParseResult () { PR = NULL; }
  OpenSpeedShop::cli::ParseResult *swapParseResult (OpenSpeedShop::cli::ParseResult *P) {
    OpenSpeedShop::cli::ParseResult *pr = PR;
    PR = P;
    return pr;
  }
  // command_t *P_Result () { return Parse_Result; }
  //command_type_t *P_Result () { return Parse_Result; }

  void Wait_On_Dependency (pthread_mutex_t& exp_lock) {
   // Suspend processing of the command.

   // Release the lock and wait for the all-clear signal.
    Assert(pthread_cond_wait(&wait_on_dependency,&exp_lock) == 0);

   // Release the recently acquired lock and continue processing the command.
    Assert(pthread_mutex_unlock(&exp_lock) == 0);
  }
  void All_Clear () {
   // Release the suspended command.
    Assert(pthread_cond_signal(&wait_on_dependency) == 0);
  }
    
  void SetSeqNum (int64_t a) { Seq_Num = a; }
  void set_Status (Command_Status S); // defined in CommandObject.cxx
  void set_Results_Used () { results_used = true; }

  void Result_Uint (uint64_t U) {
    Attach_Result (CRPTR (U));
  }
  void Result_Int (int64_t I) {
    Attach_Result (CRPTR (I));
  }
  void Result_Float (double F) {
    Attach_Result (CRPTR (F));
  }
  void Result_String (std::string S) {
    Attach_Result (CRPTR (S));
  }
  void Result_String (char *C) {
    Attach_Result (CRPTR (C));
  }
  void Result_RawString (std::string S) {
    Attach_Result (new CommandResult_RawString (S));
  }
  void Result_RawString (char *C) {
    Attach_Result (new CommandResult_RawString (C));
  }
  void Result_Predefined (CommandResult *C) {
    Attach_Result (C);
  }
  void Result_Predefined (std::list<CommandResult *>& R) {
    CMD_Result.splice( CMD_Result.end(), R);
  }

  std::list<CommandResult *>& Result_List () {
    return CMD_Result;
  }

  void Result_Annotation (std::string S) {
    CMD_Annotation.push_back(new CommandResult_RawString (S));
  }

  std::list<CommandResult_RawString *> Annotation_List () {
    return CMD_Annotation;
  }

 // The following are defined in CommandObject.cxx

 // The simple Print is for dumping information to a trace file.
  void Print (ostream& mystream);
 // The Print_Results routine is for sending results to the user.
 // The result returned is "true" if there was information printed.
  bool Print_Results (ostream& to, std::string list_seperator, std::string termination_char);
};
