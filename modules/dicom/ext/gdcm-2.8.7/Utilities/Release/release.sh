#!/bin/sh
############################################################################
#
#  Program: GDCM (Grassroots DICOM). A DICOM library
#
#  Copyright (c) 2006-2011 Mathieu Malaterre
#  All rights reserved.
#  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.
#
#     This software is distributed WITHOUT ANY WARRANTY; without even
#     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#     PURPOSE.  See the above copyright notice for more information.
#
############################################################################

echo "Start release"
date
echo ""

major=2
minor=6
patch=8
dirversion="$major.$minor"
version="$major.$minor.$patch"
version2="$major-$minor-$patch"

basedir="/tmp/gdcm_release"

check_exit_value()
{
   VALUE="$1"
   if [ "$VALUE" != "0" ]; then
    echo "error in $2"
    exit 1
   fi
}

if [ ! -d $basedir ]; then
  mkdir $basedir
else
  echo "$basedir already exist"
  echo "$basedir/gdcm already exist, cleaning it up:"
  rm -rf $basedir/gdcm
fi

echo "Checking out gdcm"
git clone --branch release git://git.code.sf.net/p/gdcm/gdcm $basedir/gdcm
check_exit_value $? "git did not return properly" || exit 1

# Get the specific tag
# There is no way apparently to directly clone and checkout a tag; you can do
# that only for branch name
cd $basedir/gdcm
git checkout "v$version"
check_exit_value $? "git checkout did not return properly" || exit 1

if [ ! -d $basedir/gdcm-build ]; then
  mkdir $basedir/gdcm-build
else
  echo "$basedir/gdcm-build already exist, cleaning it up:"
  rm -rf $basedir/gdcm-build
  mkdir $basedir/gdcm-build
fi

cd $basedir/gdcm-build

# debian default:
export JAVA_HOME=/usr/lib/jvm/default-java

cat > $basedir/gdcm-build/CMakeCache.txt << EOT
CMAKE_BUILD_TYPE:STRING=Release
GDCM_BUILD_APPLICATIONS:BOOL=ON
GDCM_BUILD_EXAMPLES:BOOL=OFF
GDCM_BUILD_SHARED_LIBS:BOOL=ON
GDCM_BUILD_TESTING:BOOL=OFF
GDCM_DOCUMENTATION:BOOL=ON
GDCM_PDF_DOCUMENTATION:BOOL=ON
GDCM_USE_VTK:BOOL=OFF
GDCM_USE_JPEGLS:BOOL=ON
GDCM_USE_PVRG:BOOL=ON
GDCM_USE_SYSTEM_OPENJPEG:BOOL=OFF
GDCM_USE_SYSTEM_OPENSSL:BOOL=OFF
GDCM_USE_SYSTEM_EXPAT:BOOL=OFF
GDCM_USE_SYSTEM_POPPLER:BOOL=OFF
GDCM_USE_SYSTEM_UUID:BOOL=OFF
GDCM_USE_SYSTEM_ZLIB:BOOL=OFF
GDCM_WRAP_CSHARP:BOOL=ON
GDCM_WRAP_JAVA:BOOL=ON
GDCM_WRAP_PYTHON:BOOL=ON
Python_ADDITIONAL_VERSIONS:STRING=2.7
CPACK_SOURCE_ZIP:BOOL=ON
EOT

cmake $basedir/gdcm
#cmake $basedir/gdcm -DCMAKE_TOOLCHAIN_FILE=$basedir/gdcm/CMake/Toolchain-gcc-m32.cmake
check_exit_value $? "cmake did not return properly" || exit 1

make -j2
check_exit_value $? "make did not return properly" || exit 1

cpack -G TGZ
check_exit_value $? "cpack did not return properly" || exit 1

cpack -G TBZ2
check_exit_value $? "cpack did not return properly" || exit 1

# source release
cpack -G ZIP --config CPackSourceConfig.cmake
check_exit_value $? "cpack did not return properly" || exit 1

cpack -G TGZ --config CPackSourceConfig.cmake
check_exit_value $? "cpack did not return properly" || exit 1

cpack -G TBZ2 --config CPackSourceConfig.cmake
check_exit_value $? "cpack did not return properly" || exit 1

# Let's start doing the VTK documentation then:
#cmake -DGDCM_VTK_DOCUMENTATION:BOOL=ON -DGDCM_USE_VTK:BOOL=ON -DVTK_DIR:PATH=/home/mathieu/tmp/vtk-5.10.1+dfsg/Build .
cmake -DGDCM_VTK_DOCUMENTATION:BOOL=ON -DGDCM_USE_VTK:BOOL=ON -DVTK_DIR:PATH=/home/mathieu/tmp/vtk6-6.2.0+dfsg1/debian/build .
check_exit_value $? "cmake did not return properly" || exit 1
#make -j4
make rebuild_cache
make vtkgdcmDoxygenDoc
check_exit_value $? "vtkgdcmDoxygenDoc did not return properly" || exit 1

# https://sourceforge.net/p/forge/documentation/File%20Management/#rsync-over-ssh
# Warning need to create /manually/ the subfolder:
# https://sourceforge.net/projects/gdcm/files/gdcm%202.x/#folder-create

# Update default version:
#https://sourceforge.net/p/forge/documentation/Using%20the%20Release%20API/
#rsync -e ssh GDCM-$version-Linux-x86_64.tar.gz          "malat,gdcm@frs.sourceforge.net:/home/frs/project/g/gd/gdcm/gdcm\ 2.x/GDCM\ $version"
#check_exit_value $? "rsync did not return properly" || exit 1
#rsync -e ssh GDCM-$version-Linux-x86_64.tar.bz2         "malat,gdcm@frs.sourceforge.net:/home/frs/project/g/gd/gdcm/gdcm\ 2.x/GDCM\ $version"
#check_exit_value $? "rsync did not return properly" || exit 1
#rsync -e ssh gdcm-$version.zip                          "malat,gdcm@frs.sourceforge.net:/home/frs/project/g/gd/gdcm/gdcm\ 2.x/GDCM\ $version"
#check_exit_value $? "rsync did not return properly" || exit 1
rsync -e ssh gdcm-$version.tar.gz                       "malat,gdcm@frs.sourceforge.net:/home/frs/project/g/gd/gdcm/gdcm\ 2.x/GDCM\ $version"
check_exit_value $? "rsync did not return properly" || exit 1

def_prop="default=windows&default=mac&default=linux&default=bsd&default=solaris&default=others"
def_path="https://sourceforge.net/projects/gdcm/files/gdcm%202.x/GDCM%20$version/gdcm-$version.tar.gz"
curl -k -H "Accept: application/json" -X PUT -d $def_prop -d "api_key=$SFAPIKEY" $def_path

rsync -e ssh gdcm-$version.tar.bz2                      "malat,gdcm@frs.sourceforge.net:/home/frs/project/g/gd/gdcm/gdcm\ 2.x/GDCM\ $version"
check_exit_value $? "rsync did not return properly" || exit 1
rsync -e ssh Utilities/doxygen/gdcm-$version-doc.tar.gz "malat,gdcm@frs.sourceforge.net:/home/frs/project/g/gd/gdcm/gdcm\ 2.x/GDCM\ $version"
check_exit_value $? "rsync did not return properly" || exit 1
sed "s/vVERSION/v$version/" $basedir/gdcm/Utilities/Release/README.md > $basedir/README.md
rsync -e ssh $basedir/README.md "malat,gdcm@frs.sourceforge.net:/home/frs/project/g/gd/gdcm/gdcm\ 2.x/GDCM\ $version"
check_exit_value $? "rsync did not return properly" || exit 1

rsync -a -r Utilities/doxygen/html malat,gdcm@web.sourceforge.net:htdocs/$dirversion
check_exit_value $? "rsync recursive html did not return properly" || exit 1
rsync -av Utilities/doxygen/gdcm-$version-doc.tar.gz malat,gdcm@web.sourceforge.net:htdocs/$dirversion
check_exit_value $? "rsync tarball did not return properly" || exit 1
# This need to be done last, sometime we cannot generated PDF, see #318
rsync -av Utilities/doxygen/latex/gdcm-$version.pdf malat,gdcm@web.sourceforge.net:htdocs/$dirversion
check_exit_value $? "rsync pdf did not return properly" || exit 1
# same comment:
rsync -e ssh Utilities/doxygen/latex/gdcm-$version.pdf  "malat,gdcm@frs.sourceforge.net:/home/frs/project/g/gd/gdcm/gdcm\ 2.x/GDCM\ $version"
check_exit_value $? "rsync did not return properly" || exit 1
