deb-greenbone-security-assistant-common=greenbone-security-assistant-common_7.0.3+dfsg.1-1_all.deb
deb-greenbone-security-assistant-dbgsym=greenbone-security-assistant-dbgsym_7.0.3+dfsg.1-1_armhf.deb
deb-greenbone-security-assistant=greenbone-security-assistant_7.0.3+dfsg.1-1_armhf.deb
deb-libopenvas9-dbgsym=libopenvas9-dbgsym_9.0.2-1_armhf.deb
deb-libopenvas9-dev=libopenvas-dev_9.0.2-1_armhf.deb
deb-libopenvas9-doc=libopenvas-doc_9.0.2-1_all.deb
deb-libopenvas9=libopenvas9_9.0.2-1_armhf.deb
deb-openvas-cli-dbgsym=openvas-cli-dbgsym_1.4.5-2_armhf.deb
deb-openvas-cli=openvas-cli_1.4.5-2_armhf.deb
deb-openvas-manager-common=openvas-manager-common_7.0.3-1_all.deb
deb-openvas-manager-dbgsym=openvas-manager-dbgsym_7.0.3-1_armhf.deb
deb-openvas-manager=openvas-manager_7.0.3-1_armhf.deb
deb-openvas-nasl-dbgsym=openvas-nasl-dbgsym_9.0.2-1_armhf.deb
deb-openvas-nasl=openvas-nasl_9.0.2-1_armhf.deb
deb-openvas-scanner-dbgsym=openvas-scanner-dbgsym_5.1.2-1_armhf.deb
deb-openvas-scanner=openvas-scanner_5.1.2-1_armhf.deb
deb-openvas=openvas_9.0.3kali1_all.deb
deb-redis-sentinel=redis-sentinel_4.0.10-2_armhf.deb
deb-redis-server=redis-server_4.0.10-2_armhf.deb
deb-redis-tools-dbgsym=redis-tools-dbgsym_4.0.10-2_armhf.deb
deb-redis-tools=redis-tools_4.0.10-2_armhf.deb
deb-redis=redis_4.0.10-2_all.deb

deb-all+=$(deb-greenbone-security-assistant-common)
deb-all+=$(deb-greenbone-security-assistant-dbgsym)
deb-all+=$(deb-greenbone-security-assistant)
deb-all+=$(deb-libopenvas9-dbgsym)
deb-all+=$(deb-libopenvas9-dev)
deb-all+=$(deb-libopenvas9-doc)
deb-all+=$(deb-libopenvas9)
deb-all+=$(deb-openvas-cli-dbgsym)
deb-all+=$(deb-openvas-cli)
deb-all+=$(deb-openvas-manager-common)
deb-all+=$(deb-openvas-manager-dbgsym)
deb-all+=$(deb-openvas-manager)
deb-all+=$(deb-openvas-nasl-dbgsym)
deb-all+=$(deb-openvas-nasl)
deb-all+=$(deb-openvas-scanner-dbgsym)
deb-all+=$(deb-openvas-scanner)
deb-all+=$(deb-openvas)
deb-all+=$(deb-redis-sentinel)
deb-all+=$(deb-redis-server)
deb-all+=$(deb-redis-tools-dbgsym)
deb-all+=$(deb-redis-tools)
deb-all+=$(deb-redis)

pkg-greenbone-security-assistant-common=greenbone-security-assistant-common
pkg-greenbone-security-assistant-dbgsym=greenbone-security-assistant-dbgsym
pkg-greenbone-security-assistant=greenbone-security-assistant
pkg-libopenvas-dev=libopenvas-dev
pkg-libopenvas-doc=libopenvas-doc
pkg-libopenvas9-dbgsym=libopenvas9-dbgsym
pkg-libopenvas9=libopenvas9
pkg-openvas-cli-dbgsym=openvas-cli-dbgsym
pkg-openvas-cli=openvas-cli
pkg-openvas-manager-common=openvas-manager-common
pkg-openvas-manager-dbgsym=openvas-manager-dbgsym
pkg-openvas-manager=openvas-manager
pkg-openvas-nasl-dbgsym=openvas-nasl-dbgsym
pkg-openvas-nasl=openvas-nasl
pkg-openvas-scanner-dbgsym=openvas-scanner-dbgsym
pkg-openvas-scanner=openvas-scanner
pkg-openvas=openvas
pkg-redis-sentinel=redis-sentinel
pkg-redis-server=redis-server
pkg-redis-tools-dbgsym=redis-tools-dbgsym
pkg-redis-tools=redis-tools
pkg-redis=redis

pkg-all+=$(pkg-greenbone-security-assistant-common)
pkg-all+=$(pkg-greenbone-security-assistant-dbgsym)
pkg-all+=$(pkg-greenbone-security-assistant)
pkg-all+=$(pkg-libopenvas9-dbgsym)
pkg-all+=$(pkg-libopenvas-dev)
pkg-all+=$(pkg-libopenvas-doc)
pkg-all+=$(pkg-libopenvas9)
pkg-all+=$(pkg-openvas-cli-dbgsym)
pkg-all+=$(pkg-openvas-cli)
pkg-all+=$(pkg-openvas-manager-common)
pkg-all+=$(pkg-openvas-manager-dbgsym)
pkg-all+=$(pkg-openvas-manager)
pkg-all+=$(pkg-openvas-nasl-dbgsym)
pkg-all+=$(pkg-openvas-nasl)
pkg-all+=$(pkg-openvas-scanner-dbgsym)
pkg-all+=$(pkg-openvas-scanner)
pkg-all+=$(pkg-openvas)
pkg-all+=$(pkg-redis-sentinel)
pkg-all+=$(pkg-redis-server)
pkg-all+=$(pkg-redis-tools-dbgsym)
pkg-all+=$(pkg-redis-tools)
pkg-all+=$(pkg-redis)

dir-greenbone-security-assistant=greenbone-security-assistant-7.0.3+dfsg.1
dir-openvas-cli=openvas-cli-1.4.5
dir-openvas-libraries=openvas-libraries-9.0.2
dir-openvas-manager=openvas-manager-7.0.3
dir-openvas-scanner=openvas-scanner-5.1.2
dir-openvas=openvas-9.0.3kali1
dir-redis=redis-4.0.10

dir-all+=$(dir-greenbone-security-assistant) 
dir-all+=$(dir-openvas) 
dir-all+=$(dir-openvas-cli) 
dir-all+=$(dir-openvas-libraries) 
dir-all+=$(dir-openvas-manager) 
dir-all+=$(dir-openvas-scanner) 
dir-all+=$(dir-redis) 

source-packages+=greenbone-security-assistant
source-packages+=libopenvas-dev
source-packages+=openvas-cli
source-packages+=openvas-manager
source-packages+=openvas-scanner
source-packages+=redis

help:
	@echo make prepare
	@echo make build
	@echo make install

prepare: install-prerequisites-packages apt-get-source

build: $(deb-libopenvas9-dev)
	sudo dpkg -i $<
	(cd $(dir-greenbone-security-assistant); dpkg-buildpackage -us -uc)
	(cd $(dir-openvas-cli); dpkg-buildpackage -us -uc)
	(cd $(dir-openvas-manager); dpkg-buildpackage -us -uc)
	(cd $(dir-openvas-scanner); dpkg-buildpackage -us -uc)
	(cd $(dir-redis); dpkg-buildpackage -us -uc)
	#(cd $(dir-openvas); dpkg-buildpackage -us -uc)

clean: delete-dir-all delete-download-all
	git clean -xdf

delete-dir-all:
	rm -rf $(dir-all)

apt-get-source:
	sudo cp -i kali.list /etc/apt/sources.list.d/
	apt-get source $(source-packages)

$(deb-libopenvas9-dev): patch
	(cd $(dir-openvas-libraries); dpkg-buildpackage -us -uc)

patch:
	-patch -p2 -N -s <openvas-cli.patch
	-patch -p2 -N -s <openvas-libraries.patch
	-patch -p2 -N -s <openvas-scanner.patch
	-patch -p2 -N -s <greenbone-security-assistant.patch
	-patch -p2 -N -s <openvas-manager.patch
	-patch -p2 -N -s <redis.patch

prerequisites-packages+=bison
prerequisites-packages+=build-essential
prerequisites-packages+=cmake
prerequisites-packages+=debhelper
prerequisites-packages+=dh-autoreconf
prerequisites-packages+=dh-strip-nondeterminism
prerequisites-packages+=doc-base
prerequisites-packages+=doxygen
prerequisites-packages+=dwz
#prerequisites-packages+=gnutls-bin
prerequisites-packages+=ike-scan
prerequisites-packages+=libgcrypt20-dev
prerequisites-packages+=libglib2.0-dev
prerequisites-packages+=libgnutls28-dev
prerequisites-packages+=libgpgme-dev
prerequisites-packages+=libhiredis-dev
prerequisites-packages+=libjemalloc-dev
prerequisites-packages+=libksba-dev
prerequisites-packages+=libldap2-dev
prerequisites-packages+=libmicrohttpd-dev
prerequisites-packages+=libpcap-dev
prerequisites-packages+=libradcli-dev
prerequisites-packages+=libsnmp-dev
prerequisites-packages+=libsqlite3-dev
prerequisites-packages+=libssh-dev
prerequisites-packages+=libxml2-dev
prerequisites-packages+=libxslt1-dev
#prerequisites-packages+=lsb-base
prerequisites-packages+=nmap
#prerequisites-packages+=openssl
prerequisites-packages+=pkg-config
prerequisites-packages+=pkg-perl-autopkgtest
prerequisites-packages+=pnscan
prerequisites-packages+=po-debconf
prerequisites-packages+=po4a
prerequisites-packages+=rsync
#prerequisites-packages+=ruby
prerequisites-packages+=snmp
prerequisites-packages+=tcl
prerequisites-packages+=uuid-dev
prerequisites-packages+=xmltoman
prerequisites-packages+=xsltproc

install-prerequisites-packages:
	sudo apt --fix-broken install
	sudo dpkg --configure -a
	sudo apt-get update
	sudo apt-get install $(prerequisites-packages) 

uninstall-prerequisites-packages:
	sudo apt --fix-broken install
	sudo dpkg --configure -a
	sudo apt remove $(prerequisites-packages)
	sudo apt autoremove

uninstall:
	sudo apt --fix-broken install
	sudo dpkg --configure -a
	sudo dpkg --purge $(pkg-all) 

install: 
	sudo apt --fix-broken install
	sudo dpkg --configure -a
	sudo dpkg -i \
		$(deb-greenbone-security-assistant) \
		$(deb-greenbone-security-assistant-common) \
		$(deb-libopenvas9) \
		$(deb-libopenvas9-doc) \
		$(deb-openvas-manager)\
		$(deb-openvas-manager-common) \
		$(deb-redis-server) \
		$(deb-redis-tools) \
		$(deb-openvas-scanner)

BASEURL=http://http.kali.org/kali/pool/main/o/openvas/
DOWNLOAD=download
download-arch-independent:
	-mkdir $(DOWNLOAD)
	(cd $(DOWNLOAD); wget -nc -nd -nH $(BASEURL)/openvas_9.0.3.dsc)
	(cd $(DOWNLOAD); wget -nc -nd -nH $(BASEURL)/openvas_9.0.3.tar.xz)
	(cd $(DOWNLOAD); wget -nc -nd -nH $(BASEURL)/openvas_9.0.3_all.deb)
	(cd $(DOWNLOAD); wget -nc -nd -nH $(BASEURL)/openvas_9.0.3kali1.dsc)
	(cd $(DOWNLOAD); wget -nc -nd -nH $(BASEURL)/openvas_9.0.3kali1.tar.xz)
	(cd $(DOWNLOAD); wget -nc -nd -nH $(BASEURL)/openvas_9.0.3kali1_all.changes)
	(cd $(DOWNLOAD); wget -nc -nd -nH $(BASEURL)/openvas_9.0.3kali1_all.deb)
	(cd $(DOWNLOAD); wget -nc -nd -nH $(BASEURL)/openvas_9.0.3kali1_amd64.buildinfo)
	(cd $(DOWNLOAD); wget -nc -nd -nH $(BASEURL)/openvas_9.0.3kali1_source.buildinfo)
	(cd $(DOWNLOAD); wget -nc -nd -nH $(BASEURL)/openvas_9.0.3kali1_source.changes)


delete-download-all:
	-rm -rf $(DOWNLOAD)

