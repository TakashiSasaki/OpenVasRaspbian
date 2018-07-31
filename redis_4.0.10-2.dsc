Format: 3.0 (quilt)
Source: redis
Binary: redis, redis-sentinel, redis-server, redis-tools
Architecture: any all
Version: 5:4.0.10-2
Maintainer: Chris Lamb <lamby@debian.org>
Homepage: https://redis.io/
Standards-Version: 4.1.5
Vcs-Browser: https://salsa.debian.org/lamby/pkg-redis
Vcs-Git: https://salsa.debian.org/lamby/pkg-redis.git
Testsuite: autopkgtest
Build-Depends: debhelper (>= 10), dpkg-dev (>= 1.17.14), libjemalloc-dev [linux-any], procps <!nocheck>, tcl <!nocheck>
Package-List:
 redis deb database optional arch=all
 redis-sentinel deb database optional arch=any
 redis-server deb database optional arch=any
 redis-tools deb database optional arch=any
Checksums-Sha1:
 d2738d9b93a3220eecc83e89a7c28593b58e4909 1738465 redis_4.0.10.orig.tar.gz
 153eb948d1206665fff5866e49d2e8ea21cd4dc8 24000 redis_4.0.10-2.debian.tar.xz
Checksums-Sha256:
 1db67435a704f8d18aec9b9637b373c34aa233d65b6e174bdac4c1b161f38ca4 1738465 redis_4.0.10.orig.tar.gz
 8402295f9105cdd17895e6504771409999306d19ce6552561a51ed90fe52e69b 24000 redis_4.0.10-2.debian.tar.xz
Files:
 115b82ea07cb4a6f37c5fd86ab5a6d45 1738465 redis_4.0.10.orig.tar.gz
 141c329a66dc13c55acc5f25d7ccb4eb 24000 redis_4.0.10-2.debian.tar.xz
