/*
  This file is part of KDE Kontact.

  SPDX-FileCopyrightText: 2003 Cornelius Scumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once
#include "kcmutils_version.h"
#include <KCModule>
#include <KPluginMetaData>

class QComboBox;
namespace Kontact
{
class KcmKontact : public KCModule
{
    Q_OBJECT

public:
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    explicit KcmKontact(QWidget *parent, const QVariantList &args);
#else
    explicit KcmKontact(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
#endif

    void load() override;
    void save() override;

private:
    QVector<KPluginMetaData> mPluginList;
    QComboBox *const mPluginCombo;
};
}
