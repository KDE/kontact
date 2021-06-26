/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KSETTINGS_DIALOG_P_H
#define KSETTINGS_DIALOG_P_H

#include "dialog.h"
#include "kcmoduleinfo.h"
#include "kcmultidialog_p.h"

#include <QHash>
#include <QSet>
#include <QString>

#include <KPageWidgetModel>
#include <KPluginInfo>
#include <KService>

namespace KSettings
{
class DialogPrivate : public KCMultiDialogPrivate
{
    friend class PageNode;
    Q_DECLARE_PUBLIC(Dialog)
protected:
    DialogPrivate(Dialog *parent);

    QHash<KPageWidgetItem *, KPluginInfo> pluginForItem;

    QStringList registeredComponents;
    QSet<KPluginMetaData> kcmsMetaData;

    QList<QPair<KPluginMetaData, QVector<KPluginMetaData>>> componentsMetaData;

    QStringList arguments;
    QStringList components;

    bool firstshow : 1;
    quint32 pluginStateDirty : 30;

    // void _k_configureTree();
    void _k_updateEnabledState(bool);
    void updateConfiguration();
    void _k_clientChanged() override;

    KPageWidgetItem *createPageItem(KPageWidgetItem *parentItem, const QString &name, const QString &comment, const QString &iconName);

    // void connectItemCheckBox(KPageWidgetItem *item, const KPluginInfo &pinfo, bool isEnabled);

private:
    /**
     * @internal
     * Check whether the plugin associated with this KCM is enabled.
     */
    bool isPluginForKCMEnabled(const KCModuleInfo *moduleinfo, KPluginInfo &pinfo) const;

    QSet<KCModuleInfo> instanceServices();
    QSet<KCModuleInfo> parentComponentsServices(const QStringList &);

    /**
     * @internal
     * This method is called only once. The KCMultiDialog is not created
     * until it's really needed. So if some method needs to access d->dlg it
     * checks for 0 and if it's not created this method will do it.
     */
    void createDialogFromServices();
};

} // namespace KSettings
#endif // KSETTINGS_DIALOG_P_H
