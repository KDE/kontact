/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KSETTINGS_DIALOG_H
#define KSETTINGS_DIALOG_H

#include "./kcmultidialog.h"

namespace KSettings
{
class DialogPrivate;

class Dialog : public KCMultiDialog
{
    friend class PageNode;
    Q_DECLARE_PRIVATE(Dialog)
    Q_OBJECT
public:
    explicit Dialog(QWidget *parent = nullptr);

    ~Dialog() override;

    /**
     * Adds a page with parentPluginMetaData used for title & icon of the component page and pluginMetaData for it's subpages
     */
    void addPluginComponent(const KPluginMetaData &parentPluginMetaData, const QVector<KPluginMetaData> &pluginMetaData);

protected:
    /**
     * Reimplemented to lazy create the dialog on first show.
     */
    void showEvent(QShowEvent *) override;

Q_SIGNALS:
    /**
     * If you use the dialog in Configurable mode and want to be notified
     * when the user changes the plugin selections use this signal. It's
     * emitted if the selection has changed and the user pressed Apply or
     * Ok. In the slot you would then load and unload the plugins as
     * requested.
     */
    void pluginSelectionChanged();
};

}

#endif // KSETTINGS_DIALOG_H
