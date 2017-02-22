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

    const SnapshotMapper& getSnapshotPathMappers();

    void setSnapshotPathMappers(
            const SnapshotMapper& snapshotPathMappers);

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
