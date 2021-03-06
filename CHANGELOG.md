# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [2.7.1] - 2020-07-13

### Added

- Add missing libgcc1 for Windows 64
- utils/progress_task: add missing QDebug include

## [2.7.0] - 2020-07-08

### Added

- Add missing translation in backup thread
- Add bundled version of 'script' for greater platform independence
- Add .cmake-format.yaml config and reformat CMakeLists.txt files

### Changed

- Bring back shortcuts accidentally removed in commit e3d0d302bfb65b38d5ee317e859a4a0444e3128c
- Fix paths of uploaded files on Linux
- Fix warnings about missing slots during initial set-up
- Fix warnings of duplicate UI element namings
- Beautify .clang-format file and reformat code

## [2.6.1] - 2020-07-06

### Changed

- Fix translation of version in about dialog
- README: Fix the URL of the travis-ci.com badge

## [2.6.0] - 2020-07-05

### Added

- pre-commit: add minimal config

### Changed

- Update versions in INSTALL file
- Follow "Keep a Changelog" standard
- Ensure path on Linux is absolute
- Update bundled acl tools for Linux to 2.2.53
- Reformat all code based on new .clang-format config
- Fix settings form on Linux where snapshot is disabled
- Update bundled plink to 0.74
- Update bundled rsync to 3.1.2 and ssh to 8.2p1
- Add working console snippet for Windows
- Disconnect signals after finishing with backup
- Fix issues with user names containing spaces
- Mind '/cygdrive' prefix in set acl process
- Fix restore to original path by omitting dir times
- Fix destination check for restore process
- Disable save and discard button in settings form until form has been changed
- Merge duplicate translations
- Fix question formatting in Linux installer
- Enhance wording and translation in GUI
- Add hint about upload bandwidth limitation
- Default to 'English' if language is not explicity set
- Update copyright
- unix_permissions: fix compilation regression from snapshot introduction
- fix build failure with clang
- README: fix URL of travis-ci.com badge
- README: switch to travis-ci.com badge

## [2.5.0] - 2018-05-03

### Changed

- Windows: Made usage of Shadow Copy Service configurable.
- Windows: Fix several issues related with the update of `rsync` in 2.4.0.
- Windows:  Selecting, backing up and restoring to individual drives is now possible,
- ACLs are properly restored again.
- All Operating Systems: Corner case for detection of running Sepiola resolved.
- All Operating Systems: Translations for German updated.
- All Operating Systems: Raise minimal key length to 2048 bytes.
- All Operating Systems: Fix conversion of longer RSA host keys from Putty- to OpenSSH-format.
- All Operating Systems: Switch default backup host to `backup.stoney-backup.com`

## [2.4.0] - 2017-01-17

### Changed

- Update `rsync` and `ssh` to their latest version for Windows and Linux
  The bundled and modified version of `rsync` used to transfer the files was
  based on an older, patched version of `rsync`.

  By switching to a Cygwin-based `rsync` (and `ssh`) version for Windows,
  we can now provide regular updates for the two tools used to transfer
  and securing the backup transfer.

  This also solves interoperability issues causing connection failures
  during file transfer due to server and client protocol mismatch as well
  as a large number of issues fixed in `rsync` itself.

- Support files larger than 4 GiB on Windows

  Since the previous rsync version was a 32-bit application (and built without
  `LARGEFILE` support), we are finally able to handle files larger than 4 GiB on
  both Windows and Linux (for both 64- and 32-bit variants).

- Use the OpenSSH `ssh` client as a transport for Windows

  Previously, `plink` was used on Windows for securing the `rsync` transport.

- Separation of Sepiola SSH configuration

  Sepiola was previously using a users `known_hosts` file to store the
  server fingerprint. To avoid possible conflicts and a potential source
  of errors, we are now using a separate file on all platforms.

## [2.2.0] - 2013-12-13

### Changed

- Several bugs fixed for scheduling backup jobs on Microsoft Windows
- Update to Qt 4.8 and building on Linux
- Fixed installation of Qt-DLLs on Windows
- Fixed crash when quota is exceeded
- Improved error messages in several places
- Updated translation
- Several build system updates and fixes

[unreleased]: https://github.com/stepping-stone/sepiola/compare/v2.7.1...HEAD
[2.7.1]: https://github.com/stepping-stone/sepiola/compare/v2.7.0...v2.7.1
[2.7.0]: https://github.com/stepping-stone/sepiola/compare/v2.6.1...v2.7.0
[2.6.1]: https://github.com/stepping-stone/sepiola/compare/v2.6.0...v2.6.1
[2.6.0]: https://github.com/stepping-stone/sepiola/compare/v2.5.0...v2.6.0
[2.5.0]: https://github.com/stepping-stone/sepiola/compare/v2.4.0...v2.5.0
[2.4.0]: https://github.com/stepping-stone/sepiola/compare/v2.2.0...v2.4.0
[2.2.0]: https://github.com/stepping-stone/sepiola/compare/v2.1.2...v2.2.0
