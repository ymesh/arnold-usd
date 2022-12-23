#
# Config Arnold USD Python 2
#
USD_VER="22.03"
RMAN_VER="24.3"

PYTHON_VERSION="2"

source ./env_python${PYTHON_VERSION}/bin/activate

app_name="arnold-usd"
app_ver="7.0.0.0"

cur_dir=`pwd`
tmp_dir="tmp_${app_name}_v${app_ver}_${USD_VER}_py${PYTHON_VERSION}"

deploy_root="/home/data/tools"
deploy_dir="${deploy_root}/USD/autodesk/${app_name}_v${app_ver}_${USD_VER}_py${PYTHON_VERSION}"

mkdir -p $tmp_dir
cd $tmp_dir

# export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
export PXR_USD_LOCATION="${deploy_root}/USD/pixar/USD-v${USD_VER}_rman${RMAN_VER}_ABI_0"
export BOOST_ROOT="${deploy_root}/boost/boost_1_75_0_ABI_0"
export Boost_DIR="${BOOST_ROOT}/lib/cmake/Boost-1.75.0"
export BOOST_LIBRARYDIR=${BOOST_ROOT}

#export MTOA_LOCATION="/opt/solidangle/mtoa/mtoa-4.0.4/2020"
#export ARNOLD_LOCATION="/opt/solidangle/Arnold-6.0.4.1-linux"

export ARNOLD_LOCATION="/opt/solidangle/Arnold-${app_ver}-linux"

# set(CMAKE_CXX_STANDARD 14 CACHE STRING "CMake CXX Standard")

cmake3 -LA -G "Unix Makefiles" \
-DCMAKE_BUILD_TYPE="Release" \
-DCMAKE_INSTALL_PREFIX=${deploy_dir} \
-DUSD_MONOLITHIC_BUILD=OFF \
-DUSD_STATIC_BUILD=OFF \
-DTBB_STATIC_BUILD=OFF \
-DTBB_NO_EXPLICIT_LINKAGE=OFF \
-DBUILD_USE_CUSTOM_BOOST=OFF \
-DBUILD_DISABLE_CXX11_ABI=ON \
-DBUILD_HEADERS_AS_SOURCES=OFF \
-DBUILD_SCHEMAS=OFF \
-DBUILD_RENDER_DELEGATE=ON \
-DBUILD_SCENE_DELEGATE=ON \
-DBUILD_PROC_SCENE_FORMAT=ON \
-DBUILD_NDR_PLUGIN=ON \
-DBUILD_PROCEDURAL=ON \
-DBUILD_USD_WRITER=ON \
-DBUILD_USD_IMAGING_PLUGIN=ON \
-DBUILD_DOCS=OFF \
-DBUILD_TESTSUITE=OFF \
-DBUILD_UNIT_TESTS=OFF \
-DBUILD_USE_PYTHON3=OFF \
-DUSD_LOCATION=${PXR_USD_LOCATION} \
-DARNOLD_LOCATION=${ARNOLD_LOCATION} \
-DCMAKE_CXX_FLAGS="-D_GLIBCXX_USE_CXX11_ABI=0" \
-DCMAKE_CXX_STANDARD="14" \
-DBOOST_ROOT=${BOOST_ROOT} \
-DBoost_DIR=${Boost_DIR} \
../..

cd ..

# -DTBB_INCLUDE_DIR="${TBBROOT}/include" \
# -DOpenGL_GL_PREFERENCE=GLVND \

# ``find_package()`` uses ``<PackageName>_ROOT`` variables.
# CMP0074
# -------
# ``find_package()`` uses ``<PackageName>_ROOT`` variables.

if [ $? -eq 0 ]
then
  echo "* "
  echo "* cmake config completed." 
  echo "* run \"make -C ${tmp_dir}\" (or \"make -C ${tmp_dir} install\") " 
  echo "* "
else
  echo "* "
  echo "* cmake config error!"
  echo "* "
fi

