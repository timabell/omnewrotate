#!/bin/bash
release=$1
if [ "$release" == "" ]; then
	release="master"
	release=`git log -1 --pretty=format:%h $release`
	git archive --prefix omnewrotate-$release/ $release | gzip > omnewrotate-$release.tar.gz
else
	git archive --prefix omnewrotate-$release/ $release > omnewrotate-$release.tar
	mkdir fixup-tmp
	tar -C fixup-tmp -xf omnewrotate-$release.tar omnewrotate-$release/setup.py
	sed -i s/##version##/\"$release\"/ fixup-tmp/omnewrotate-$release/setup.py
	tar --delete -f omnewrotate-$release.tar omnewrotate-$release/setup.py
	tar -C fixup-tmp -uf omnewrotate-$release.tar omnewrotate-$release/setup.py
	rm -rf fixup-tmp
	gzip omnewrotate-$release.tar
	md5sum omnewrotate-$release.tar.gz > omnewrotate-$release.tar.gz.md5
fi
echo "archive omnewrotate-$release.tgz created. contents:"
tar -tzf omnewrotate-$release.tar.gz
