#!/bin/bash

mkdir third_party ||:
cd third_party

git clone https://github.com/open-source-parsers/jsoncpp.git
cd jsoncpp
# Generate common jsoncpp header and source file. They will be included in the project as source files
python3 amalgamate.py
cd ..

git clone https://github.com/tdv/mif.git

cd mif
# Replace boost libraries fownload location in mif repository
# Old download location does not work
#
# The hack is needed because sed command has different implementaiton in OSX and Linux
# See explanation here
# https://stackoverflow.com/questions/16745988/sed-command-with-i-option-in-place-editing-works-fine-on-ubuntu-but-not-mac
if [[ "$OSTYPE" == "darwin"* ]]; then
    sed -i '' 's,https://dl.bintray.com/boostorg/release/1.76.0/source/boost_1_76_0.tar.gz,https://sourceforge.net/projects/boost/files/boost/1.76.0/boost_1_76_0.tar.gz,g' download_third_party.sh
else
    sed -i 's,https://dl.bintray.com/boostorg/release/1.76.0/source/boost_1_76_0.tar.gz,https://sourceforge.net/projects/boost/files/boost/1.76.0/boost_1_76_0.tar.gz,g' download_third_party.sh
fi

./download_third_party.sh
