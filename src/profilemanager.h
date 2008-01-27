/*
    This file is part of KDE Kontact.

    Copyright (c) 2007 Frank Osterfeld <frank.osterfeld@kdemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KONTACT_PROFILEMANAGER_H
#define KONTACT_PROFILEMANAGER_H

#include <qmap.h>
#include <qobject.h>
#include <qstring.h>

template <class T> class QValueList;

namespace KIO {
    class Job;
}

namespace Kontact {

class Profile
{
    friend class ProfileManager;
public:
    Profile();

    explicit Profile( const QString& id, bool isLocal = false );

    QString id() const;

    QString name() const;

    QString description() const;

    bool isNull() const;

    void setName( const QString& name );

    void setDescription( const QString& description );

    bool operator==( const Kontact::Profile& other ) const;

    QString saveLocation() const;

private: // ProfileManager only

    enum SetLocalMode {
        DoNotCopyProfileFiles,
        CopyProfileFiles
    };
    void setLocal( SetLocalMode mode );
    bool isLocal() const;
    void setOriginalLocation( const QString& path );
    void setId( const QString& id );

private:

    static void copyConfigFiles( const QString& source, const QString& dest );

    QString localSaveLocation() const;

private:
    QString m_id;
    QString m_name;
    QString m_description;
    bool m_local;
    QString m_originalLocation;
};

class ProfileManager : public QObject
{
Q_OBJECT
public:
    enum ImportError {
        SuccessfulImport=0,
        NoValidProfile
    };

    enum ExportError {
        SuccessfulExport=0,
        DirectoryDoesNotExist,
        DirectoryNotWritable
    };

    static ProfileManager* self();

    ~ProfileManager();

    Kontact::Profile profileById( const QString& id ) const;

    bool addProfile( const Kontact::Profile& profile, bool syncConfig = true );

    void removeProfile( const Kontact::Profile& profile );

    void removeProfile( const QString& id );

    void updateProfile( const Kontact::Profile& profile );

    void loadProfile( const QString& id );

    void saveToProfile( const QString& id );

    QValueList<Kontact::Profile> profiles() const;

    ExportError exportProfileToDirectory( const QString& id, const QString& path );

    ImportError importProfileFromDirectory( const QString& path );

    QString generateNewId() const;

signals:
    void profileAdded( const QString& id );

    void profileRemoved( const QString& id );

    void profileUpdated( const QString& id );

    void profileLoaded( const QString& id );

    void saveToProfileRequested( const QString& id );

    void profileImportFinished( ImportError status );

private:
    static ProfileManager* m_self;

    static Kontact::Profile readFromConfiguration( const QString& configFile, bool isLocal );

    explicit ProfileManager( QObject* parent = 0 );

    void readConfig();

    void writeConfig() const;

    void writeProfileConfig( const Kontact::Profile& profile ) const;

private:
    QMap<QString, Kontact::Profile> m_profiles;
};

}

#endif // KONTACT_PROFILEMANAGER_H
