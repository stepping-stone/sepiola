/*
 * filesystem_snapshot_path_mapper.cc
 *
 *  Created on: Apr 30, 2014
 *      Author: pat
 */

#include "filesystem_snapshot_path_mapper.hh"

#include <QString>

FilesystemSnapshotPathMapper::FilesystemSnapshotPathMapper(const QString& _partition, const BackupSelectionHash& includeRules )
{
    this->partition = _partition;
    this->relativeIncludes = includeRules;
}

FilesystemSnapshotPathMapper::FilesystemSnapshotPathMapper()
{
}

FilesystemSnapshotPathMapper::~FilesystemSnapshotPathMapper()
{
}

const QString& FilesystemSnapshotPathMapper::getPartition() const
{
    return partition;
}

void FilesystemSnapshotPathMapper::setPartition(const QString& partition)
{
    this->partition = partition;
}

const BackupSelectionHash& FilesystemSnapshotPathMapper::getRelativeIncludes() const
{
    return relativeIncludes;
}

void FilesystemSnapshotPathMapper::setRelativeIncludes(const BackupSelectionHash& relativeIncludes)
{
    this->relativeIncludes = relativeIncludes;
}

const QString& FilesystemSnapshotPathMapper::getSnapshotPath() const
{
    return snapshotPath;
}

void FilesystemSnapshotPathMapper::setSnapshotPath(const QString& snapshotPath)
{
    this->snapshotPath = snapshotPath;
}

void FilesystemSnapshotPathMapper::addFileToRelativeIncludes(QString filename, bool backup)
{
    this->relativeIncludes.insert( filename, backup );
}
