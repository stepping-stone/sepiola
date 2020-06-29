/*
#| sepiola - Open Source Online Backup Client
#| Copyright (c) 2007-2020 stepping stone AG
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

#include "settings/settings.hh"
#include "tools/abstract_metadata.hh"
#include "tools/abstract_rsync.hh"
#include "tools/abstract_ssh.hh"
#include "tools/abstract_scheduler.hh"
#include "tools/abstract_snapshot.hh"

#ifdef Q_OS_WIN32
#include "tools/at.hh"
#include "tools/schtasks.hh"
#include "tools/shadow_copy.hh"
#include "tools/optionalSnapshot.hh"
#else
#include "tools/crontab.hh"
#include "tools/dummy_snapshot.hh"
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

#include <memory>

ToolFactory::ToolFactory()
{
}

ToolFactory::~ToolFactory()
{
}

AbstractMetadata * ToolFactory::getMetadataImpl()
{
#ifdef Q_OS_MAC
	return new UnixPermissions;
#elif defined Q_OS_WIN32
	return new SetAcl(Settings::getInstance()->getSetAclName());
#else
	return new PosixAcl(Settings::getInstance()->getGetfaclName(), Settings::getInstance()->getSetfaclName());
#endif

}

AbstractRsync * ToolFactory::getRsyncImpl()
{
	return new Rsync;
}

AbstractSsh * ToolFactory::getSshImpl()
{
	return new Plink;
}

AbstractScheduler * ToolFactory::getSchedulerImpl()
{
#ifdef Q_OS_WIN32
    if ( Schtasks::isSchtasksSupported() )
    {
        return new Schtasks;
    }
    return new At;
#else
	return new Crontab;
#endif
}

AbstractSnapshot * ToolFactory::getSnapshotImpl()
{
#ifdef Q_OS_WIN32
    return new OptionalSnapshot(shared_ptr<AbstractSnapshot> (new ShadowCopy));
#else
    return new DummySnapshot;
#endif
}

