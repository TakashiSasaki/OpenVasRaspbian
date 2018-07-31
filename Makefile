pkggsacommon=greenbone-security-assistant-common
debgsacommon=greenbone-security-assistant-common_7.0.3+dfsg.1-1_all.deb
pkggsasym=greenbone-security-assistant-dbgsym
debgsasym=greenbone-security-assistant-dbgsym_7.0.3+dfsg.1-1_armhf.deb
pkggsa=greenbone-security-assistant
debgsa=greenbone-security-assistant_7.0.3+dfsg.1-1_armhf.deb
pkglibdev=libopenvas-dev
deblibdev=libopenvas-dev_9.0.2-1_armhf.deb
pkglibdoc=libopenvas-doc
deblibdoc=libopenvas-doc_9.0.2-1_all.deb
pkglibsym=libopenvas9-dbgsym
deblibsym=libopenvas9-dbgsym_9.0.2-1_armhf.deb
pkglib=libopenvas9
deblib=libopenvas9_9.0.2-1_armhf.deb
pkgclisym=openvas-cli-dbgsym
debclisym=openvas-cli-dbgsym_1.4.5-2_armhf.deb
pkgcli=openvas-cli
debcli=openvas-cli_1.4.5-2_armhf.deb
pkgnaslsym=openvas-nasl-dbgsym
debnaslsym=openvas-nasl-dbgsym_9.0.2-1_armhf.deb
pkgnasl=openvas-nasl
debnasl=openvas-nasl_9.0.2-1_armhf.deb
pkgscannersym=openvas-scanner-dbgsym
debscannersym=openvas-scanner-dbgsym_5.1.2-1_armhf.deb
pkgscanner=openvas-scanner
debscanner=openvas-scanner_5.1.2-1_armhf.deb
pkgopenvas=openvas
debopenvas=openvas_9.0.3kali1_all.deb
pkgredissentinel=redis-sentinel
debredissentinel=redis-sentinel_4.0.10-2_armhf.deb
pkgrediserver=redis-server
debrediserver=redis-server_4.0.10-2_armhf.deb
pkgredistoolssym=redis-tools-dbgsym
debredistoolssym=redis-tools-dbgsym_4.0.10-2_armhf.deb
pkgredistools=redis-tools
debredistools=redis-tools_4.0.10-2_armhf.deb
pkgredis=redis
debredis=redis_4.0.10-2_all.deb

dircli=openvas-cli-1.4.5
dirlib=openvas-libraries-9.0.2
dirscan=openvas-scanner-5.1.2
dirgsa=greenbone-security-assistant-7.0.3+dfsg.1
diropenvas=openvas-9.0.3kali1
dirredis=redis-4.0.10
dirmanager=openvas-manager-7.0.3

delete-source-directories:
	rm -rf $(dircli) $(dirlib) $(dirscan) $(dirgsa) $(diropenvas)\
		$(dirmanager) $(dirredis) $(directories)

apt-get-source:
	apt-get source openvas libopenvas-dev openvas-scanner greenbone-security-assistant openvas-cli redis openvas-manager

uninstall:
	sudo dpkg --purge $(pkggsacommon) $(pkggsasym) $(pkggsa)\
		$(pkglibdev) $(pkglibdoc) $(pkglibsym) $(pkglib)\
		$(pkgclisym) $(pkgcli) \
		$(pkgnaslsym) $(pkgnasl) \
		$(pkgscannersym) $(pkgscanner) \
		$(pkgopenvas) \
		$(pkgredissentinel) $(pkgrediserver) $(pkgredistoolssym) \
		$(pkgredistools) $(pkgredis)

buildpackage: patch
	(cd $(dircli); dpkg-buildpackage -us -uc)
	(cd $(dirlib); dpkg-buildpackage -us -uc)
	(cd $(dirscan); dpkg-buildpackage -us -uc)
	(cd $(dirgsa); dpkg-buildpackage -us -uc)
	(cd $(diropenvas); dpkg-buildpackage -us -uc)
	(cd $(dirmanager); dpkg-buildpackage -us -uc)
	(cd $(dirredis); dpkg-buildpackage -us -uc)

patch:
	-patch -p2 -N -s <openvas-cli.patch
	-patch -p2 -N -s <openvas-libraries.patch
	-patch -p2 -N -s <openvas-scanner.patch
	-patch -p2 -N -s <greenbone-security-assistant.patch
	-patch -p2 -N -s <openvas-manager.patch
	-patch -p2 -N -s <redis.patch

prepare:
	sudo apt-get update
	sudo apt-get install build-essential \
		po4a \
		pkg-perl-autopkgtest \
		dwz \
		dh-autoreconf \
		dh-strip-nondeterminism \
		po-debconf \
		bison \
		lsb-base \
		doc-base \
		gnutls-bin \
		debhelper \
		cmake \
		libsqlite3-dev \
		pkg-config \
		xmltoman \
		libmicrohttpd-dev \
		libxslt1-dev \
		xsltproc \
		libxml2-dev \
		libglib2.0-dev \
		libgcrypt20-dev \
		libgnutls28-dev \
		libpcap-dev \
		doxygen \
		openssl \
		rsync \
		nmap \
		snmp \
		pnscan \
		ike-scan \
		libgpgme-dev \
		libhiredis-dev \
		libksba-dev \
		libldap2-dev \
		libpcap-dev \
		libssh-dev \
		uuid-dev \
		libsnmp-dev \
		libradcli-dev \
		libjemalloc-dev \
		tcl \
		ruby \


installRedis:
	sudo dpkg -i redis-server_4.0.10-2_armhf.deb \
		redis-tools_4.0.10-2_armhf.deb

install: installRedis
	sudo dpkg -i greenbone-security-assistant-common_7.0.3+dfsg.1-1_all.deb\
		greenbone-security-assistant_7.0.3+dfsg.1-1_armhf.deb\
		libopenvas-doc_9.0.2-1_all.deb\
		libopenvas9_9.0.2-1_armhf.deb\
		openvas-cli_1.4.5-2_armhf.deb\
		openvas-manager-common_7.0.3-1_all.deb\
		openvas-manager_7.0.3-1_armhf.deb\
		openvas-nasl_9.0.2-1_armhf.deb\
		openvas-scanner_5.1.2-1_armhf.deb\
		openvas_9.0.3kali1_all.deb	

