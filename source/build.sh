#!/bin/bash
type=${1:-asan}
clean=${2:-0}
all=${3:-1}
compiler=${4:-g++-13}
export CXX=$compiler;
BUILD=$JDE_DIR/Public/build/so.sh;
pushd `pwd` > /dev/null;
if [ $all -eq 1 ]; then
	$BUILD ../../Framework/source $type $clean $compiler || exit 1;
	cd ../../Public/libs;
	$BUILD crypto/src $type $clean $compiler || exit 1;
	$BUILD db/src $type $clean $compiler || exit 1;
	$BUILD db/src/drivers/mysql $type $clean $compiler || exit 1;
	$BUILD ql $type $clean $compiler || exit 1;
	$BUILD access/src $type $clean $compiler || exit 1;
	$BUILD web/client $type $clean $compiler || exit 1;
	$BUILD web/server $type $clean $compiler || exit 1;
	$BUILD app/shared $type $clean $compiler || exit 1;
	$BUILD app/client $type $clean $compiler || exit 1;
	$BUILD opc/src $type $clean $compiler || exit 1;
	$BUILD opc/tests $type $clean $compiler || exit 1;
fi;
popd  > /dev/null;
if [ ! -d .obj ];	then
	mkdir .obj;
	clean=1;
fi;
cd .obj;
if [ ! -d $type ]; then
	mkdir $type;
	clean=1;
fi;
cd $type;
if (( $clean == 1 )) || [ ! -f CMakeCache.txt ]; then
	rm -f CMakeCache.txt;
	cmake -DCMAKE_CXX_COMPILER=$compiler -DCMAKE_BUILD_TYPE=$type  ../.. > /dev/null;
	make clean;
fi
make -j;
cd - > /dev/null;
exit $?;
