/*                                                                      
    This file is part of Kontact.                                  
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
                                                                        
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or   
    (at your option) any later version.                                 
                                                                        
    This program is distributed in the hope that it will be useful,     
    but WITHOUT ANY WARRANTY; without even the implied warranty of      
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the        
    GNU General Public License for more details.                        
                                                                        
    You should have received a copy of the GNU General Public License   
    along with this program; if not, write to the Free Software         
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#ifndef KCMKABSUMMARY_H
#define KCMKABSUMMARY_H

#include <kcmodule.h>

class QButtonGroup;
class QCheckBox;
class QSpinBox;

class KCMKABSummary : public KCModule
{
  Q_OBJECT

  public:
    KCMKABSummary( QWidget *parent = 0, const char *name = 0 );

    virtual void load();
    virtual void save();
    virtual void defaults();

  private slots:
    void modified();
    void buttonClicked( int );
    void customDaysChanged( int );

  private:
    void initGUI();

    QButtonGroup *mDaysGroup;
    QButtonGroup *mShowGroup;
    QCheckBox *mShowBirthdays;
    QCheckBox *mShowAnniversaries;
    QSpinBox *mCustomDays;
};

#endif
