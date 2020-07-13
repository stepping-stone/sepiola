# What is Sepiola

![Sepiola Logo](http://www.sepiola.org/fileadmin/templates/images/logo.png)

[Sepiola](http://www.sepiola.org) is a genus of bobtail squid comprising around 15 species or a platform independent Open Source Online Backup Client.

Thanks to the intuitive and simple graphical user interface, the backup of your precious data is a mere child's play.

[![Build Status](https://travis-ci.com/stepping-stone/sepiola.svg)](https://travis-ci.com/stepping-stone/sepiola)

## Characteristics

* Supports Windows, Mac OS X and Linux
* Encrypt data transfer with the help of Secure Shell (SSH)
* Manual or automated backups (cronjob or scheduler)
* Access Control Lists (ACLs) are stored within the backup

## Secure Data Transfer

Sepiola uses Secure Shell (SSH) for the data transfer. This means, that all transferred data is encrypted. This way, chances of data espionage are slim.

## Public/Private Keys for a Password-free Login

To permit a password-free login, Sepiola creates a Secure Shell Public/Private key pair after the first start of the programme. The public key is copied onto the server. From now on, you will not receive a password request any more and your password is not stored any where on your system.

# Third-party software components

Sepiola uses a set of open source tools, some patched some unpatched.
The actual requirements depend on the platform and are described in the INSTALL
file. The patches of the tools can be found in the patches subdirectory and the
original sources as follows:

* rsync: https://rsync.samba.org/
* putty: https://www.chiark.greenend.org.uk/~sgtatham/putty/
* script: https://mirrors.edge.kernel.org/pub/linux/utils/util-linux/
* openssh: https://www.openssh.com/
* getfacl/setfacl: https://savannah.nongnu.org/projects/acl/
* SetACL: http://sourceforge.net/projects/setacl/

# Development

The development follows the [git flow](http://nvie.com/posts/a-successful-git-branching-model/) model.

# License & Copyright

Copyright (c) 2007-2020 stepping stone AG, Switzerland.

Released under the GPL-2 license.
