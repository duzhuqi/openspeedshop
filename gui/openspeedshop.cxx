/*! \class OpenSpeedshop
    This is the implementation of the MainWinwdow container for Open Speed
    Shop.  It contains the container for the statusBar, and the menu bar.
 */

#include "openspeedshop.hxx"

#include <qvariant.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qimage.h>
#include <qpixmap.h>

#include "openspeedshop.ui.hxx"
/*!
 *  Constructs a OpenSpeedshop as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
OpenSpeedshop::OpenSpeedshop( QWidget* parent, const char* name, WFlags fl )
    : QMainWindow( parent, name, fl )
{
  lfd = NULL;
  sfd = NULL;
  pidStr = QString::null;
  executableName = QString::null;
  experimentName = QString::null;
  argsStr = QString::null;
  rankStr = QString::null;
  hostStr = QString::null;
  expStr = QString::null;

  (void)statusBar();

  if ( !name )
  {
	setName( "OpenSpeedshop" );
  }
  setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth() ) );
  setCentralWidget( new QWidget( this, "qt_central_widget" ) );
  OpenSpeedshopLayout = new QVBoxLayout( centralWidget(), 11, 6, "OpenSpeedshopLayout"); 

  // actions
#ifdef MOVE_FILE_ATTACH_ACTIONS
  fileLoadNewAction = new QAction( this, "fileLoadNewAction" );
  fileLoadNewAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "filenew" ) ) );

  fileAttachNewProcessAction = new QAction( this, "fileAttachNewProcessAction" );
  fileAttachNewProcessAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "process" ) ) );
#endif // MOVE_FILE_ATTACH_ACTIONS

  fileOpenAction = new QAction( this, "fileOpenAction" );
  fileOpenAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "fileopen" ) ) );
  fileSaveAction = new QAction( this, "fileSaveAction" );
  fileSaveAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "filesave" ) ) );
#ifdef EVENTUALLY
  fileSaveAsAction = new QAction( this, "fileSaveAsAction" );
#endif // EVENTUALLY
  fileExitAction = new QAction( this, "fileExitAction" );
#ifdef EVENTUALLY
  editUndoAction = new QAction( this, "editUndoAction" );
  editUndoAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "undo" ) ) );
  editRedoAction = new QAction( this, "editRedoAction" );
  editRedoAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "redo" ) ) );
  editCutAction = new QAction( this, "editCutAction" );
  editCutAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "editcut" ) ) );
  editCopyAction = new QAction( this, "editCopyAction" );
  editCopyAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "editcopy" ) ) );
  editPasteAction = new QAction( this, "editPasteAction" );
  editPasteAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "editpaste" ) ) );
  editFindAction = new QAction( this, "editFindAction" );
  editFindAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "searchfind" ) ) );
#endif // EVENTUALLY
  helpContentsAction = new QAction( this, "helpContentsAction" );
  helpIndexAction = new QAction( this, "helpIndexAction" );
  helpAboutAction = new QAction( this, "helpAboutAction" );


  // menubar
  menubar = new QMenuBar( this, "menubar" );

  fileMenu = new QPopupMenu( this );
#ifdef MOVE_FILE_ATTACH_ACTIONS
  fileLoadNewAction->addTo( fileMenu );
  fileAttachNewProcessAction->addTo( fileMenu );
#endif // MOVE_FILE_ATTACH_ACTIONS
  fileOpenAction->addTo( fileMenu );
  fileSaveAction->addTo( fileMenu );
#ifdef EVENTUALLY
  fileSaveAsAction->addTo( fileMenu );
#endif // EVENTUALLY
  fileMenu->insertSeparator();
  fileMenu->insertSeparator();
  fileExitAction->addTo( fileMenu );
  menubar->insertItem( QString(""), fileMenu, 1 );

  editMenu = new QPopupMenu( this );
#ifdef EVENTUALLY
  editUndoAction->addTo( editMenu );
  editRedoAction->addTo( editMenu );
  editMenu->insertSeparator();
  editCutAction->addTo( editMenu );
  editCopyAction->addTo( editMenu );
  editPasteAction->addTo( editMenu );
  editMenu->insertSeparator();
  editFindAction->addTo( editMenu );
  menubar->insertItem( QString(""), editMenu, 2 );
#endif // EVENTUALLY


    // signals and slots connections
#ifdef MOVE_FILE_ATTACH_ACTIONS
  connect( fileLoadNewAction, SIGNAL( activated() ), this, SLOT( fileLoadNewProgram() ) );
  connect( fileAttachNewProcessAction, SIGNAL( activated() ), this, SLOT( fileAttachNewProcess() ) );
#endif // MOVE_FILE_ATTACH_ACTIONS
  connect( fileOpenAction, SIGNAL( activated() ), this, SLOT( fileOpen() ) );
  connect( fileSaveAction, SIGNAL( activated() ), this, SLOT( fileSave() ) );
#ifdef EVENTUALLY
  connect( fileSaveAsAction, SIGNAL( activated() ), this, SLOT( fileSaveAs() ) );
#endif // EVENTUALLY
  connect( fileExitAction, SIGNAL( activated() ), this, SLOT( fileExit() ) );
#ifdef EVENTUALLY
  connect( editUndoAction, SIGNAL( activated() ), this, SLOT( editUndo() ) );
  connect( editRedoAction, SIGNAL( activated() ), this, SLOT( editRedo() ) );
  connect( editCutAction, SIGNAL( activated() ), this, SLOT( editCut() ) );
  connect( editPasteAction, SIGNAL( activated() ), this, SLOT( editPaste() ) );
  connect( editFindAction, SIGNAL( activated() ), this, SLOT( editFind() ) );
#endif // EVENTUALLY
  connect( helpIndexAction, SIGNAL( activated() ), this, SLOT( helpIndex() ) );
  connect( helpContentsAction, SIGNAL( activated() ), this, SLOT( helpContents() ) );
  connect( helpAboutAction, SIGNAL( activated() ), this, SLOT( helpAbout() ) );
  init();


  menubar->insertSeparator();

  helpMenu = new QPopupMenu( this );
  helpContentsAction->addTo( helpMenu );
  helpIndexAction->addTo( helpMenu );
  helpMenu->insertSeparator();
  helpAboutAction->addTo( helpMenu );
  menubar->insertItem( tr("&Help"), helpMenu );

  assistant = new QAssistantClient(NULL);
//    assistant->setArguments(QStringList("-hideSidebar"));
  QStringList slist;
  slist.append("-profile");
  slist.append("doc/help.adp");
//    slist.append("-hideSidebar");
  assistant->setArguments(slist);

  languageChange();

  resize( QSize(850, 620).expandedTo(minimumSizeHint()) );
  clearWState( WState_Polished );
}

/*!
 *  Destroys the object and frees any allocated resources
 */
OpenSpeedshop::~OpenSpeedshop()
{
    destroy();
    // no need to delete child widgets, Qt does it all for us
}

/*!
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void OpenSpeedshop::languageChange()
{
  setCaption( tr( "Open/SpeedShop" ) );
#ifdef MOVE_FILE_ATTACH_ACTIONS
  fileLoadNewAction->setText( tr( "Load New Program " ) );
  fileLoadNewAction->setMenuText( tr( "Load &New Program" ) );
  fileLoadNewAction->setAccel( tr( "Ctrl+N" ) );
  fileAttachNewProcessAction->setText( tr( "Attach To Executable " ) );
  fileAttachNewProcessAction->setMenuText( tr( "&Attach To Executable" ) );
  fileAttachNewProcessAction->setAccel( tr( "Ctrl+A" ) );
#endif // MOVE_FILE_ATTACH_ACTIONS
  fileOpenAction->setText( tr( "Open Saved Experiment..." ) );
  fileOpenAction->setMenuText( tr( "&Open Saved Experiment..." ) );
  fileOpenAction->setAccel( tr( "Ctrl+O" ) );
  fileSaveAction->setText( tr( "Save Window Setup" ) );
  fileSaveAction->setMenuText( tr( "&Save Window Setup" ) );
  fileSaveAction->setAccel( tr( "Ctrl+S" ) );
#ifdef EVENTUALLY
  fileSaveAsAction->setText( tr( "Save As" ) );
  fileSaveAsAction->setMenuText( tr( "Save &As..." ) );
  fileSaveAsAction->setAccel( QString::null );
#endif // EVENTUALLY
  fileExitAction->setText( tr( "Exit" ) );
  fileExitAction->setMenuText( tr( "E&xit" ) );
  fileExitAction->setAccel( QString::null );
#ifdef EVENTUALLY
  editUndoAction->setText( tr( "Undo" ) );
  editUndoAction->setMenuText( tr( "&Undo" ) );
  editUndoAction->setAccel( tr( "Ctrl+Z" ) );
  editRedoAction->setText( tr( "Redo" ) );
  editRedoAction->setMenuText( tr( "&Redo" ) );
  editRedoAction->setAccel( tr( "Ctrl+Y" ) );
  editCutAction->setText( tr( "Cut" ) );
  editCutAction->setMenuText( tr( "&Cut" ) );
  editCutAction->setAccel( tr( "Ctrl+X" ) );
  editCopyAction->setText( tr( "Copy" ) );
  editCopyAction->setMenuText( tr( "C&opy" ) );
  editCopyAction->setAccel( tr( "Ctrl+C" ) );
  editPasteAction->setText( tr( "Paste" ) );
  editPasteAction->setMenuText( tr( "&Paste" ) );
  editPasteAction->setAccel( tr( "Ctrl+V" ) );
  editFindAction->setText( tr( "Find" ) );
  editFindAction->setMenuText( tr( "&Find..." ) );
  editFindAction->setAccel( tr( "Ctrl+F" ) );
#endif // EVENTUALLY
  helpContentsAction->setText( tr( "Contents" ) );
  helpContentsAction->setMenuText( tr( "&Contents..." ) );
  helpContentsAction->setAccel( QString::null );
  helpIndexAction->setText( tr( "Index" ) );
  helpIndexAction->setMenuText( tr( "&Index..." ) );
  helpIndexAction->setAccel( QString::null );
  helpAboutAction->setText( tr( "About" ) );
  helpAboutAction->setMenuText( tr( "&About" ) );
  helpAboutAction->setAccel( QString::null );
  if (menubar->findItem(1))
  {
    menubar->findItem(1)->setText( tr( "&File" ) );
  }
#ifdef EVENTUALLY
  if (menubar->findItem(2))
  {
    menubar->findItem(2)->setText( tr( "&Edit" ) );
  }
#endif // EVENTUALLY
#ifdef HOLD
  if (menubar->findItem(3))
  {
    menubar->findItem(3)->setText( tr( "&Help" ) );
  }
#endif // HOLD
}

void
OpenSpeedshop::print()
{
  printf("pidStr = %s\n",  pidStr.ascii() );
  printf("executableName = %s\n",  executableName.ascii() );
  printf("argsStr = %s\n",  argsStr.ascii() );
  printf("rankStr = %s\n",  rankStr.ascii() );
  printf("hostStr = %s\n",  hostStr.ascii() );
  printf("expStr = %s\n",  expStr.ascii() );
}
