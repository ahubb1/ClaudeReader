 #!/bin/bash
#Script to create a simple unix release of the project

base_dir=../Releases/Unix/
base_file=bdreader
base_name=$base_dir$base_file-
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
	echo -e Overwrite ? '('y/n')'
	read valid
	if [[ $valid == y* ]]
	then
		sudo rm -rf $fullname
		sudo rm $base_file'_'$version_nb-1_amd64.deb
	else
		exit
	fi
fi

#Copy the files into the new directory
echo -e '\t\t' ----Files copy----
echo -n -e '\t' Create $fullname folder...
mkdir $fullname
echo "done."
echo -n -e '\t' Copy sources...
cp -r src $fullname/
#mkdir $fullname/include
#cp -r include/stlplus3 $fullname/include/
#cp include/* $fullname/include/
echo "done."
#echo -n -e '\t' Copy libraries...
#mkdir $fullname/lib
#cp -r lib/unix $fullname/lib
#echo "done."
echo -n -e '\t' Copy doc...
mkdir $fullname/doc
cp doc/* $fullname/doc/
echo "done."
echo -n -e '\t' Copy Makefiles...
cp -r Makefile.am $fullname/
cp -r configure.ac $fullname/
echo "done."
echo -n -e '\t' Copy shared files...
mkdir $fullname/share
cp share/* $fullname/share/
echo "done."
echo -n -e '\t' Copy other files...
cp CHANGES $fullname/
cp COPYING $fullname/
cp README $fullname/
echo "done."
#echo -n -e '\t' Copy Code::Blocks project...
#mkdir $fullname/codeblocks
#cp -r BDReader.cbp $fullname/codeblocks/
#echo "done."
echo 

#Run the makefile for testing
echo -e '\t\t' ----Release testing----
origdir=$(pwd)
cd $fullname
echo -e '\t' Autoreconf...
autoreconf --install
echo done.
echo -e '\t' Configure...
./configure
echo Configure done.
echo -e '\t' Make test...
make
echo 
if [ -a src/bdreader ]
then
	echo make successfull.
else
	echo make error...exit.
	exit
fi
echo
echo -e '\t' Make install test...
sudo make install
bdreader
echo
echo -e '\t' Make uninstall test ...
sudo make uninstall
echo
echo -e '\t' Make clean test ...
sudo make clean


#Archive creation
echo -e '\t\t' ----Archive creation----
cd ..
echo -e Create an archive '('$base_file-$version_nb'.tar.gz) ? (y/n)'
read valid
if [[ $valid == y* ]]
then
	echo -n -e '\t' Creation...
	tar -zcvf $base_file-$version_nb.tar.gz $base_file-$version_nb
	echo "done."
fi

#Deb package preparation
echo -e '\t\t' ----Deb package preparation----
echo -e Prepare a deb package ? '('y/n')'
read valid
if [[ $valid == y* ]]
then
	echo -n -e '\t' Move files to doc...
	mv $base_file-$version_nb/README $base_file-$version_nb/doc/
	mv $base_file-$version_nb/CHANGES $base_file-$version_nb/doc/
	mv $base_file-$version_nb/COPYING $base_file-$version_nb/doc/
	echo -n -e '\t' Creation of the archive...
	tar -zcvf $base_file'_'$version_nb.orig.tar.gz $base_file-$version_nb
	echo "done."
	echo -n -e '\t' Creation of the package...
	cd $base_file-$version_nb/ && dh_make --single -e kennyyy@users.sf.net -c gpl3
	cp ../DebFilesTemplates/* debian/
	rm debian/*.ex
	rm debian/*.EX
	dpkg-buildpackage -rsudo
	echo "done."
	echo -n -e '\t' Package testing...
	cd ..
	sudo chown $USER $base_file'_'$version_nb-1_amd64.deb
	lintian -Ivi $base_file'_'$version_nb-1_amd64.deb
	echo "done."
	
	echo -e '\t\t' ----Upload on PPA----
	echo -e Upload package on PPA ? '('y/n')'
	read valid
	if [[ $valid == y* ]]
	then
		dput ppa:kennyyy/bdreader $base_file'_'$version_nb-1_amd64.changes
	fi
fi
exit

