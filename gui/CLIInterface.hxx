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


#ifndef CLIINTERFACE_H
#define CLIINTERFACE_H

#include "qobject.h"
#include "qevent.h"
#include "qtimer.h"

#include "SS_Input_Manager.hxx"

// This will need to be larger for a shipped product... something like 30-60
// seconds.    However, for our current testing purposes. 5 seconds seems
// like a number that doesn't irrirtate too much.
#define MAXTIME 5000

class CLIInterface : QObject
{
  //! Q_OBJECT is needed as there are slots defined
  Q_OBJECT
  public:
    //! Constructor for the command line inteface (cli) interface.
    CLIInterface(int _wid);

    //! Destructor for the command line inteface (cli) interface.
    ~CLIInterface();

    //! The window id (defined in the cli) for this connection.
    int wid;

    //! Return a list of string values from a given cli command.
    bool getStringListValueFromCLI(char *command, std::list<std::string> *string_list, bool mark_value_for_delete=true, int mt=MAXTIME, bool warn_of_time=false );

    //! Return string value from a given cli command.
    bool getStringValueFromCLI(char *command, std::string *val_string, bool mark_value_for_delete = true , int maxTime = MAXTIME, bool warn_of_time = true  );

    //! Return a list of int values from a given cli command.
    bool getIntListValueFromCLI(char *command, std::list<int64_t> *int_list, bool mark_value_for_delete=true, int mt=MAXTIME, bool warn_of_time=false );

    //! Return an int value from a given cli command.
    bool getIntValueFromCLI(char *command, int64_t *val, bool mark_value_for_delete = true , int maxTime = MAXTIME, bool warn_of_time = true  );

    //! Run a command and return a success or failure result.
    bool runSynchronousCLI(char *command, int maxTime = MAXTIME, bool warn_of_time = true );

#ifdef PULL
    //! A routine to set the interrupt flag.
    void setInterrupt(bool val) { interrupt = val; };

    //! A routine to get the interrupt flag.
    bool getInterrupt() { return interrupt; };
#endif // PULL
   
    //! The flag set when a command has been interrupted.
    static bool interrupt;

  private:
    //! The timer set to how long the command should wait before aborting.
    QTimer *timer;

    //! The time value for the timer.
    int maxTime;

    //! The flag to tell if we need to warn before returning.
    bool warn_of_time;

    //! A routine to check status and timer before continuing.
    bool shouldWeContinue();

    //! A routine to check status of the command.
    Input_Line_Status checkStatus(InputLineObject *clip);

  public slots:
    void wakeupFromTimer();

};

#endif // CLIINTERFACE_H
