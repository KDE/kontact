/*
   SPDX-FileCopyrightText: 2012-2022 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "ksettingsdialog/kontactsettingsdialog.h"

namespace Kontact
{
class KontactConfigureDialog : public KontactSettingsDialog
{
    Q_OBJECT

public:
    explicit KontactConfigureDialog(QWidget *parent = nullptr);
    ~KontactConfigureDialog() override;

protected:
    QSize sizeHint() const override;

protected Q_SLOTS:
    /** reimplemented */
    void slotApply();

    /** reimplemented */
    void slotOk();
    void emitConfigChanged();
};
}

