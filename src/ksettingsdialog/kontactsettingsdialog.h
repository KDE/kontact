/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KSETTINGS_DIALOG_H
#define KSETTINGS_DIALOG_H

#include "kontactkcmultidialog.h"

class KontactSettingsDialogPrivate;

class KontactSettingsDialog : public KontactKCMultiDialog
{
    Q_DECLARE_PRIVATE(KontactSettingsDialog)
    Q_OBJECT
public:
    explicit KontactSettingsDialog(QWidget *parent = nullptr);

    ~KontactSettingsDialog() override;

    /**
     * Adds a page with parentPluginMetaData used for title & icon of the component page and pluginMetaData for it's subpages
     */
    void addPluginComponent(const KPluginMetaData &parentPluginMetaData, const QVector<KPluginMetaData> &pluginMetaData);

protected:
    /**
     * Reimplemented to lazy create the dialog on first show.
     */
    void showEvent(QShowEvent *) override;
};

#endif // KSETTINGS_DIALOG_H
