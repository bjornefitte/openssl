#! /bin/sh -x

# $1 is expected to be $TRAVIS_OS_NAME

CONFIGURE_DEBUG=1 ./Configure dist
if [ "$1" == osx ]; then
    make NAME='_srcdist' TARFILE='_srcdist.tar' \
         TAR_COMMAND='$(TAR) $(TARFLAGS) -cvf -' tar
else
    make TARFILE='_srcdist.tar' NAME='_srcdist' dist
fi
