#!/bin/bash
#if error occur, stop script
echo "=======================Bulid XDataCollector Test================="

echo "$1"
echo "$2"
echo "$3"

project_path=$(cd `dirname $0`; pwd)
project_name="${project_path##*/}"

echo $project_path
echo $project_name

check_build_dir=`ls -l |grep "Build"`

echo $check_build_dir

if [[ $check_build_dir =~ "Build" ]]
then
	echo "Has Bulid Directroy!"
else
	echo "Create Build Directory!"
	mkdir Build
fi

#easylog build
check_liblog_dir=`ls -l ./feature/log |grep "Build"`
echo $check_liblog_dir
if [[ $check_liblog_dir =~ "Build" ]]
then
	echo "Has log build Directory"
else
	echo "Create log Build Directory"
	mkdir ./feature/log/Build
fi

cd $project_path/feature/log/Build
cmake -D CMAKE_BUILD_TYPE=$2 ..

if [[ "$1" =~ "clean" ]]
then
	make clean-all
else
	echo "====================================....Build log start....====================================="
	make
	echo "====================================....Build log end....======================================="
fi


#app build
cd $project_path/Build/
cmake -D CMAKE_BUILD_TYPE=$2 ..

if [[ "$1" =~ "clean" ]]
then
	make clean-all
	rm -rf ./*
	#clear swp
	cd $project_path 

	#python version
	python -V
	rtn="$?"
	if [[ $rtn == "0" ]]
	then
		echo "python"
		python cleartmp.py
	else
		echo "python3"
		python3 cleartmp.py
	fi

	echo $check_python_ver
	echo "====================================....clean Success....========================================="
else
	echo "====================================....Build start....==============================================="
	make
	echo "generate maintest at ./Build/bin/"
	echo "====================================....Build Success....========================================="
	echo "press anykey to running xDC!"
	read input

	cd $project_path/Build/bin

	./xDC &
fi
