/*
  This file is part of the KDE Kontact.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2008 Rafael Fernández López <ereslibre@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "sidepanebase.h"
using namespace Kontact;

#include <KontactInterface/Core>
#include <QVBoxLayout>

SidePaneBase::SidePaneBase(KontactInterface::Core *core, QWidget *parent)
    : QWidget(parent)
    , mCore(core)
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins({});
}

SidePaneBase::~SidePaneBase()
{
}

KontactInterface::Core *SidePaneBase::core() const
{
    return mCore;
}
