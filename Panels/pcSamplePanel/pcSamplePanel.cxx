#include "pcSamplePanel.hxx"   // Change this to your new class header file name
#include "PanelContainer.hxx"   // Do not remove
#include "plugin_entry_point.hxx"   // Do not remove

#include <qapplication.h>
#include <qbuttongroup.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qiconset.h>

#include <qbitmap.h>

#include <qmessagebox.h>

#include "SourcePanel.hxx"
#include "SourceObject.hxx"
#include "SourceObject.hxx"
#include "TopPanel.hxx"

#include "LoadAttachObject.hxx"



/*!  pcSamplePanel Class

     Autor: Al Stipek (stipek@sgi.com)
 */


/*! The default constructor.   Unused. */
pcSamplePanel::pcSamplePanel()
{ // Unused... Here for completeness...
}


/*! Constructs a new UserPanel object */
/*! This is the most often used constructor call.
    \param pc    The panel container the panel will initially be attached.
    \param n     The initial name of the panel container
 */
pcSamplePanel::pcSamplePanel(PanelContainer *pc, const char *n) : Panel(pc, n)
{
  nprintf( DEBUG_CONST_DESTRUCT ) ("pcSamplePanel::pcSamplePanel() constructor called\n");

  mw = getPanelContainer()->getMainWindow();

  frameLayout = new QVBoxLayout( getBaseWidgetFrame(), 1, 2, getName() );

  pco = new ProcessControlObject(frameLayout, getBaseWidgetFrame(), (Panel *)this );
  runnableFLAG = FALSE;
  pco->runButton->setEnabled(FALSE);
  pco->runButton->enabledFLAG = FALSE;

  statusLayout = new QHBoxLayout( 0, 10, 0, "statusLayout" );
  
  statusLabel = new QLabel( getBaseWidgetFrame(), "statusLabel");
  statusLayout->addWidget( statusLabel );

  statusLabelText = new QLineEdit( getBaseWidgetFrame(), "statusLabelText");
  statusLabelText->setReadOnly(TRUE);
  statusLayout->addWidget( statusLabelText );

  frameLayout->addLayout( statusLayout );

  languageChange();

  PanelContainerList *lpcl = new PanelContainerList();
  lpcl->clear();

  QWidget *pcSampleControlPanelContainerWidget = new QWidget( getBaseWidgetFrame(),
                                        "pcSampleControlPanelContainerWidget" );
  topPC = createPanelContainer( pcSampleControlPanelContainerWidget,
                              "PCSamplingControlPanel_topPC", NULL,
                              pc->getMasterPCList() );
  frameLayout->addWidget( pcSampleControlPanelContainerWidget );

printf("Create an Application\n");
printf("# Application theApplication;\n");
printf("# //Create a process for the command in the suspended state\n");
printf("# load the executable or attach to the process.\n");
printf("# if( loadExecutable ) {\n");
printf("# theApplication.createProcess(command);\n");
printf("# } else { // attach \n");
printf("#   theApplication.attachToPrcess(pid, hostname);\n");
printf("# }\n");
printf("set the parameters.\n");
printf("# pcCollector.setParameter(param);  // obviously looping over each.\n");
printf("attach the collector to the Application.\n");
printf("# // Attach the collector to all threads in the application\n");
printf("# theApplication.attachCollector(theCollector.getValue());\n");

  pcSampleControlPanelContainerWidget->show();
  topPC->show();
  topLevel = TRUE;
  topPC->topLevel = TRUE;

  SourcePanel *sp = (SourcePanel *)topPC->dl_create_and_add_panel("Source Panel", topPC);

// Begin demo position at dummy file... For the real stuff we'll need to 
// look up the main()... and position at it...
if( mw && !mw->executableName.isEmpty() && mw->executableName.endsWith("fred_calls_ted") )
{
  char *plugin_directory = getenv("OPENSPEEDSHOP_PLUGIN_PATH");
  char buffer[200];
  strcpy(buffer, plugin_directory);
  strcat(buffer, "/../../../usability/phaseI/fred_calls_ted.c");
printf("load (%s)\n", buffer);
  SourceObject *spo = new SourceObject("main", buffer, 22, TRUE, NULL);

  if( !sp->listener((void *)spo) )
  {
    fprintf(stderr, "Unable to position at main in %s\n", buffer);
  } else
  {
nprintf( DEBUG_CONST_DESTRUCT ) ("Positioned at main in %s ????? \n", buffer);
  }
}
// End demo.

  updateInitialStatus();
  
}


//! Destroys the object and frees any allocated resources
/*! The only thing that needs to be cleaned up is the baseWidgetFrame.
 */
pcSamplePanel::~pcSamplePanel()
{
  nprintf( DEBUG_CONST_DESTRUCT ) ("  pcSamplePanel::~pcSamplePanel() destructor called\n");
  delete frameLayout;
}

//! Add user panel specific menu items if they have any.
bool
pcSamplePanel::menu(QPopupMenu* contextMenu)
{
  nprintf( DEBUG_PANELS ) ("pcSamplePanel::menu() requested.\n");

  contextMenu->insertSeparator();
  contextMenu->insertItem("&Save As ...", this, SLOT(saveAsSelected()), CTRL+Key_S ); 
  contextMenu->insertSeparator();
  contextMenu->insertItem(tr("Load &New Program..."), this, SLOT(loadNewProgramSelected()), CTRL+Key_N );
  contextMenu->insertItem(tr("Detach &From Program..."), this, SLOT(detachFromProgramSelected()), CTRL+Key_N );
  contextMenu->insertItem(tr("Attach To &Executable..."), this, SLOT(attachToExecutableSelected()), CTRL+Key_E );
  contextMenu->insertSeparator();
  contextMenu->insertItem(tr("&Manage Collectors..."), this, SLOT(manageCollectorsSelected()), CTRL+Key_M );
  contextMenu->insertItem(tr("Manage &Processes..."), this, SLOT(manageProcessesSelected()), CTRL+Key_P );
  contextMenu->insertItem(tr("&Manage &Data Sets..."), this, SLOT(manageDataSetsSelected()), CTRL+Key_D );
  contextMenu->insertSeparator();
  contextMenu->insertItem(tr("S&ource Panel..."), this, SLOT(loadSourcePanel()), CTRL+Key_O );

  return( TRUE );
}

void
pcSamplePanel::loadNewProgramSelected()
{
  nprintf( DEBUG_PANELS ) ("pcSamplePanel::loadNewProgramSelected()\n");
  if( runnableFLAG == TRUE )
  {
    printf("Disconnect First?\n"); 
    if( detachFromProgramSelected() == FALSE )
    {
      return;
    }
  }
  if( mw )
  {
    mw->executableName = QString::null;
    mw->pidStr = QString::null;
    mw->fileLoadNewProgram();
  }
}   

bool
pcSamplePanel::detachFromProgramSelected()
{
  nprintf( DEBUG_PANELS ) ("pcSamplePanel::detachFromProgramSelected()\n");

  
  if( QMessageBox::question(
            this,
            tr("Detach?"),
            tr("Process or executable already attached: Do you want to detach from the exising process(es)?"),
            tr("&Yes"), tr("&No"),
            QString::null, 0, 1 ) )
  {
    return FALSE;
  }

  if( mw )
  {
    mw->executableName = QString::null;
    mw->pidStr = QString::null;
  }

  updateInitialStatus();

  SourceObject *spo = new SourceObject(QString::null, QString::null, 0, TRUE, NULL);

  broadcast((char *)spo, NEAREST_T);

runnableFLAG = FALSE;
}

void
pcSamplePanel::attachToExecutableSelected()
{
  nprintf( DEBUG_PANELS ) ("pcSamplePanel::attachToExecutableSelected()\n");
  if( runnableFLAG == TRUE )
  {
    if( detachFromProgramSelected() == FALSE )
    {
      return;
    }
  }
  if( mw )
  {
    mw->executableName = QString::null;
    mw->pidStr = QString::null;
    mw->fileAttachNewProcess();
  }
}   

void
pcSamplePanel::manageCollectorsSelected()
{
  nprintf( DEBUG_PANELS ) ("pcSamplePanel::manageCollectorsSelected()\n");
}   

void
pcSamplePanel::manageProcessesSelected()
{
  nprintf( DEBUG_PANELS ) ("pcSamplePanel::managerProcessesSelected()\n");
}   

void
pcSamplePanel::manageDataSetsSelected()
{
  nprintf( DEBUG_PANELS ) ("pcSamplePanel::managerDataSetsSelected()\n");
}   

//! Save ascii version of this panel.
/*! If the user panel provides save to ascii functionality, their function
     should provide the saving.
 */
void 
pcSamplePanel::save()
{
  nprintf( DEBUG_PANELS ) ("pcSamplePanel::save() requested.\n");
}

//! Save ascii version of this panel (to a file).
/*! If the user panel provides save to ascii functionality, their function
     should provide the saving.  This callback will invoke a popup prompting
     for a file name.
 */
void 
pcSamplePanel::saveAs()
{
  nprintf( DEBUG_SAVEAS ) ("pcSamplePanel::saveAs() requested.\n");
}

//! This function listens for messages.
int 
pcSamplePanel::listener(void *msg)
{
  nprintf( DEBUG_MESSAGES ) ("pcSamplePanel::listener() requested.\n");
  int ret_val = 0; // zero means we didn't handle the message.

  ControlObject *co = NULL;
  LoadAttachObject *lao = NULL;

  MessageObject *mo = (MessageObject *)msg;

  if( mo->msgType  == "ControlObject" )
  {
    co = (ControlObject *)msg;
    nprintf( DEBUG_MESSAGES ) ("we've got a ControlObject\n");
  } else if( mo->msgType  == "LoadAttachObject" )
  {
    lao = (LoadAttachObject *)msg;
    nprintf( DEBUG_MESSAGES ) ("we've got a LoadAttachObject\n");
  } else
  {
//    fprintf(stderr, "Unknown object type recieved.\n");
//    fprintf(stderr, "msgType = %s\n", mo->msgType.ascii() );
    return 0;  // 0 means, did not act on message
  }

  if( co )
  {
//    if( DEBUG_MESSAGES )
//    {
//      co->print();
//    }

    switch( (int)co->cot )
    {
      case  ATTACH_PROCESS_T:
        nprintf( DEBUG_MESSAGES ) ("Attach to a process\n");
        break;
      case  DETACH_PROCESS_T:
        nprintf( DEBUG_MESSAGES ) ("Detach from a process\n");
        ret_val = 1;
        break;
      case  ATTACH_COLLECTOR_T:
        nprintf( DEBUG_MESSAGES ) ("Attach to a collector\n");
        ret_val = 1;
        break;
      case  REMOVE_COLLECTOR_T:
        nprintf( DEBUG_MESSAGES ) ("Remove a collector\n");
        ret_val = 1;
        break;
      case  RUN_T:
        nprintf( DEBUG_MESSAGES ) ("Run\n");
        statusLabelText->setText( tr("Process running...") );
{ // Begin demo only...
pco->runButton->setEnabled(FALSE);
pco->runButton->enabledFLAG = FALSE;
qApp->processEvents(1);
sleep(1);
qApp->processEvents(1);
sleep(1);
qApp->processEvents(1);
sleep(1);
qApp->processEvents(1);
sleep(1);
statusLabelText->setText( tr("Process completed...") );
sleep(1);
qApp->processEvents(1);
pco->runButton->setEnabled(TRUE);
runnableFLAG = TRUE;
pco->pauseButton->setEnabled(FALSE);
pco->pauseButton->enabledFLAG = FALSE;
pco->continueButton->setEnabled(FALSE);
pco->continueButton->setEnabled(FALSE);
pco->continueButton->enabledFLAG = FALSE;
pco->updateButton->setEnabled(FALSE);
pco->updateButton->setEnabled(FALSE);
pco->updateButton->enabledFLAG = FALSE;
pco->terminateButton->setEnabled(FALSE);
pco->terminateButton->setFlat(TRUE);
pco->terminateButton->setEnabled(FALSE);

// TopPanel *tp = (TopPanel *)topPC->dl_create_and_add_panel("Top Panel", topPC); 
// TopPanel *tp = (TopPanel *)getPanelContainer()->getMasterPC()->dl_create_and_add_panel("Top Panel", topPC); 
// TopPanel *tp = (TopPanel *)topPC->getMasterPC()->dl_create_and_add_panel("Top Panel", topPC); 

        Panel *p = getPanelContainer()->dl_create_and_add_panel("Top Panel", topPC);
} // End demo only...
        ret_val = 1;
        break;
      case  PAUSE_T:
        nprintf( DEBUG_MESSAGES ) ("Pause\n");
        statusLabelText->setText( tr("Process suspended...") );
        ret_val = 1;
        break;
      case  CONT_T:
        nprintf( DEBUG_MESSAGES ) ("Continue\n");
          statusLabelText->setText( tr("Process continued...") );
          sleep(1);
          statusLabelText->setText( tr("Process running...") );
        ret_val = 1;
        break;
      case  UPDATE_T:
        nprintf( DEBUG_MESSAGES ) ("Update\n");
        ret_val = 1;
        break;
      case  INTERRUPT_T:
        nprintf( DEBUG_MESSAGES ) ("Interrupt\n");
        ret_val = 1;
        break;
      case  TERMINATE_T:
        statusLabelText->setText( tr("Process terminated...") );
        ret_val = 1;
 //       nprintf( DEBUG_MESSAGES ) ("Terminate\n");
        break;
      default:
        break;
    }
 } else if( lao )
 {
   nprintf( DEBUG_MESSAGES ) ("we've got a LoadAttachObject message\n");

   if( lao->loadNowHint == TRUE || runnableFLAG == FALSE )
   {
     mw->executableName = lao->executableName;
     mw->pidStr = lao->pidStr;
     updateInitialStatus();
 
     ret_val = 1;
    }
 }

  return ret_val;  // 0 means, did not want this message and did not act on anything.
}

void
pcSamplePanel::updateInitialStatus()
{
  if( !mw->executableName.isEmpty() )
  {

    // Begin demo position at dummy file... For the real stuff we'll need to 
    // look up the main()... and position at it...
    if( mw && !mw->executableName.isEmpty() && mw->executableName.endsWith("fred_calls_ted") )
    {
      statusLabelText->setText( tr(QString("Loaded:  "))+mw->executableName+tr(QString("  Click on the Run button to begin the experiment.")) );
      char *plugin_directory = getenv("OPENSPEEDSHOP_PLUGIN_PATH");
      char buffer[200];
      strcpy(buffer, plugin_directory);
      strcat(buffer, "/../../../usability/phaseI/fred_calls_ted.c");
      SourceObject *spo = new SourceObject("main", buffer, 22, TRUE, NULL);

      if( broadcast((char *)spo, NEAREST_T) == 0 )
      { // No source view up...
        char *panel_type = "Source Panel";
        Panel *p = getPanelContainer()->dl_create_and_add_panel(panel_type, topPC);
        if( p != NULL )
        {
          if( !p->listener((void *)spo) )
          {
            fprintf(stderr, "Unable to position at main in %s\n", buffer);
          } else
          {
            nprintf( DEBUG_CONST_DESTRUCT ) ("Positioned at main in %s ????? \n", buffer);
          }
        }
        runnableFLAG = TRUE;
        pco->runButton->setEnabled(TRUE);
        pco->runButton->enabledFLAG = TRUE;
      }
    } else
    {
      QString msg = QString(tr("File entered is not an executable file.  No main() entry found.\n") );
      QMessageBox::information( (QWidget *)this, "Process or executable needed...",
                                   msg, QMessageBox::Ok );
      runnableFLAG = FALSE;
      pco->runButton->setEnabled(FALSE);
      pco->runButton->enabledFLAG = FALSE;
      return;
    }
// End demo.

  } else if( !mw->pidStr.isEmpty() )
  {
    if( runnableFLAG == TRUE )
    {
      if( detachFromProgramSelected() == FALSE )
      {
        return;
      }
    }
    statusLabelText->setText( tr(QString("Attached to:  "))+mw->pidStr+tr(QString("  Click on the Run button to begin collecting data.")) );
  } else
  {
    statusLabel->setText( tr("Status:") ); statusLabelText->setText( tr("Select a process or executable with the \"Load New *... or Attach To *...\" menu.") );

    runnableFLAG = FALSE;
    pco->runButton->setEnabled(FALSE);
    pco->runButton->enabledFLAG = FALSE;
    return;
  }
  runnableFLAG = TRUE;
  pco->runButton->setEnabled(TRUE);
  pco->runButton->enabledFLAG = TRUE;
}

/*
*  Sets the strings of the subwidgets using the current
 *  language.
 */
void
pcSamplePanel::languageChange()
{
  statusLabel->setText( tr("Status:") ); statusLabelText->setText( tr("Select a process or executable with the \"Load New *... or Attach To *...\" menu.") );
}

#include "SaveAsObject.hxx"
void
pcSamplePanel::saveAsSelected()
{
  nprintf( DEBUG_CONST_DESTRUCT ) ("From this pc on down, send out a saveAs message and put it to a file.\n");

  QFileDialog *sfd = NULL;
  QString dirName = QString::null;
  if( sfd == NULL )
  {
    sfd = new QFileDialog(this, "file dialog", TRUE );
    sfd->setCaption( QFileDialog::tr("Enter filename:") );
    sfd->setMode( QFileDialog::AnyFile );
    sfd->setSelection(QString("pcSamplePanel.html"));
    QString types(
                  "Any Files (*);;"
                  "Image files (*.png *.xpm *.jpg);;"
                  "Text files (*.txt);;"
                  "(*.c *.cpp *.cxx *.C *.c++ *.f* *.F*);;"
                  );
    sfd->setFilters( types );
    sfd->setDir(dirName);
  }

  QString fileName = QString::null;
  if( sfd->exec() == QDialog::Accepted )
  {
    fileName = sfd->selectedFile();

    if( !fileName.isEmpty() )
    { 
      SaveAsObject *sao = new SaveAsObject(fileName);

      *sao->ts << "<html>";
      *sao->ts << "<head>";
      *sao->ts << "<meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"content-type\"> ";
      *sao->ts << "<title>pcSampleReport</title>";
      *sao->ts << "<h2>pcSampleReport</h2>";
      *sao->ts << "</head>";

sao->f->flush();

  
      broadcast((char *)sao, ALL_DECENDANTS_T, topPC);

      delete( sao );
    }
  }
}

void
pcSamplePanel::loadSourcePanel()
{
  printf("From this pc on down, send out a saveAs message and put it to a file.\n");
  SourcePanel *sp = (SourcePanel *)topPC->dl_create_and_add_panel("Source Panel", topPC);
}
