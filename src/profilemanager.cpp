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

#include "profilemanager.h"

#include <kio/job.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>
#include <kurl.h>

#include <qdir.h>
#include <qstringlist.h>
#include <qvaluelist.h>

Kontact::Profile::Profile( const QString& id, bool isLocal ) : m_id( id ), m_local( isLocal )
{
}

Kontact::Profile::Profile() : m_local( false )
{
}

QString Kontact::Profile::id() const
{
    return m_id;
}

QString Kontact::Profile::name() const
{
    return m_name;
}

QString Kontact::Profile::description() const
{
    return m_description;
}

bool Kontact::Profile::isNull() const
{
    return m_id.isNull();
}

void Kontact::Profile::setId( const QString& id )
{
    m_id = id;
}

void Kontact::Profile::setDescription( const QString& description )
{
    m_description = description;
}

void Kontact::Profile::setName( const QString& name )
{
    m_name = name;
}

void Kontact::Profile::setLocal( SetLocalMode mode )
{
    if ( m_local )
        return;

    if ( mode == CopyProfileFiles )
        copyConfigFiles( m_originalLocation, localSaveLocation() );

    m_local = true;
}

bool Kontact::Profile::isLocal() const
{
    return m_local;
}

void Kontact::Profile::setOriginalLocation( const QString& path )
{
    m_originalLocation = path;
}

QString Kontact::Profile::localSaveLocation() const
{

    return  m_id.isNull() ? QString() : locateLocal( "data", "kontact/profiles/" + m_id, /*create folder=*/true );
}

QString Kontact::Profile::saveLocation() const
{
    return m_local ? localSaveLocation() : m_originalLocation;
}

bool Kontact::Profile::operator==( const Kontact::Profile& other ) const
{
    return m_id == other.m_id && m_name == other.m_name && m_description == other.m_description;
}

Kontact::ProfileManager* Kontact::ProfileManager::m_self = 0;

static KStaticDeleter<Kontact::ProfileManager> profileManagerSD;

Kontact::ProfileManager* Kontact::ProfileManager::self() 
{
    if ( m_self == 0 )
    {
        profileManagerSD.setObject( m_self, new Kontact::ProfileManager );
        m_self->readConfig();
    }
    return m_self;
}

Kontact::ProfileManager::ProfileManager( QObject* parent ) : QObject( parent )
{
}

Kontact::ProfileManager::~ProfileManager()
{
    writeConfig();
}

void Kontact::ProfileManager::writeConfig() const
{
    const QValueList<Kontact::Profile> profiles = m_profiles.values();
    for ( QValueList<Kontact::Profile>::ConstIterator it = profiles.begin(), end = profiles.end(); it != end; ++it )
    {
        writeProfileConfig( *it );
    }
}

Kontact::Profile Kontact::ProfileManager::readFromConfiguration( const QString& configFile, bool isLocal )
{
    KConfig profileCfg( configFile, true /*read-only*/, false /*no KDE global*/ );
    const QString configDir = configFile.left( configFile.findRev( QDir::separator(), -1 ) );
    profileCfg.setGroup( "Kontact Profile" );
    const QString id = profileCfg.readEntry( "Identifier" );
    Kontact::Profile profile( id );
    profile.setName( profileCfg.readEntry( "Name" ) );
    profile.setDescription( profileCfg.readEntry( "Description" ) );
    profile.setOriginalLocation( configDir );
    if ( isLocal )
        profile.setLocal( Kontact::Profile::DoNotCopyProfileFiles );
    return profile;
}

void Kontact::ProfileManager::writeProfileConfig( const Kontact::Profile& profile ) const
{
    const QString profileDir = profile.saveLocation();
    const QString cfgPath = profileDir + "/profile.cfg";
    KConfig profileCfg( cfgPath, false /*read-only*/, false /*no KDE global*/ );
    profileCfg.setGroup( "Kontact Profile" );
    profileCfg.writeEntry( "Identifier", profile.id() );
    profileCfg.writeEntry( "Name", profile.name() );
    profileCfg.writeEntry( "Description", profile.description() );
}

void Kontact::ProfileManager::readConfig()
{
    
    const QStringList profilePaths = KGlobal::dirs()->findAllResources( "data", QString::fromLatin1( "kontact/profiles/*/profile.cfg" ) );

    typedef QMap<QString, Kontact::Profile> ProfileMap;
    ProfileMap profiles;
    ProfileMap globalProfiles;

    const QString localPrefix = locateLocal( "data", "kontact/profiles/", /*createDir=*/false );
    for ( QStringList::ConstIterator it = profilePaths.begin(), end = profilePaths.end(); it != end; ++it )
    {
        const bool isLocal = (*it).startsWith( localPrefix );
        const Kontact::Profile profile = readFromConfiguration( *it, isLocal );
        if ( profile.isNull() )
            continue;
        if ( isLocal )
            profiles[profile.id()] = profile;
        else 
            globalProfiles[profile.id()] = profile;
    }
    
    for ( ProfileMap::ConstIterator it = globalProfiles.begin(), end = globalProfiles.end(); it != end; ++it )
    {
        if ( !profiles.contains( it.key() ) )
            profiles[it.key()] = it.data();
    }

    for ( ProfileMap::ConstIterator it = profiles.begin(), end = profiles.end(); it != end; ++it )
    {
        addProfile( *it, false /*dont sync config */ );
    }
}

QValueList<Kontact::Profile> Kontact::ProfileManager::profiles() const
{
    return m_profiles.values();
}

Kontact::Profile Kontact::ProfileManager::profileById( const QString& id ) const
{
    return m_profiles[id];
}

void Kontact::ProfileManager::updateProfile( const Kontact::Profile& profile_ )
{
    const QString id = profile_.id();
    if ( id.isNull() || m_profiles[id] == profile_ )
        return;
    Kontact::Profile profile( profile_ );
    m_profiles[id] = profile;
    profile.setLocal( Kontact::Profile::CopyProfileFiles );
    writeProfileConfig( profile );
    emit profileUpdated( id );
}

void Kontact::Profile::copyConfigFiles( const QString& source_, const QString& dest_ )
{
    const KURL source = KURL::fromPathOrURL( source_+"/*rc" );
    const KURL dest = KURL::fromPathOrURL( dest_ );
    KIO::CopyJob* job = KIO::copy( source, dest, /*showProgressInfo=*/false );
    // TODO better check for the copy result
}

void Kontact::ProfileManager::saveToProfile( const QString& id )
{
    Kontact::Profile profile = profileById( id );
    if ( profile.isNull() )
        return;
    profile.setLocal( Kontact::Profile::CopyProfileFiles );
    writeProfileConfig( profile );
    emit saveToProfileRequested( id );
}

bool Kontact::ProfileManager::addProfile( const Kontact::Profile& profile, bool syncConfig )
{
    const QString id = profile.id();
    if ( m_profiles.contains( id ) )
        return false;
    m_profiles[id] = profile;
    emit profileAdded( id );
    emit saveToProfileRequested( id );
    if ( syncConfig ) {
        writeProfileConfig( profile );
    }

    return true;
}

void Kontact::ProfileManager::loadProfile( const QString& id )
{
    if ( !m_profiles.contains( id ) )
        return;
    emit profileLoaded( id );
}

void Kontact::ProfileManager::removeProfile( const Kontact::Profile& profile )
{
    removeProfile( profile.id() );
}

void Kontact::ProfileManager::removeProfile( const QString& id )
{
    if ( !m_profiles.contains( id ) )
        return;
    Kontact::Profile profile = profileById( id );
    if ( profile.isLocal() ) {
        KURL location = KURL::fromPathOrURL( profile.saveLocation() );
        KIO::DeleteJob* job = KIO::del( location, /*shred*/ false, /*showProgressInfo=*/false );
        // TODO check result
    }
    m_profiles.remove( id );
    emit profileRemoved( id );
 }

Kontact::ProfileManager::ExportError Kontact::ProfileManager::exportProfileToDirectory( const QString& id, const QString& path )
{
    if ( !m_profiles.contains( id ) )
        return SuccessfulExport;

    if ( !QDir( path ).exists() )
        return DirectoryDoesNotExist;

    const Kontact::Profile profile = profileById( id );
    const KURL source = KURL::fromPathOrURL( profile.saveLocation() );
    const KURL target = KURL::fromPathOrURL( path + QDir::separator() + profile.name() );

    KIO::CopyJob* job = KIO::copy( source, target, /*showProgressInfo=*/false );
    // TODO check result

    return SuccessfulExport;
}

Kontact::ProfileManager::ImportError Kontact::ProfileManager::importProfileFromDirectory( const QString& path )
{
    Kontact::Profile profile = readFromConfiguration( path + "/profile.cfg", /*isLocal=*/ true );
    if ( profile.isNull() )
        return NoValidProfile;

    profile.setId( generateNewId() );

    const KURL source = KURL::fromPathOrURL( path );
    const KURL target = KURL::fromPathOrURL( profile.saveLocation() );

    KIO::CopyJob* job = KIO::copy( source, target, /*showProgressInfo=*/false );
    // TODO better check for the copy result

    addProfile( profile );

    return SuccessfulImport;
}

QString Kontact::ProfileManager::generateNewId() const
{
    while ( true )
    {
        const QString newId = KApplication::randomString( 10 );
        if ( !m_profiles.contains( newId ) )
            return newId;
    }
}

#include "profilemanager.moc"
