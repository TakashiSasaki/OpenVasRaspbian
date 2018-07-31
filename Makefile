getSource:
	apt-get source openvas libopenvas-dev openvas-scanner greenbone-security-assistant openvas-cli redis

buildpackage: patch
#	(cd openvas-cli-1.4.5; dpkg-buildpackage -us -uc)
	#(cd openvas-libraries-9.0.2; dpkg-buildpackage -us -uc)
	#(cd openvas-scanner-5.1.2; dpkg-buildpackage -us -uc)
	#(cd greenbone-security-assistant-7.0.3+dfsg.1; dpkg-buildpackage -us -uc)
	#(cd openvas-9.0.3kali1; dpkg-buildpackage -us -uc)
	(cd redis-4.0.10; dpkg-buildpackage -us -uc)

patch:
	-patch -p2 -N -s <openvas-cli.patch
	-patch -p2 -N -s <openvas-libraries.patch
	-patch -p2 -N -s <openvas-scanner.patch
	-patch -p2 -N -s <greenbone-security-assistant.patch
	-patch -p2 -N -s <openvas.patch
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

