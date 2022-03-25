/*
  This file is part of the KDE Kontact.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2008 Rafael Fernández López <ereslibre@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <QWidget>

namespace KontactInterface
{
class Core;
class Plugin;
}

namespace Kontact
{
class SidePaneBase : public QWidget
{
    Q_OBJECT

public:
    explicit SidePaneBase(KontactInterface::Core *core, QWidget *parent);
    ~SidePaneBase() override;

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
