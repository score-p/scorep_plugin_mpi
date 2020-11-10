# scorep_plugin_mpi
Score-P MPI data acquisition plugin

git clone --recurse-submodules  https://github.com/shuki-zanyovka/scorep_plugin_mpi
cd scorep_plugin_mpi

To build:

mkdir BUILD
cd BUILD

cmake ../ -DCMAKE_C_STANDARD_COMPUTED_DEFAULT=GNU -DCMAKE_CXX_STANDARD_COMPUTED_DEFAULT=GNU -DCMAKE_CXX_COMPILER=/usr/local/bin/g++ -DCMAKE_C_COMPILER=/usr/local/bin/gcc
make


