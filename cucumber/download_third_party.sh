mkdir third_party_sources ||:
cd third_party_sources

git clone https://github.com/tdv/mif.git

./mif/download_third_party.sh

# replace boost binaries download path (it is invalid in the repository)
# shall be 
# wget -c 'http://sourceforge.net/projects/boost/files/boost/1.76.0/boost_1_76_0.tar.bz2/download'
