/*
#| sepiola - Open Source Online Backup Client
#| Copyright (C) 2007-2017 stepping stone GmbH
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

#ifndef DUMMY_SNAPSHOT_HH_
#define DUMMY_SNAPSHOT_HH_

#include "abstract_snapshot.hh"

/**
 * The dummy snapshot class provides an empty snapshot method
 * @author Pat Kläy, pat.klaey@stepping-stone.ch
 */
class DummySnapshot : public AbstractSnapshot
{
    Q_OBJECT

public:

    /**
     * Creates the DummySnapshot object
     */
    DummySnapshot();

    /**
     * Destroys the DummySnapshot
     */
    virtual ~DummySnapshot();

    const SnapshotMapper& getSnapshotPathMappers() const;

private:
    SnapshotMapper snapshotPathMappers;

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

#endif /* DUMMY_SNAPSHOT_HH_ */
