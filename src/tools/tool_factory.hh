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

#ifndef TOOL_FACTORY_HH
#define TOOL_FACTORY_HH

#include <memory>

#include "settings/settings.hh"
#include "tools/abstract_metadata.hh"
#include "tools/abstract_rsync.hh"
#include "tools/abstract_ssh.hh"
#include "tools/abstract_scheduler.hh"

using std::auto_ptr;

/**
 * The ToolFactory class provides methods for constructing real implementation object
 * based on the running operating system
 * @author Bruno Santschi, santschi@puzzle.ch
 */
class ToolFactory
{
public:

	/**
	 * Constructs a ToolFactory
	 */
	ToolFactory();

	/**
	 * Destroys the ToolFactory
	 */
	virtual ~ToolFactory();

	/**
	 * Returns an AbstractAcl implementation
	 * @return an acl implementation
	 */
	static auto_ptr< AbstractMetadata > getMetadataImpl();

	/**
	 * Returns an AbstractRsync implementation
	 * @return an rsync implementation
	 */
	static auto_ptr< AbstractRsync > getRsyncImpl();

	/**
	 * Returns an AbstractSsh implementation
	 * @return a ssh implementation
	 */
	static auto_ptr< AbstractSsh > getSshImpl();

	/**
	 * Returns an AbstractScheduler implementation
	 * @return a scheduler implementation
	 */
	static auto_ptr< AbstractScheduler > getSchedulerImpl();
};

#endif
