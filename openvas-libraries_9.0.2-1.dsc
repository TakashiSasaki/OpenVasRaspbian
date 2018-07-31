Format: 3.0 (quilt)
Source: openvas-libraries
Binary: libopenvas-dev, libopenvas-doc, libopenvas9, openvas-nasl
Architecture: any all
Version: 9.0.2-1
Maintainer: Debian Security Tools <team+pkg-security@tracker.debian.org>
Uploaders: ChangZhuo Chen (陳昌倬) <czchen@debian.org>, SZ Lin (林上智) <szlin@debian.org>
Homepage: http://www.openvas.org/
Standards-Version: 4.1.4
Vcs-Browser: https://salsa.debian.org/pkg-security-team/openvas-libraries
Vcs-Git: https://salsa.debian.org/pkg-security-team/openvas-libraries.git
Build-Depends: debhelper (>= 10), bison, cmake, libgcrypt-dev, libglib2.0-dev, libgnutls28-dev, libgpgme-dev, libhiredis-dev, libksba-dev, libldap2-dev, libpcap-dev, libssh-dev, uuid-dev, libsnmp-dev, libradcli-dev
Build-Depends-Indep: doxygen
Package-List:
 libopenvas-dev deb libdevel optional arch=any
 libopenvas-doc deb doc optional arch=all
 libopenvas9 deb libs optional arch=any
 openvas-nasl deb net optional arch=any
Checksums-Sha1:
 3343cb79876065fe74d6328c04ebe63848bf427f 641967 openvas-libraries_9.0.2.orig.tar.gz
 8da79ee9d792f2c250a618a2c4a85ca1d1be4ee0 15004 openvas-libraries_9.0.2-1.debian.tar.xz
Checksums-Sha256:
 580ee80e39814fa0a718fb652cb97ff46548409cc998cad706462094fe52bc79 641967 openvas-libraries_9.0.2.orig.tar.gz
 ed5e3e251dfab354beb02b70e24c5f13310cbd743c852f020d8adfc130e94c35 15004 openvas-libraries_9.0.2-1.debian.tar.xz
Files:
 e6064270d3fdbf649eb7ba96b15848ee 641967 openvas-libraries_9.0.2.orig.tar.gz
 869164982e52190e779dd09315cbe2cd 15004 openvas-libraries_9.0.2-1.debian.tar.xz
