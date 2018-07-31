-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA256

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
Build-Depends: debhelper (>= 11~), dpkg-dev (>= 1.17.14), libjemalloc-dev [linux-any], procps <!nocheck>, tcl <!nocheck>
Package-List:
 redis deb database optional arch=all
 redis-sentinel deb database optional arch=any
 redis-server deb database optional arch=any
 redis-tools deb database optional arch=any
Checksums-Sha1:
 d2738d9b93a3220eecc83e89a7c28593b58e4909 1738465 redis_4.0.10.orig.tar.gz
 1942827d834b0cac8b4e2c98ecd997ba3f8e4bff 23992 redis_4.0.10-2.debian.tar.xz
Checksums-Sha256:
 1db67435a704f8d18aec9b9637b373c34aa233d65b6e174bdac4c1b161f38ca4 1738465 redis_4.0.10.orig.tar.gz
 76e0c5c76b84815f1e4651c719d0f79b9317cf4516fc48235e035875120d252c 23992 redis_4.0.10-2.debian.tar.xz
Files:
 115b82ea07cb4a6f37c5fd86ab5a6d45 1738465 redis_4.0.10.orig.tar.gz
 502911bd64317a3398eb519f46ed4b9a 23992 redis_4.0.10-2.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQIzBAEBCAAdFiEEwv5L0nHBObhsUz5GHpU+J9QxHlgFAls+fVwACgkQHpU+J9Qx
HljDBg//cc2pzlMLT7JWdB+6mYgf9D8sWVoguhVOUML9NwUpPt5SlGSNpIUkP4L5
obeJiWTU1X8g9IS0B1PuE/ottl2hLyzQxaS+GtN0Kqpo+GOEY3TNgLWTUMUryteT
PEjVgVDNyS/Dx2WI5coA0uIpBAW4luuJlBY7GmdJKGupkDIndoGGr6uUsQkC04QC
alrrVlpfky8RYEDlndAXD7Uifsmrq0nXy1xKfzJAuSOM5MDS3xrMLyrBElyojzDS
yhyguwQ+Xat+cL3GXPvydoDbFVg+Fio8VGI3FacWJrTlhMR0SKR+67kg1PXXKPEh
9rlg1lQGlv2dsNWlOkCM50HNBeidzMbjgk0EJp/iN/7RmgsXKsAV3UuNc/8i5qHx
ectiJSAXL7sQJU093Gspw21fSzGnLAfz9BdAeHC4FzljpQdFkaZJ48sMItUPvOuO
WH7ILPhc2zwvg3kb3FVdpUv6U7oJbe4/g24hBOu7rI29fSQIlzBCM7BIS4QtCRIX
WdqW1f6lnLBmSRLzDHuXOuCZR/jI3DZsVT3V3W5zMMHmnqd+sdi3MEUaeZtvZE8z
lRlQGgjkHnr9oJMHMwW7Lcy/YXMfRuKmiRCH//EOfkbUIC9sLJfVxXUJpERio5C+
QWUnOYBR6vCITY6IzYXQbHDqo5P7m7o8AbDLUkVyfH3ciZrLs+0=
=sFtT
-----END PGP SIGNATURE-----
