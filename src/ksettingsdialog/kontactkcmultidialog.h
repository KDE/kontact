/*
    SPDX-FileCopyrightText: 2000 Matthias Elter <elter@kde.org>
    SPDX-FileCopyrightText: 2003 Daniel Molkentin <molkentin@kde.org>
    SPDX-FileCopyrightText: 2003, 2006 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QScrollArea>
#include <QScrollBar>

#include <KPageDialog>
#include <KPluginMetaData>

class KontactKCMultiDialogPrivate;

/**
 * @short A class that offers a KPageDialog containing arbitrary
 *        KControl Modules.
 *
 * @author Matthias Elter <elter@kde.org>, Daniel Molkentin <molkentin@kde.org>
 */
class KontactKCMultiDialog : public KPageDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(KontactKCMultiDialog)

public:
    explicit KontactKCMultiDialog(QWidget *parent = nullptr);
    ~KontactKCMultiDialog() override;

    /**
     * Add a module to the dialog. Its position will be determined based on the @c X-KDE-Weight value.
     * @param metaData KPluginMetaData that will be used to load the plugin
     * @since 5.84
     */
    KPageWidgetItem *addModule(const KPluginMetaData &metaData, KPageWidgetItem *parent = nullptr);

Q_SIGNALS:
    /**
     * Emitted after the KCModules have been told to save their configuration.
     * It is emitted once for every instance the KCMs that were changed belong
     * to.
     *
     * You can make use of this if you have more than one component in your
     * application. pluginId tells you the instance that has to reload its
     * configuration.
     *
     * The applyClicked and okClicked signals are emitted before the
     * configuration is saved.
     *
     * @param pluginId The name of the instance that needs to reload its
     *                     configuration.
     */
    void configCommitted(const QString &pluginId);

protected:
    /**
     * This constructor can be used by subclasses to provide a custom KPageWidget.
     */
    KontactKCMultiDialog(KontactKCMultiDialogPrivate &dd, KPageWidget *pageWidget, QWidget *parent, Qt::WindowFlags flags = Qt::WindowFlags());

    KontactKCMultiDialogPrivate *const d_ptr;

    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

protected Q_SLOTS:
    /**
     * This slot is called when the user presses the "Default" Button.
     * You can reimplement it if needed.
     *
     * @note Make sure you call the original implementation.
     **/
    void slotDefaultClicked();

    /**
     * This slot is called when the user presses the "Reset" Button.
     * You can reimplement it if needed.
     *
     * @note Make sure you call the original implementation.
     */
    void slotUser1Clicked();

    /**
     * This slot is called when the user presses the "Apply" Button.
     * You can reimplement it if needed.
     *
     * @note Make sure you call the original implementation.
     **/
    void slotApplyClicked();

    /**
     * This slot is called when the user presses the "OK" Button.
     * You can reimplement it if needed.
     *
     * @note Make sure you call the original implementation.
     **/
    void slotOkClicked();

    /**
     * This slot is called when the user presses the "Help" Button.
     * It reads the X-DocPath field of the currently selected KControl
     * module's .desktop file to find the path to the documentation,
     * which it then attempts to load.
     *
     * You can reimplement this slot if needed.
     *
     * @note Make sure you call the original implementation.
     **/
    void slotHelpClicked();

private:
    Q_PRIVATE_SLOT(d_func(), void _k_slotCurrentPageChanged(KPageWidgetItem *, KPageWidgetItem *))
    Q_PRIVATE_SLOT(d_func(), void _k_clientChanged())
};

/**
 * @brief Custom QScrollArea class that doesn't limit its size hint
 *
 * See original QScrollArea::sizeHint() function,
 * where the size hint is bound by 36*24 font heights
 *
 * Workaround for https://bugreports.qt.io/browse/QTBUG-10459
 */

class UnboundScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    QSize sizeHint() const override
    {
        if (widget()) {
            // Try to avoid horizontal scrollbar, which just scrolls a scrollbar width.
            // We always need to reserve space for the vertical scroll bar,
            // because we can’t know here whether vertical scrolling will be used.
            QSize withScrollbar = widget()->sizeHint();
            withScrollbar.rwidth() += verticalScrollBar()->sizeHint().width() + 4;
            return withScrollbar;
        } else {
            return QScrollArea::sizeHint();
        }
    }

    explicit UnboundScrollArea(QWidget *w)
        : QScrollArea(w)
    {
    }
    ~UnboundScrollArea() override = default;
};
