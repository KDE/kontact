/*
  This file is part of KDE Kontact.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "kcmkontact.h"
using namespace Qt::Literals::StringLiterals;

#include "prefs.h"
using namespace Kontact;

#include <KontactInterface/Plugin>

#include <KLocalizedString>
#include <KPluginFactory>
#include <QComboBox>

#include <QCheckBox>
#include <QHBoxLayout>
#include <QStandardItemModel>

K_PLUGIN_CLASS_WITH_JSON(KcmKontact, "data/kontactconfig.json")
KcmKontact::KcmKontact(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
    , mPluginCombo(new QComboBox(widget()))
{
    auto topLayout = new QVBoxLayout(widget());
    auto pluginStartupLayout = new QHBoxLayout();
    topLayout->addLayout(pluginStartupLayout);
    auto forceStartupPluginCheckBox = new QCheckBox(Prefs::self()->forceStartupPluginItem()->label(), widget());
    forceStartupPluginCheckBox->setObjectName("kcfg_ForceStartupPlugin"_L1);
    pluginStartupLayout->addWidget(forceStartupPluginCheckBox);

    mPluginCombo->setToolTip(i18nc("@info:tooltip", "Select the initial plugin to use on each start"));
    mPluginCombo->setWhatsThis(i18nc("@info:whatsthis",
                                     "Select the plugin from this drop down list to be used as the "
                                     "initial plugin each time Kontact is started. Otherwise, Kontact "
                                     "will restore the last active plugin from the previous usage."));
    connect(mPluginCombo, &QComboBox::currentIndexChanged, this, [this]() {
        markAsChanged();
    });
    pluginStartupLayout->addWidget(mPluginCombo);
    pluginStartupLayout->addStretch();
    mPluginCombo->setEnabled(false);

    connect(forceStartupPluginCheckBox, &QAbstractButton::toggled, mPluginCombo, &QWidget::setEnabled);
    auto showSideBarCheckbox = new QCheckBox(Prefs::self()->sideBarOpenItem()->label(), widget());
    showSideBarCheckbox->setObjectName("kcfg_SideBarOpen"_L1);
    topLayout->addWidget(showSideBarCheckbox);

    addConfig(Prefs::self(), widget());
    topLayout->addStretch();
    load();
}

void KcmKontact::load()
{
    const KConfigGroup grp(Prefs::self()->config(), u"Plugins"_s);
    const QList<KPluginMetaData> pluginMetaDatas = KPluginMetaData::findPlugins(u"pim6/kontact"_s, [](const KPluginMetaData &data) {
        return data.rawData().value(u"X-KDE-KontactPluginVersion"_s).toInt() == KONTACT_PLUGIN_VERSION;
    });

    int activeComponent = 0;
    mPluginCombo->clear();
    mPluginList.clear();
    for (const KPluginMetaData &plugin : pluginMetaDatas) {
        // skip summary only plugins
        if (plugin.rawData().contains("X-KDE-KontactPluginHasPart"_L1)) {
            bool var = plugin.rawData().value(u"X-KDE-KontactPluginHasPart"_s).toBool();

            if (!var) {
                continue;
            }
        }

        mPluginCombo->addItem(plugin.name());
        mPluginList.append(plugin);

        // skip disabled plugins
        const QString pluginName = plugin.pluginId();
        if (!grp.readEntry(pluginName + u"Enabled"_s, true)) {
            const QStandardItemModel *qsm = qobject_cast<QStandardItemModel *>(mPluginCombo->model());
            if (qsm) {
                qsm->item(mPluginCombo->count() - 1, 0)->setEnabled(false);
            }
        }
        if (pluginName == Prefs::self()->activePlugin()) {
            activeComponent = mPluginList.count() - 1;
        }
    }

    mPluginCombo->setCurrentIndex(activeComponent);
    KCModule::load();
}

void KcmKontact::save()
{
    const KPluginMetaData plugin = mPluginList.at(mPluginCombo->currentIndex());
    Prefs::self()->setActivePlugin(plugin.pluginId());
    Prefs::self()->setForcedStartupPlugin(plugin.pluginId());
    KCModule::save();
}

#include "kcmkontact.moc"

#include "moc_kcmkontact.cpp"
