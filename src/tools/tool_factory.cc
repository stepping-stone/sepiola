/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2007, 2008  stepping stone GmbH
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

#include <QDebug>

#include "tools/at.hh"
#include "tools/schtasks.hh"
#include "tools/crontab.hh"
#include "tools/plink.hh"
#include "tools/posix_acl.hh"
#include "tools/rsync.hh"
#include "tools/set_acl.hh"
#include "tools/unix_permissions.hh"
#include "tools/tool_factory.hh"

ToolFactory::ToolFactory()
{
}

ToolFactory::~ToolFactory()
{
}

auto_ptr< AbstractMetadata > ToolFactory::getMetadataImpl()
{
	if ( Settings::IS_MAC )
	{
		return auto_ptr< AbstractMetadata >( new UnixPermissions );
	}
	if ( Settings::IS_WINDOWS )
	{
		return auto_ptr< AbstractMetadata >( new SetAcl );
	}
	return auto_ptr< AbstractMetadata >( new PosixAcl );
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
	if ( Settings::IS_WINDOWS )
	{
		if ( Schtasks::isSchtasksSupported() )
		{
			return auto_ptr< AbstractScheduler >( new Schtasks );
		}
		return auto_ptr< AbstractScheduler >( new At );
	}
	return auto_ptr< AbstractScheduler >( new Crontab );
}

