/*
   SPDX-FileCopyrightText: 2012-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KONTACTCONFIGUREDIALOG_H
#define KONTACTCONFIGUREDIALOG_H

#include <ksettings/Dialog>

namespace Kontact {
class KontactConfigureDialog : public KSettings::Dialog
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

#endif /* KONTACTCONFIGUREDIALOG_H */
