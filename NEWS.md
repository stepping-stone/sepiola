# Sepiola Release Notes

## Version 2.5.0

* Windows: Made usage of Shadow Copy Service configurable.

* Windows: Fix several issues related with the update of `rsync` in 2.4.0.

* Windows:  Selecting, backing up and restoring to individual drives is now possible,
  ACLs are properly restored again.

* All Operating Systems: Corner case for detection of running Sepiola resolved.


## Version 2.4.0

* Update `rsync` and `ssh` to their latest version for Windows and Linux

  The bundled and modified version of `rsync` used to transfer the files was
  based on an older, patched version of `rsync`.

  By switching to a Cygwin-based `rsync` (and `ssh`) version for Windows,
  we can now provide regular updates for the two tools used to transfer
  and securing the backup transfer.

  This also solves interoperability issues causing connection failures
  during file transfer due to server and client protocol mismatch as well
  as a large number of issues fixed in `rsync` itself.

* Support files larger than 4 GiB on Windows

  Since the previous rsync version was a 32-bit application (and built without
  `LARGEFILE` support), we are finally able to handle files larger than 4 GiB on
  both Windows and Linux (for both 64- and 32-bit variants).

* Use the OpenSSH `ssh` client as a transport for Windows

  Previously, `plink` was used on Windows for securing the `rsync` transport.

* Separation of Sepiola SSH configuration

  Sepiola was previously using a users `known_hosts` file to store the
  server fingerprint. To avoid possible conflicts and a potential source
  of errors, we are now using a separate file on all platforms.


## Version 2.2.0

* Several bugs fixed for scheduling backup jobs on Microsoft Windows
* Update to Qt 4.8 and building on Linux
* Fixed installation of Qt-DLLs on Windows
* Fixed crash when quota is exceeded
* Improved error messages in several places
* Updated translation
* Several build system updates and fixes
