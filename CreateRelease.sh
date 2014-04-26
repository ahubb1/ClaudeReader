 #!/bin/bash
#Script to create a simple unix release of the project

 
base_name=../Releases/Unix/bdreader_src_v 
echo -------------------------
echo BDReader release creation
echo -------------------------
echo Base folder name: $base_name
echo -n Version number ?
read version_nb
echo
fullname=$base_name$version_nb
if [ -a $fullname ]
then
	echo $fullname folder already exists... exit.
	exit
fi

#Copy the files into the new directory
echo -e '\t\t' ----Files copy----
echo -n -e '\t' Create $fullname folder...
mkdir $fullname
echo "done."
echo -n -e '\t' Copy sources...
cp -r src $fullname/
cp -r include $fullname/
echo "done."
echo -n -e '\t' Copy libraries...
mkdir $fullname/lib
cp -r lib/unix $fullname/lib
echo "done."
echo -n -e '\t' Copy doc...
mkdir $fullname/doc
cp doc/* $fullname/doc/
echo "done."
echo -n -e '\t' Copy Makefile...
cp -r Makefile $fullname/
echo "done."
echo -n -e '\t' Copy help files...
cp -r about $fullname/
cp -r help $fullname/
cp -r COPYING $fullname/
echo "done."
echo -n -e '\t' Copy Code::Blocks project...
mkdir $fullname/codeblocks
cp -r BDReader.cbp $fullname/codeblocks/
echo "done."
echo 

#Run the makefile for testing
echo -e '\t\t' ----Release testing----
origdir=$(pwd)
cd $fullname
echo -e '\t' Make test...
make
echo 
if [ -a bin/Release/BDReader ]
then
	echo make successfull.
else
	echo make error...exit.
	exit
fi
echo
echo -e '\t' Make install test...
sudo make install
BDReader
echo
echo -e '\t' Make uninstall test ...
sudo make uninstall
echo
echo -e '\t' Make clean test ...
sudo make clean


#Archive creation
echo -e '\t\t' ----Archive creation----
cd $origdir
echo -e Create an archive '('$fullname'.tar.gz) ? (y/n)'
read valid
if [[ $valid == y* ]]
then
	echo -n -e '\t' Creation...
	tar -zcvf $fullname.tar.gz $fullname
	echo "done."
fi
exit

