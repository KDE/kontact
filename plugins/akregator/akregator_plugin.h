/***************************************************************************
 *     Copyright (C) 2004 by Sashmit Bhaduri                               *
 *     smt@vfemail.net                                                     *
 *                                                                         *
 *     Licensed under GPL.                                                 *
 ***************************************************************************/


#ifndef AKREGATOR_PLUGIN_H
#define AKREGATOR_PLUGIN_H

#include <klocale.h>
#include <kparts/part.h>

#include <kontact/plugin.h>

class KAboutData;

class aKregatorPlugin : public Kontact::Plugin
{
  Q_OBJECT

  public:
    aKregatorPlugin( Kontact::Core *core, const char *name,
                       const QStringList & );
    ~aKregatorPlugin();

    int weight() const { return 100; }

    virtual QStringList configModules() const;
  protected:
    KParts::Part *createPart();
};

#endif
