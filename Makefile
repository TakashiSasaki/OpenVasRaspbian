prepare:
	sudo apt-get update
	apt-get source openvas libopenvas-dev openvas-scanner greenbone-security-assistant openvas-cli redis
	sudo apt-get install build-essential po4a pkg-perl-autopkgtest dwz 
	sudo apt-get install dh-autoreconf dh-strip-nondeterminism po-debconf bison 
	sudo apt-get install lsb-base doc-base gnutls-bin debhelper cmake
	sudo apt-get install libsqlite3-dev pkg-config xmltoman 
	sudo apt-get install libmicrohttpd-dev libxslt1-dev xsltproc libxml2-dev
	sudo apt-get install libglib2.0-dev libgcrypt20-dev libgnutls28-dev libpcap-dev doxygen openssl 
	sudo apt-get install redis-server rsync nmap snmp pnscan ike-scan
	sudo apt-get install libgpgme-dev libhiredis-dev libksba-dev libldap2-dev libpcap-dev libssh-dev
	sudo apt-get install uuid-dev libsnmp-dev libradcli-dev 
	sudo apt-get install libjemalloc-dev tcl

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

