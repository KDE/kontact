/*
  This file is part of KDE Kontact.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <KPageDialog>

namespace KontactInterface
{
class Core;
class Plugin;
}

class KAboutData;

namespace Kontact
{
class AboutDialog : public KPageDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(KontactInterface::Core *core);

protected:
    void addAboutPlugin(KontactInterface::Plugin *plugin);

    void addAboutData(const QString &title, const QString &icon, const KAboutData &about);

    void addLicenseText(const KAboutData &about);

    QString formatPerson(const QString &name, const QString &email);

private:
    void saveSize();
    KontactInterface::Core *const mCore;
};
}

