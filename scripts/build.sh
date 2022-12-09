printf "Checking if build directory exists... "
if [ -d build ]
then
  echo "YES. Deleteing"
  rm -rf build
else
  echo "NO"
fi

printf "Creating build directory... "
mkdir -p build
cd build
echo "DONE"

echo "Starting configuration"
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchains/mingw.cmake .. || exit 1

echo "Building"
cmake --build . || exit 1

echo "Installing"
cmake --install . || exit 1
