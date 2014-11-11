/*
  This file is part of KDE Kontact.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "kcmkontact.h"
#include "prefs.h"
using namespace Kontact;

#include <KontactInterface/Plugin>

#include <KAboutData>
#include <KComboBox>
#include <KServiceTypeTrader>
#include <KLocalizedString>

#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

extern "C"
{
    Q_DECL_EXPORT KCModule *create_kontactconfig(QWidget *parent, const char *)
    {
        return new KcmKontact(parent);
    }
}

KcmKontact::KcmKontact(QWidget *parent)
    : KPrefsModule(Prefs::self(), parent)
{
    QBoxLayout *topLayout = new QVBoxLayout(this);
    QBoxLayout *pluginStartupLayout = new QHBoxLayout();
    topLayout->addItem(pluginStartupLayout);
    topLayout->addStretch();

    KPrefsWidBool *forceStartupPlugin =
        addWidBool(Prefs::self()->forceStartupPluginItem(), this);
    pluginStartupLayout->addWidget(forceStartupPlugin->checkBox());

    PluginSelection *selection =
        new PluginSelection(Prefs::self()->forcedStartupPluginItem(), this);
    addWid(selection);

    pluginStartupLayout->addWidget(selection->comboBox());
    selection->comboBox()->setEnabled(false);

    pluginStartupLayout->addStretch(1);

    connect(forceStartupPlugin->checkBox(), SIGNAL(toggled(bool)),
            selection->comboBox(), SLOT(setEnabled(bool)));
    load();
}

const KAboutData *KcmKontact::aboutData() const
{
    KAboutData *about = new KAboutData(
        QLatin1String("kontactconfig"),
        i18nc("@title", "KDE Kontact"),
        QString(),
        QString(),
        KAboutLicense::GPL,
        i18nc("@info:credit", "(c), 2003 Cornelius Schumacher"));

    about->addAuthor(i18nc("@info:credit", "Cornelius Schumacher"),
                     i18nc("@info:credit", "Developer"),
                     QLatin1String("schumacher@kde.org"));
    about->addAuthor(i18nc("@info:credit", "Tobias Koenig"),
                     i18nc("@info:credit", "Developer"),
                     QLatin1String("tokoe@kde.org"));

    return about;
}

PluginSelection::PluginSelection(KConfigSkeleton::ItemString *item, QWidget *parent)
{
    mItem = item;
    mPluginCombo = new KComboBox(parent);
    mPluginCombo->setToolTip(
        i18nc("@info:tooltip", "Select the initial plugin to use on each start"));
    mPluginCombo->setWhatsThis(
        i18nc("@info:whatsthis",
              "Select the plugin from this drop down list to be used as the "
              "initial plugin each time Kontact is started. Otherwise, Kontact "
              "will restore the last active plugin from the previous usage."));
    connect(mPluginCombo, static_cast<void (KComboBox::*)(int)>(&KComboBox::currentIndexChanged), this, &PluginSelection::changed);
}

PluginSelection::~PluginSelection()
{
}

void PluginSelection::readConfig()
{
    const KService::List offers = KServiceTypeTrader::self()->query(
                                      QString::fromLatin1("Kontact/Plugin"),
                                      QString::fromLatin1("[X-KDE-KontactPluginVersion] == %1").arg(KONTACT_PLUGIN_VERSION));

    int activeComponent = 0;
    mPluginCombo->clear();
    mPluginList.clear();
    KService::List::ConstIterator end(offers.end());
    for (KService::List::ConstIterator it = offers.begin(); it != end; ++it) {
        KService::Ptr service = *it;
        // skip summary only plugins
        QVariant var = service->property(QLatin1String("X-KDE-KontactPluginHasPart"));
        if (var.isValid() && var.toBool() == false) {
            continue;
        }
        mPluginCombo->addItem(service->name());
        mPluginList.append(service);

        if (service->property(QLatin1String("X-KDE-PluginInfo-Name")).toString() == mItem->value()) {
            activeComponent = mPluginList.count() - 1;
        }
    }

    mPluginCombo->setCurrentIndex(activeComponent);
}

void PluginSelection::writeConfig()
{
    KService::Ptr ptr =  mPluginList.at(mPluginCombo->currentIndex());
    mItem->setValue(ptr->property(QLatin1String("X-KDE-PluginInfo-Name")).toString());
}

QList<QWidget *> PluginSelection::widgets() const
{
    QList<QWidget *> widgets;
    widgets.append(mPluginCombo);

    return widgets;
}

