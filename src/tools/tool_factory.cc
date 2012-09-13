/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2007-2012 stepping stone GmbH
#|
#| This program is free software; you can redistribute it and/or
#| modify it under the terms of the GNU General Public License
#| Version 2 as published by the Free Software Foundation.
#|
#| This program is distributed in the hope that it will be useful,
#| but WITHOUT ANY WARRANTY; without even the implied warranty of
#| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#| GNU General Public License for more details.
#|
#| You should have received a copy of the GNU General Public License
#| along with this program; if not, write to the Free Software
#| Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <QtCore/QtGlobal>

#ifdef Q_OS_WIN32
#include "tools/at.hh"
#include "tools/schtasks.hh"
#else
#include "tools/crontab.hh"
#endif

#include "tools/plink.hh"
#include "tools/rsync.hh"

#ifdef Q_OS_MAC
#include "tools/unix_permissions.hh"
#elif defined Q_OS_WIN32
#include "tools/set_acl.hh"
#else
#include "tools/posix_acl.hh"
#endif

#include "tools/tool_factory.hh"

ToolFactory::ToolFactory()
{
}

ToolFactory::~ToolFactory()
{
}

auto_ptr< AbstractMetadata > ToolFactory::getMetadataImpl()
{
#ifdef Q_OS_MAC
	return auto_ptr< AbstractMetadata >( new UnixPermissions );
#elif defined Q_OS_WIN32
	return auto_ptr< AbstractMetadata >( new SetAcl(Settings::getInstance()->getSetAclName()) );
#else
	return auto_ptr< AbstractMetadata >( new PosixAcl(Settings::getInstance()->getGetfaclName(), Settings::getInstance()->getGetfaclName()) );
#endif

}

auto_ptr< AbstractRsync > ToolFactory::getRsyncImpl()
{
	return auto_ptr< AbstractRsync >( new Rsync );
}

auto_ptr< AbstractSsh > ToolFactory::getSshImpl()
{
	return auto_ptr< AbstractSsh >( new Plink );
}

auto_ptr< AbstractScheduler > ToolFactory::getSchedulerImpl()
{
#ifdef Q_OS_WIN32
    if ( Schtasks::isSchtasksSupported() )
    {
        return auto_ptr< AbstractScheduler >( new Schtasks );
    }
    return auto_ptr< AbstractScheduler >( new At );
#else
	return auto_ptr< AbstractScheduler >( new Crontab );
#endif
}

