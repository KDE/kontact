/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KSETTINGS_DIALOG_H
#define KSETTINGS_DIALOG_H

#include "./kcmultidialog.h"

#include <KService>

template<class T>
class QList;

namespace KSettings
{
class DialogPrivate;

/**
 * @short Generic configuration dialog that works over component boundaries
 *
 * For more information see \ref KSettings.
 *
 * This class aims to standardize the use of configuration dialogs in applications.
 * Especially when using KParts and/or Plugins you face problems creating consistent
 * config dialogs.
 *
 * To show a configuration dialog you only have to call the show method and be
 * done with it. A code example:
 *
 * You initialize @c m_cfgdlg with
 * @code
 * m_cfgdlg = new Dialog(this);
 * @endcode
 *
 * If you use a KPart that was not especially designed for your app you can use
 * the second constructor:
 * @code
 * QStringList kpartslist;
 * for (all my kparts) {
 *   kpartslist += m_mypart->componentData().componentName();
 * }
 *
 * m_cfgdlg = new Dialog(kpartslist, this);
 * @endcode
 *
 * and ideally you can connect the "Configure MyApp" action to the config
 * dialog show() slot:
 * @code
 * KStandardAction::preferences(m_cfgdlg, &QDialog::show, actionCollection());
 * @endcode
 *
 * If you need to be informed when the config is changed by the dialog, you can
 * connect to the @c KCMultiDialog::configCommitted() signal (which emits the
 * component name as its argument):
 * @code
 * connect(m_cfgdlg, QOverload<const QByteArray &>::of(&KCMultiDialog::configCommitted), this, &Foo::slotConfigUpdated);
 * @endcode
 *
 * @see KSettings.
 *
 * @author Matthias Kretz <kretz@kde.org>
 */
class Dialog : public KCMultiDialog
{
    friend class PageNode;
    Q_DECLARE_PRIVATE(Dialog)
    Q_OBJECT
public:
    /**
     * Construct a new Preferences Dialog for the application. It uses all
     * KCMs with X-KDE-ParentApp set to QCoreApplication::instance()->applicationName().
     *
     * @param content      Select whether you want a static or configurable
     *                     config dialog.
     * @param parent       The parent is only used as the parent for the
     *                     dialog - centering the dialog over the parent
     *                     widget.
     */
    explicit Dialog(QWidget *parent = nullptr);

    /**
     * Construct a new Preferences Dialog with the pages for the selected
     * instance names. For example if you want to have the configuration
     * pages for the kviewviewer KPart you would pass a
     * QStringList consisting of only the name of the part "kviewviewer".
     *
     * @param components   A list of the names of the components that your
     *                     config dialog should merge the config pages in.
     * @param parent       The parent is only used as the parent for the
     *                     dialog - centering the dialog over the parent
     *                     widget.
     */
    explicit Dialog(const QStringList &components, QWidget *parent = nullptr);

    ~Dialog() override;

    /**
     * bla bla bla
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

private:
    // Q_PRIVATE_SLOT(d_func(), void _k_configureTree())
    Q_PRIVATE_SLOT(d_func(), void _k_updateEnabledState(bool))
};

}

#endif // KSETTINGS_DIALOG_H
