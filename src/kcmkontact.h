/*
  This file is part of KDE Kontact.

  SPDX-FileCopyrightText: 2003 Cornelius Scumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once
#include <KCModule>
#include <KPluginMetaData>

class QComboBox;
namespace Kontact
{
class KcmKontact : public KCModule
{
    Q_OBJECT

public:
    explicit KcmKontact(QObject *parent, const KPluginMetaData &data);

    void load() override;
    void save() override;

private:
    QList<KPluginMetaData> mPluginList;
    QComboBox *const mPluginCombo;
};
}
