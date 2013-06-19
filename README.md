# What is Sepiola

![Sepiola Logo](http://www.sepiola.org/fileadmin/templates/images/logo.png)

Sepiola is a genus of bobtail squid comprising around 15 species or a platform independent Open Source Online Backup Client.

Thanks to the intuitive and simple graphical user interface, the backup of your precious data is a mere child's play.

[![Continuous Integration status](https://secure.travis-ci.org/stepping-stone/sepiola.png)](http://travis-ci.org/stepping-stone/sepiola)

## Characteristics

* supports Windows, Mac OS X and Linux
* encrypt data transfer with the help of Secure Shell (SSH)
* manual or automated backups (cronjob or scheduler)
* Access Control Lists (ACLs) are stored within the backup

## Secure Data Transfer

Sepiola uses Secure Shell (SSH) for the data transfer. This means, that all transferred data is encrypted. This way, chances of data espionage are slim.

## Public/Private Keys for a Password-free Login

To permit a password-free login, Sepiola creates a Secure Shell Public/Private key pair after the first start of the programme. The public key is copied onto the server. From now on, you will not receive a password request any more and your password is not stored any where on your system.


# Installation

Sepiola requires a set of open source tools to work, some patched some unpatched.
The actual requirements depend on the platform and are described in the INSTALL
file. The patches of the tools can be found in the patches subdirectory and the
original sources as follows:

* rsync: http://samba.org/ftp/rsync/src/
* putty: ftp://ftp.chiark.greenend.org.uk/users/sgtatham/
* openssh: http://www.openssh.com/portable.html
* getfacl/setfacl: ftp://oss.sgi.com/projects/xfs/cmd_tars/
* SetACL: http://sourceforge.net/project/showfiles.php?group_id=69165&package_id=68032


# License & Copyright

Copyright 2013 (c) stepping stone GmbH, Switzerland.

Released under the GPL-2 license.
