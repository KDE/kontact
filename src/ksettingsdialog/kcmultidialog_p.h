/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KCMULTIDIALOG_P_H
#define KCMULTIDIALOG_P_H

#include "kcmultidialog.h"
#include <QList>
#include <QStringList>

class KCModuleProxy;
class KPageWidgetItem;

class KCMultiDialogPrivate
{
    Q_DECLARE_PUBLIC(KCMultiDialog)
protected:
    KCMultiDialogPrivate(KCMultiDialog *parent)
        : currentModule(nullptr)
        , q_ptr(parent)
    {
    }

    virtual ~KCMultiDialogPrivate()
    {
    }

    KCModuleProxy *currentModule;

    struct CreatedModule {
        KCModuleProxy *kcm;
        KPageWidgetItem *item;
        QStringList componentNames;
    };

    typedef QList<CreatedModule> ModuleList;
    ModuleList modules;

    void _k_slotCurrentPageChanged(KPageWidgetItem *current, KPageWidgetItem *previous);
    virtual void _k_clientChanged();
    void _k_dialogClosed();
    void _k_updateHeader(bool use, const QString &message);

    KCMultiDialog *q_ptr;

private:
    void init();
    void apply();
    bool resolveChanges(KCModuleProxy *currentProxy);
    bool moduleSave(KCModuleProxy *module);
};

#endif // KCMULTIDIALOG_P_H
