prepare:
	sudo apt-get update
	apt-get source openvas libopenvas-dev openvas-scanner greenbone-security-assistant openvas-cli
	sudo apt-get install build-essential
	sudo apt-get install lsb-base doc-base gnutls-bin debhelper cmake
	sudo apt-get install libsqlite3-dev pkg-config xmltoman 
	sudo apt-get install libmicrohttpd-dev libxslt1-dev xsltproc libxml2-dev
	sudo apt-get install libglib2.0-dev libgcrypt20-dev libgnutls28-dev libpcap-dev doxygen openssl 
	sudo apt-get install redis-server rsync nmap snmp pnscan ike-scan
	sudo apt-get install libgpgme-dev libhiredis-dev libksba-dev libldap2-dev libpcap-dev libssh-dev
	sudo apt-get install uuid-dev libsnmp-dev libradcli-dev 
