/*
  This file is part of the KDE Kontact.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2008 Rafael Fernández López <ereslibre@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KONTACT_SIDEPANEBASE_H
#define KONTACT_SIDEPANEBASE_H

#include <QWidget>

namespace KontactInterface {
class Core;
class Plugin;
}

namespace Kontact {
class SidePaneBase : public QWidget
{
    Q_OBJECT

public:
    SidePaneBase(KontactInterface::Core *core, QWidget *parent);
    virtual ~SidePaneBase();

    virtual void setCurrentPlugin(const QString &) = 0;

Q_SIGNALS:
    void pluginSelected(KontactInterface::Plugin *);

public Q_SLOTS:
    /**
      This method is called by the core whenever the count
      of plugins has changed.
     */
    virtual void updatePlugins() = 0;

protected:
    KontactInterface::Core *core() const;

private:
    KontactInterface::Core *const mCore;
};
}

#endif
