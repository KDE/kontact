/*
  This file is part of KDE Kontact.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "kcmkontact.h"
#include "prefs.h"
using namespace Kontact;

#include <KontactInterface/Plugin>

#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QComboBox>

#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QStandardItemModel>

K_PLUGIN_CLASS_WITH_JSON(KcmKontact, "data/kontactconfig.json")
KcmKontact::KcmKontact(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , mPluginCombo(new QComboBox(parent))
{
    auto topLayout = new QVBoxLayout(this);
    auto pluginStartupLayout = new QHBoxLayout();
    topLayout->addLayout(pluginStartupLayout);

    auto forceStartupPluginCheckBox = new QCheckBox(Prefs::self()->forceStartupPluginItem()->label(), this);
    forceStartupPluginCheckBox->setObjectName(QStringLiteral("kcfg_ForceStartupPlugin"));
    pluginStartupLayout->addWidget(forceStartupPluginCheckBox);

    mPluginCombo->setToolTip(i18nc("@info:tooltip", "Select the initial plugin to use on each start"));
    mPluginCombo->setWhatsThis(i18nc("@info:whatsthis",
                                     "Select the plugin from this drop down list to be used as the "
                                     "initial plugin each time Kontact is started. Otherwise, Kontact "
                                     "will restore the last active plugin from the previous usage."));
    connect(mPluginCombo, &QComboBox::currentIndexChanged, this, [this]() {
        Q_EMIT changed(true);
    });
    pluginStartupLayout->addWidget(mPluginCombo);
    pluginStartupLayout->addStretch();
    mPluginCombo->setEnabled(false);

    connect(forceStartupPluginCheckBox, &QAbstractButton::toggled, mPluginCombo, &QWidget::setEnabled);

    auto showSideBarCheckbox = new QCheckBox(Prefs::self()->sideBarOpenItem()->label(), this);
    showSideBarCheckbox->setObjectName(QStringLiteral("kcfg_SideBarOpen"));
    topLayout->addWidget(showSideBarCheckbox);

    addConfig(Prefs::self(), this);
    topLayout->addStretch();
    load();
}

void KcmKontact::load()
{
    const KConfigGroup grp(Prefs::self()->config(), "Plugins");
    const QVector<KPluginMetaData> pluginMetaDatas =
        KPluginMetaData::findPlugins(QStringLiteral("kontact" QT_STRINGIFY(QT_VERSION_MAJOR)), [](const KPluginMetaData &data) {
            return data.rawData().value(QStringLiteral("X-KDE-KontactPluginVersion")).toInt() == KONTACT_PLUGIN_VERSION;
        });

    int activeComponent = 0;
    mPluginCombo->clear();
    mPluginList.clear();
    for (const KPluginMetaData &plugin : pluginMetaDatas) {
        // skip summary only plugins
        if (plugin.rawData().contains(QLatin1String("X-KDE-KontactPluginHasPart"))) {
            bool var = plugin.rawData().value(QStringLiteral("X-KDE-KontactPluginHasPart")).toBool();

            if (!var) {
                continue;
            }
        }

        mPluginCombo->addItem(plugin.name());
        mPluginList.append(plugin);

        // skip disabled plugins
        const QString pluginName = plugin.pluginId();
        if (!grp.readEntry(pluginName + QStringLiteral("Enabled"), true)) {
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
