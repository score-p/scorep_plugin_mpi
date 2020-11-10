/**
* Copyright (C) Huawei Technologies Co., Ltd. 2020.  ALL RIGHTS RESERVED.
*/

#include <mpi.h> /* Adds MPI_T definiKons as well */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "mpi_t_sampling_overrides.h"
#include "utility.h"

/* Fix for error: libgyan.so: undefined symbol: __gxx_personality_v0 */
void *__gxx_personality_v0;

#if 0
/*
   Hook for MPI_Init_thread() function
*/
int MPI_Init_thread(int *argc, char ***argv, int required, int *provided)
{
   printf("MPI_Init_thread()\n");   
   return __MPI_Init(argc, argv);
}
#endif

#if 0
/*
   Hook for MPI_Init() function
*/
int MPI_Init(int *argc, char ***argv)
{
   printf("MPI_Init()\n");
   return __MPI_Init(argc, argv);
}

int MPI_Finalize(void)
{
   printf("MPI_Finalize()\n");
   return __MPI_Finalize();
}

#endif
