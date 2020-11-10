/**
* Copyright (C) Huawei Technologies Co., Ltd. 2020.  ALL RIGHTS RESERVED.
*/

#include "utility.h"

#include <mpi.h> /* Adds MPI_T definiKons as well */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "mpi_t_sampling_overrides.h"
#include "mpi_t_sampling.h"

/* MPI_T Sampling object instantiation, per thread */
mpi_t_sampling mpi_t_sampling_object;

#if 0
/*
  Override / Hook for MPI_Init() function
*/
int __MPI_Init(int *argc, char ***argv)
{
   return mpi_t_sampling_object.MPI_Init(argc, argv);
}

int __MPI_Finalize(void)
{
	return mpi_t_sampling_object.MPI_Finalize();
}

#endif
