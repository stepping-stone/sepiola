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

#ifndef DUMMY_WINDOWS_SNAPSHOT_HH
#define DUMMY_WINDOWS_SNAPSHOT_HH

#include "abstract_snapshot.hh"

#include <memory>

class OptionalSnapshot : public AbstractSnapshot
{
    Q_OBJECT

public:

    /**
     * Creates the OptionalSnapshot object
     */
    OptionalSnapshot(shared_ptr<AbstractSnapshot> ptr);

    /**
     * Destroys the OptionalSnapshot
     */
    virtual ~OptionalSnapshot();

    const SnapshotMapper& getSnapshotPathMappers() const;

    /**
     * Checks if there is something to clean up
     */
    virtual void checkCleanup();

private:
    SnapshotMapper _snapshotPathMappers;
    shared_ptr<AbstractSnapshot> _snapshot;

public slots:

    /**
     * Creates a new snapshot object
     */
    void createSnapshotObject();

    /**
     * Initializes the snapshot object
     */
    void initializeSnapshot();

    /**
     * Adds all the given files to the snapshot selection
     * @param The BackupSelectionHash which defines all files which are later
     * backed-up
     */
    void addFilesToSnapshot( const BackupSelectionHash includeRules );

    /**
     * Executes the snapshot
     */
    void takeSnapshot();

    /**
     * Cleans up the snapshot
     */
    void cleanupSnapshot();
};

#endif // DUMMY_WINDOWS_SNAPSHOT_HH
