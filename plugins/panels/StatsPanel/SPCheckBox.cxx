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

#include <qapplication.h>
#include <qwidget.h>

#include "SPCheckBox.hxx"

class QCheckBox;

SPCheckBox::SPCheckBox( QWidget *parent, const char *name, SPCheckBoxList *list) : QCheckBox(parent,name)
{
  checkBoxList = list;
  if( list )
  {
    connect( this, SIGNAL( clicked() ), this, SLOT( toggleCallback() ) );
  }

}

SPCheckBox::~SPCheckBox()
{
}

void
SPCheckBox::toggleCallback()
{
  for( SPCheckBoxList::Iterator it = checkBoxList->begin();
       it != checkBoxList->end();
       it++ )
  {
    SPCheckBox *cb = (SPCheckBox *)*it;
    if( cb != this )
    {
      cb->setChecked(FALSE);
    }
  }

  setChecked(TRUE);
}

