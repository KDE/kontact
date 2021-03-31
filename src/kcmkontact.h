/*
  This file is part of KDE Kontact.

  SPDX-FileCopyrightText: 2003 Cornelius Scumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <KCModule>
#include <KService>
class QComboBox;
class QCheckBox;
namespace Kontact
{
class KcmKontact : public KCModule
{
    Q_OBJECT

public:
    explicit KcmKontact(QWidget *parent = nullptr);

    void load() override;
    void save() override;

    const KAboutData *aboutData() const override;

private:
    KService::List mPluginList;
    QComboBox *mPluginCombo = nullptr;
    QCheckBox *mShowSideBarCheckbox = nullptr;
};
}

