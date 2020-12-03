# scorep_plugin_mpi
Score-P MPI data acquisition plugin

# To clone,
$ git clone --recurse-submodules  https://github.com/shuki-zanyovka/scorep_plugin_mpi
$ cd scorep_plugin_mpi

# To build,
mkdir BUILD
cd BUILD

cmake ../ -DCMAKE_C_STANDARD_COMPUTED_DEFAULT=GNU -DCMAKE_CXX_STANDARD_COMPUTED_DEFAULT=GNU -DCMAKE_CXX_COMPILER=mpic++ -DCMAKE_C_COMPILER=mpicc
make

# To use, please enable the plugin as follows,
export LOCAL_USER_SCOREP_INSTALL_PATH=<Score-P 6.0 Install Path>/scorep-6.0
export SCOREP_PLUGIN_MPI_PATH=<Plugin Path>/scorep_plugin_mpi/BUILD
export OMPI_PATH=<OpenMPI Install Path>

export PATH=$OMPI_PATH/bin:$LOCAL_USER_SCOREP_INSTALL_PATH/bin:$PATH
export LD_LIBRARY_PATH=$SCOREP_PLUGIN_UCX_PATH:$SCOREP_PLUGIN_MPI_PATH:$OMPI_PATH:$UCS_LIB_PATH:$LOCAL_USER_SCOREP_INSTALL_PATH:$LD_LIBRARY_PATH

export SCOREP_METRIC_PLUGINS="scorep_plugin_mpi"
export SCOREP_METRIC_SCOREP_PLUGIN_MPI=MPI_T@1

# Disable profilin and enable tracing,
export SCOREP_ENABLE_PROFILING=false
export SCOREP_ENABLE_TRACING=true
export SCOREP_TOTAL_MEMORY=4000M

# It is recommended to use a Score-P filter file to reduce Score-P overhead,
export SCOREP_FILTERING_FILE=./filter.scorep

# Contact us at,
shuki.zanyovka@huawei.com

# A note regarding licensing,
The entire module is licensed via the GPL license but some of the code is based on GYAN, which is part of the mpi-tools git at, 
https://github.com/llnl/mpi-tools

The GYAN licensed files are the files under the following directory in this git repo,
- src/mpi_t_sampling

These files are therefore licensed under the GYAN license at,
https://github.com/LLNL/mpi-tools/blob/master/mpi_t/gyan/LICENSE.txt

