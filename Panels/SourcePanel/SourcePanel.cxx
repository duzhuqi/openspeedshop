/*! \class SourcePanel
 The SourcePanel is responsible for managing source files.

 This class will manage all source details.  Including highlighting/clearing
 lines, positioning source, searching the source, fielding context sensitive
 menus, basically all functions dealing with source file manipulation.
 
 */
#include "SourcePanel.hxx"   // Change this to your new class header file name
#include "PanelContainer.hxx"   // Do not remove
#include "plugin_entry_point.hxx"   // Do not remove

#include <stdlib.h>  // for atoi()

#include <qapplication.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qinputdialog.h>

#include <qmessagebox.h>

#include <qcursor.h>

#include "SourceObject.hxx"

#include "debug.hxx"


#ifdef OLDWAY
#include "InfoEventFilter.hxx"
#endif // OLDWAY

/*! Unused constructor. */
SourcePanel::SourcePanel()
{ // Unused... Here for completeness...
}


/*! This constructor creates a title label and a QTextEdit.
*/
SourcePanel::SourcePanel(PanelContainer *pc, const char *n) : Panel(pc, n)
{
  nprintf(DEBUG_CONST_DESTRUCT) ( "SourcePanel::SourcePanel() constructor called\n");
  frameLayout = new QVBoxLayout( getBaseWidgetFrame(), 1, 2, getName() );

  lastTop = 0;
  last_para = -1;   // For last find command.
  last_index = -1;   // For last find command.
  lastSearchText = QString::null;
  lineCount = 0;
  line_numbersFLAG = FALSE;
  highlightList = NULL;

  label = new QLabel( getBaseWidgetFrame(), "text label", 0 );
  label->setCaption("SourcePanel: text label");
  QFont font = label->font();
  font.setBold(TRUE);
  label->setFont(font);
  label->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  QString label_text = "No source file specified.";
  label->setText(label_text);

  textEdit = new SPTextEdit( this, getBaseWidgetFrame() );
  textEdit->setCaption("SourcePanel: SPTextEdit");

  addWhatsThis(textEdit, this);

  textEdit->setTextFormat(PlainText);  // This makes one para == 1 line.
  textEdit->setReadOnly(TRUE);
  textEdit->setWordWrap(QTextEdit::NoWrap);
  vscrollbar = textEdit->verticalScrollBar();
  hscrollbar = textEdit->horizontalScrollBar();
  if( vscrollbar )
  {
    connect( vscrollbar, SIGNAL(sliderReleased()),
           this, SLOT(valueChanged()) );
    connect( vscrollbar, SIGNAL(nextLine()),
           this, SLOT(valueChanged()) );
    connect( vscrollbar, SIGNAL(prevLine()),
           this, SLOT(valueChanged()) );
    connect( vscrollbar, SIGNAL(nextPage()),
           this, SLOT(valueChanged()) );
    connect( vscrollbar, SIGNAL(prevPage()),
           this, SLOT(valueChanged()) );
  }

  defaultColor = textEdit->color();

  connect( textEdit, SIGNAL(clicked(int, int)),
           this, SLOT(clicked(int, int)) );

  frameLayout->addWidget(label);
  frameLayout->addWidget(textEdit);

  textEdit->show();
  label->show();

  textEdit->setFocus();
}


/*!
 *  Destroys the object and frees any allocated resources
 */
SourcePanel::~SourcePanel()
{
  nprintf(DEBUG_CONST_DESTRUCT) ("  SourcePanel::~SourcePanel() destructor called\n");
  delete textEdit;
  delete label;
  delete frameLayout;
}

/*!
 * Add local panel options here..
 */
bool
SourcePanel::menu(QPopupMenu* contextMenu)
{
  nprintf(DEBUG_PANELS) ("SourcePanel::menu() requested.\n");

  contextMenu->insertSeparator();
  contextMenu->insertItem("&Open New", this, SLOT(chooseFile()), CTRL+Key_O );
  contextMenu->insertItem("&Goto Line...", this, SLOT(goToLine()), CTRL+Key_G );
  if( line_numbersFLAG == TRUE )
  {
    contextMenu->insertItem("Hide &Line Numbers...", this,
      SLOT(showLineNumbers()), CTRL+Key_L );
  } else
  {
    contextMenu->insertItem("Show &Line Numbers...", this,
      SLOT(showLineNumbers()), CTRL+Key_L );
  }
  contextMenu->insertItem("&Find...", this, SLOT(findString()), CTRL+Key_F );
  contextMenu->insertSeparator();
  contextMenu->insertItem("Zoom In", this, SLOT(zoomIn()), CTRL+Key_Plus );
  contextMenu->insertItem("Zoom Out", this, SLOT(zoomOut()), CTRL+Key_Minus );

  return( TRUE );
}

/*! This routine is called to create a context sensitive dynamic menu 
    base on where the mouse is located at the time of the request. */
void
SourcePanel::createPopupMenu( QPopupMenu* contextMenu, const QPoint &pos )
{
  nprintf(DEBUG_PANELS) ("Popup the context sensitive menu here.... can you augment it with the default popupmenu?\n");

  textEdit->setCursorPosition(textEdit->paragraphAt(pos), 0);
  if( whatIsAtPos(pos) )
  {
    contextMenu->insertItem("Tell Me MORE!!!", this, SLOT(details()), CTRL+Key_1 );
    contextMenu->insertItem("Who calls this routine?", this, SLOT(whoCallsMe()), CTRL+Key_2 );
    contextMenu->insertItem("What routines are called from here?", this, SLOT(whoDoICall()), CTRL+Key_2 );
  }
}

/*!
 * Add local save() functionality here.
 */
void
SourcePanel::save()
{
  nprintf(DEBUG_PANELS) ("SourcePanel::save() requested.\n");
}

/*! 
 * Add local saveAs() functionality here.
 */
void
SourcePanel::saveAs()
{
  nprintf(DEBUG_PANELS) ("SourcePanel::saveAs() requested.\n");
}

/*! 
 * The listener function fields requests to load and position source files.
 */
int 
SourcePanel::listener(void *msg)
{
  SourceObject *spo = (SourceObject *)msg;
  nprintf(DEBUG_PANELS) ("SourcePanel::listener() requested.\n");

  if( !spo )
  {
    return 0;  // 0 means, did not act on message.
  }

  // Check the message type to make sure it's our type...
  if( spo->msgType != "SourceObject" )
  {
    nprintf(DEBUG_PANELS) ("SourcePanel::listener() Not a SourceObject.\n");
    return 0; // o means, did not act on message.
  }

  if( spo->raiseFLAG == TRUE )
  {
    getPanelContainer()->raisePanel((Panel *)this);
  }

  loadFile(spo->fileName);

  highlightList = spo->highlightList;
  doFileHighlights();

  // Try to highlight the line...
/*
  highlightLine(spo->line_number, "yellow", TRUE);
*/
  hscrollbar->setValue(0);

  if( spo->raiseFLAG == TRUE )
  {
    int i=0;
    Panel *p = NULL;
    for( PanelList::Iterator it = getPanelContainer()->panelList.begin();
             it != getPanelContainer()->panelList.end();
             ++it )
    {
      p = (Panel *)*it;
  nprintf(DEBUG_PANELS) ("Is this it? (%s)\n", p->getName() );
      if( p == this )
      {
  nprintf(DEBUG_PANELS) ("Set page %d current.\n", i );
        getPanelContainer()->tabWidget->setCurrentPage(i);
  //      getPanelContainer()->tabWidget->showPage(i);
      }
      i++;
    }
  }

  nprintf(DEBUG_PANELS) ("Try to position at line %d\n", spo->line_number);
// textEdit->setUpdatesEnabled( TRUE );

  positionLineAtCenter(spo->line_number);
  
  return 1;
}

/*!
 * Add message broadcaster() functionality here.
 */
int
SourcePanel::broadcast(char *msg, BROADCAST_TYPE bt)
{
  nprintf(DEBUG_PANELS) ("SourcePanel::broadcast() requested.\n");
  return 0;
}

void
SourcePanel::info(QPoint p, QObject *target)
{
  nprintf(DEBUG_PANELS) ("SourcePanel::info() called.\n");
  int tew = 0;
  int vbw = 0;

  QPoint pos = textEdit->mapFromGlobal( p );

  // If we have a vertical scrollbar, see if the event was generated 
  // in the scrollbar area.
  if( textEdit->vbar )
  {
//    printf("we have a vbar\n");
    tew = textEdit->width();
    vbw = textEdit->vbar->width();
  }

  // If it was a scrollbar event then convert the pos.y location...
  if( textEdit->vbar && pos.x() > tew - vbw )
  { // This is a scrollbar pos.  Convert to a source location then continue
    // as usual.
    int vbh = textEdit->vbar->height() - (textEdit->vbar->width()*2);
    int y = pos.y()-textEdit->vbar->width();

// printf("first: The nominalized y=%d\n", y);
    if( y < 0 )
    {
//      y = 0;
      return;
    }
    if( y > vbh )
    {
//      y = vbh;
      return;
    }

    float ratio = (float)y/(float)vbh;
//    printf("second: The ratio=%f\n", ratio);

    int new_y = int (ratio * (float)textEdit->vbar->maxValue());
//    printf("finally: The new_y=%d (maxValue=%d)\n", new_y, textEdit->vbar->maxValue() );
  
    // set this to a textEdit.document() value...
    pos.setY( new_y );
  } else
  {
    pos.setY( pos.y() + vscrollbar->value() );
  }
//  printf("tew=%d vbw=%d pos.x()=%d pos.y()=%d\n", tew, vbw, pos.x(), pos.y() );

  // Now, based on the pos (location) see if there's anything interesting 
  // we can detail.
  int line = whatIsAtPos( pos );
  nprintf(DEBUG_PANELS) ("SourcePanel::info() line=%d\n", line);
  if( line <= 0 )
  {
//    printf("NOTHING TO DISPLAY\n");
    return;
  }

  char *desc = getDescription( line );

  QString msg;
  msg = QString(tr("Details?\nDescription for line %1: %2")).arg(line).arg(desc);
  displayWhatsThis(msg);
}

/*! This routine pops up a dialog box to select a file to be loaded. */
void
SourcePanel::chooseFile()
{
  nprintf(DEBUG_PANELS) ("SourcePanel::chooseFile() entered\n");

  QString fn = QFileDialog::getOpenFileName( QString::null, QString::null,
                           this);
  if( !fn.isEmpty() )
  {
    loadFile( fn );
  } else
  {
//    statusBar()->message( "Loading aborted", 2000 );
  }
}

/*! Go to a particalar line and position it at the center of the display. */
void
SourcePanel::goToLine()
{
  nprintf(DEBUG_PANELS) ("SourcePanel::goToLine() entered\n");
  bool ok;
  QString text = QInputDialog::getText(
          "Goto Line", "Enter line number:", QLineEdit::Normal,
          QString::null, &ok, this );
  if( ok && !text.isEmpty() )
  {
    // user entered something and pressed OK
    int line = atoi(text.ascii());
    nprintf(DEBUG_PANELS) ("goto line %d\n", line);
    positionLineAtCenter(line);
//    positionLineAtTop(line);
  } else
  {
    // user entered nothing or pressed Cancel
  }
}

/*! Display/Undisplay line numbers in the display. */
void
SourcePanel::showLineNumbers()
{
  if( line_numbersFLAG == TRUE )
  {
    line_numbersFLAG = FALSE;
  } else
  {
    line_numbersFLAG = TRUE;
  }
  nprintf(DEBUG_PANELS) ("SourcePanel::showLineNumbers() entered\n");
  loadFile( fileName );
}

/*! Pops up a dialog to find a particular string in the currently displayed
    source. */
void
SourcePanel::findString()
{
  nprintf(DEBUG_PANELS) ("SourcePanel::findString() entered\n");
  int para = last_para == -1 ? lastTop : last_para;
  int index = last_index == -1 ? 0 : last_index;
  bool ok;
  QString text = QInputDialog::getText(
          "Find String", "Enter search expression:", QLineEdit::Normal,
          lastSearchText, &ok, this );
  if( ok && !text.isEmpty() )
  {
    // user entered something and pressed OK
    if(  textEdit->find(text, TRUE, TRUE, TRUE, &para, &index ) )
    {
      positionLineAtTop(para);
      last_para = para;
      last_index = index+1;
      lastSearchText = text;
    } else
    {
       nprintf(DEBUG_PANELS) ("DIDN'T FIND THE STRING ANYMORE.\n");
      QString msg;
      msg = QString("No string %1 found from line %2 until the end of file").arg(lastSearchText).arg(para);
      QMessageBox::information( (QWidget *)this, "Search Results...",
                               msg, QMessageBox::Ok );
    }
  } else
  {
    // user entered nothing or pressed Cancel
  }
}

/*! If font has a larger  pointSize, bump up one size. */
void
SourcePanel::zoomIn()
{
  textEdit->zoomIn();
}

/*! If font has a maller  pointSize, bump down one size. */
void
SourcePanel::zoomOut()
{
  textEdit->zoomOut();
}

/* Load a given file in the display. */
void
SourcePanel::loadFile(const QString &_fileName)
{
  nprintf(DEBUG_PANELS) ("SourcePanel::loadFile() entered\n");

  bool sameFile = FALSE;
  if( fileName == _fileName )
  {
    sameFile = TRUE;
    nprintf(DEBUG_PANELS) ("loadFile:: sameFile: lastTop=%d\n", lastTop );
  } else
  {
    if( highlightList )
    {
      clearHighlightList();
    }
    lastTop = 0;
    last_para = -1;   // For last find command.
    last_index = -1;   // For last find command.

  }
  fileName = _fileName;

  QFile f( fileName );
  if( !f.open( IO_ReadOnly ) )
  {
    QString msg;
    msg = QString("Unable to open file: %1").arg(fileName);
    QMessageBox::information( (QWidget *)this, tr("Details..."),
                               msg, QMessageBox::Ok );
    return;
  }

  textEdit->hide();
  // Disabling highlights, makes updating the source much quicker and cleaner.
  textEdit->setUpdatesEnabled( FALSE );

  QTextStream ts( &f );
  textEdit->clear();
  if( line_numbersFLAG )
  {
    char line_number_buffer[10];
    QString line;
    QString line_number;
    QString new_line("\n");
    lineCount = 1;
    while( !ts.atEnd() )
    {
      line = ts.readLine();  // line of text excluding '\n'
      sprintf(line_number_buffer, "%6d ", lineCount);
      line_number = QString(line_number_buffer);
      textEdit->append(line_number+line+new_line);
      lineCount++;
    }
  } else
  {

    textEdit->setText( ts.read() );
    // Need to set cursor position so subsequent position requests are fielded.
    textEdit->setCursorPosition(0, 0);
  }
  lineCount = textEdit->paragraphs();
  lineCount--;

  nprintf(DEBUG_PANELS) ("lineCount=%d\n", lineCount);
  textEdit->setModified( FALSE );
  label->setText(fileName);

  clearAllHighlights();

  clearAllSelections();

  textEdit->setUpdatesEnabled( TRUE );

  textEdit->show();

  textEdit->clearScrollBar();

  if( sameFile == FALSE ) // Then position at top.
  {
    textEdit->moveCursor(QTextEdit::MoveHome, FALSE);
  } else
  {
    // Redisplay the high lights.
    doFileHighlights();

    positionLineAtTop(lastTop);
    nprintf(DEBUG_PANELS) ("loadFile:: down here sameFile: lastTop=%d\n", lastTop);
  }
}

/*! Get more information about the current posotion (if any). */
void
SourcePanel::details()
{
  nprintf(DEBUG_PANELS) ("SourcePanel::details() entered\n");

  int line = 0;
  int index = 0;
  textEdit->getCursorPosition(&line, &index);
  line++;

  char *desc = getDescription(line);

  QString msg;
  msg = QString("Details?\nDescription for line %1: %2").arg(line).arg(desc);
  QMessageBox::information( (QWidget *)this, tr("Details..."),
    msg, QMessageBox::Ok );

}

/*! prototype: Display the who calls me information. */
void
SourcePanel::whoCallsMe()
{
  nprintf(DEBUG_PANELS) ("SourcePanel::whoCallsMe() entered\n");

  int para = 0;
  int index = 0;
  textEdit->getCursorPosition(&para, &index);
  para++;

  char *desc = getDescription(para);

  QString msg;
  msg = QString("Who Calls Me?\nDescription for line %1: %2").arg(para).arg(desc);
  QMessageBox::information( (QWidget *)this, "Who Calls Me...",
    msg, QMessageBox::Ok );
}

/*! prototype: Display the who do I call information. */
void
SourcePanel::whoDoICall()
{
  nprintf(DEBUG_PANELS) ("SourcePanel::whoDoICall() entered\n");

  int para = 0;
  int index = 0;
  textEdit->getCursorPosition(&para, &index);
  para++;

  char *desc = getDescription(para);

  QString msg;
  msg = QString("Who Do I Call?\nDescription for line %1: %2").arg(para).arg(desc);
  QMessageBox::information( (QWidget *)this, "Who Do I Call...",
    msg, QMessageBox::Ok );
}


/*! Highlight a line with the given color. */
void
SourcePanel::highlightLine(int line, char *color, bool inverse)
{
  // para == line when QTextEdit is in PlainText mode
  line--;
  nprintf(DEBUG_PANELS) ("highlightLine(%d, %s, %d)\n", line, color, inverse);
  if( inverse )
  {
    textEdit->setParagraphBackgroundColor(line, QColor(color) );
  } else
  {
    textEdit->setSelection(line, 0, line, textEdit->paragraphLength(line));
    textEdit->setColor( color );
  }

  // Annotate the scrollbar for this highlight....
  textEdit->annotateScrollBarLine(line, QColor(color));
}

/*! Clear the highlight at the give line. */
void
SourcePanel::clearHighlightedLine(int line)
{
  // para == line when QTextEdit is in PlainText mode
  textEdit->setSelection(line, 0, line, textEdit->paragraphLength(line));
  textEdit->setColor( defaultColor );
}

/*! Highlight a segment in a give color. */
void
SourcePanel::highlightSegment(int from_para, int from_index, int to_para, int to_index, char *color)
{
  from_para--;
  to_para--;
  textEdit->setSelection(from_para, from_index, to_para, to_index);
  textEdit->setColor( color );

  // Annotate the scrollbar for this highlight....   (from_para == the line.)
  textEdit->annotateScrollBarLine(from_para, QColor(color));
}

/*! Clear the highlighted segment. */
void
SourcePanel::clearHighlightSegment(int from_para, int from_index, int to_para, int to_index )
{
  textEdit->setSelection(from_para, from_index, to_para, to_index);
  textEdit->setColor( defaultColor );
}

/*! Clears all user selections. */
void
SourcePanel::clearAllSelections()
{
  // Clears the black background box(es) on the screen.
  textEdit->selectAll(FALSE);
}

/*! Clear all highlights. */
void
SourcePanel::clearAllHighlights()
{
  textEdit->selectAll(TRUE);
  textEdit->setColor(defaultColor);
  textEdit->selectAll(FALSE);
}

/*! Clear the highlight list. */
void
SourcePanel::clearHighlightList()
{
  HighlightObject *hlo = NULL;
  for( HighlightList::Iterator it = highlightList->begin();
       it != highlightList->end();
       ++it)
  {
    hlo = (HighlightObject *)*it;
    delete hlo;
  }
  delete highlightList;
  highlightList = NULL;
}

/*! Center the line in the display. */
void
SourcePanel::positionLineAtCenter(int center_line)
{
  nprintf(DEBUG_PANELS) ("positionLineAtCenter(%d)\n", center_line);

  int heightForWidth = textEdit->heightForWidth(80);
  float lineHeight = (heightForWidth-hscrollbar->height())/(float)lineCount;
  int height = textEdit->height();
  nprintf(DEBUG_PANELS) ("height=%d visibleLines=%f\n", height, height/lineHeight );
  int visibleLines = (int)(height/lineHeight);
  nprintf(DEBUG_PANELS) ("visibleLines=%d page=%d\n", visibleLines, vscrollbar->pageStep() );

   int top_line = center_line - (visibleLines/2);
   if( top_line < 1 )
   {
     top_line = 1;
   }

   positionLineAtTop(top_line);
}

/*! Position the line at the top of the display. */
void
SourcePanel::positionLineAtTop(int top_line)
{

  lastTop = top_line;
  nprintf(DEBUG_PANELS) ("positionLineAtTop(top_line=%d)\n", top_line);
  top_line--; // We subtract 1 as textEdit is 0 based.
  // Clears the black background box(es) on the screen.
  int heightForWidth = textEdit->heightForWidth(80);
  float lineHeight = (heightForWidth-hscrollbar->height())/(float)lineCount;
  int value = (int) top_line * (int)lineHeight;
 
  nprintf(DEBUG_PANELS) ("So I think the value is %d\n", value);
  vscrollbar->setValue(value);

  // This forces a screen position.   Otherwise, some reposition, when the 
  // screen was not yet realized, were not being reposition at all.
  textEdit->setCursorPosition(top_line, 0);
}

/*! If the position is within a highlight, return true. */
int
SourcePanel::whatIsAtPos(const QPoint &pos)
{
  // remember, lines (para) and characters are 0 based.
  nprintf(DEBUG_PANELS) ("SourcePanel::whatIsAtPos() length=%d\n", textEdit->length() );

  if( textEdit->length() == 0 )
  {
    return 0;
  }
  int para = 0;
  int line = textEdit->paragraphAt(pos);
// pos is 0 base, we bump line by one to get it sync'd with file line numbers.
  line++;
  int c = textEdit->charAt(pos, &para);

  nprintf(DEBUG_PANELS) ("whatIsAtPos() line=%d para=%d c=(%d)\n", line, para, c );
  if( !highlightList )
  {
    return(0);
  }

  HighlightObject *hlo = NULL;
  for( HighlightList::Iterator it = highlightList->begin();
       it != highlightList->end();
       ++it)
  {
    hlo = (HighlightObject *)*it;
    if( hlo->line == line )
    {
      nprintf(DEBUG_PANELS) ("We have data at that line!!!\n");
      return( hlo->line );
    }
  }

  return(0);  // Return nothing highlighted.
}

/*! The user clicked.  -unused. */
void
SourcePanel::clicked(int para, int offset)
{
  nprintf(DEBUG_PANELS) ("You clicked?\n");
}

/*! The value changed... That means we've scrolled.   Recalculate the
    top line (top_line) and set the lastTop. */
void
SourcePanel::valueChanged()
{
// This is not correct, but it's gets close enough for right now.  FIX
  nprintf(DEBUG_PANELS) ("Your valueChanged.\n");
  float minValue = (float)vscrollbar->minValue();
  float maxValue = (float)vscrollbar->maxValue();
  float value = (float)vscrollbar->value();
  int lineStep = vscrollbar->lineStep();
  int height = textEdit->height();
  int heightForWidth = textEdit->heightForWidth(80);
  int pointSize = textEdit->pointSize();
  float lineHeight = (heightForWidth-vscrollbar->width())/(float)lineCount;


  nprintf(DEBUG_PANELS) ("heightForWidth = %d pointSize=%d\n", heightForWidth, pointSize );


  nprintf(DEBUG_PANELS) ("height=%d visibleLines=%f page=%d\n", height, height/lineHeight, vscrollbar->pageStep() );


  nprintf(DEBUG_PANELS) ("minValue=%f maxValue=%f value=%f\n", minValue, maxValue, value);
  nprintf(DEBUG_PANELS) ("lineStep=%d lineHeight=%f lineCount=%d\n",
    lineStep, lineHeight, lineCount);

  int top_line = (int)(value/lineHeight);
  top_line++;
  lastTop = top_line;
  nprintf(DEBUG_PANELS) ("top_line =%d\n", top_line);
}

/*! If there's a highlight list.... highlight the lines. */
void
SourcePanel::doFileHighlights()
{
  // Begin for demos and testing (for now)...   FIX
  if( !highlightList || highlightList->empty() )
  {
    return;
  }

  HighlightObject *hlo = NULL;
  textEdit->setUpdatesEnabled( FALSE );
  for( HighlightList::Iterator it = highlightList->begin();
       it != highlightList->end();
       ++it)
  {
    hlo = (HighlightObject *)*it;
    highlightLine(hlo->line, hlo->color, TRUE);
//    highlightSegment(para, index, para, index+4, "yellow");
  }
  // End for demos and testing (for now)...   FIX


  // Don't forget to turn the refreshing (for resize etc) back on...
  textEdit->setUpdatesEnabled( TRUE );
}

/*! If theres a description field, return it. */
char *
SourcePanel::getDescription(int line)
{
  HighlightObject *hlo = NULL;
  for( HighlightList::Iterator it = highlightList->begin();
       it != highlightList->end();
       ++it)
  {
    hlo = (HighlightObject *)*it;
    if( hlo->line == line )
    {
      return( hlo->description );
    }
  }

  return(0);  // Return nothing highlighted.
}

void SourcePanel::handleSizeEvent(QResizeEvent *e)
{
  nprintf(DEBUG_PANELS) ("entered\n");
//  Panel::handleSizeEvent(e);
  textEdit->clearScrollBar();
  doFileHighlights();
}
