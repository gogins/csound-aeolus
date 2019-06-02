#!/bash
echo "Starting completely fresh rebuild on Linux..."
git pull
bash install-dependencies.bash
mkdir -p build
rm -rf build/*
cd build
echo "Configuring build..."
cmake ..
echo "Building..."
make
echo "Packaging for Debian..."
cpack
find . -name '*.deb' -ls -exec dpkg -c '{}' ';'
cd ..
echo "Finished completely fresh rebuild on Linux."

