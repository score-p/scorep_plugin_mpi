//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014, Lawrence Livermore National Security,
//  LLC. Produced at the Lawrence Livermore National Laboratory. Written
//  by Tanzima Z. Islam (islam3@llnl.gov).
// CODE-LLNL-CODE-647221. All rights reserved.
// This file is part of mpi_T-tools. For details, see
//  https://computation-rnd.llnl.gov/mpi_t/gyan.php.
// Please also read this file - FULL-LICENSE.txt.
// This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License (as published by
//  the Free Software Foundation) version 2.1 dated February 1999.
// This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the IMPLIED WARRANTY OF
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the terms
//  and conditions of the GNU General Public License for more
//  details. You should have received a copy of the GNU Lesser General
//  Public License along with this program; if not, write to the Free
//  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
//  02111-1307 USA
//////////////////////////////////////////////////////////////////////////////
/*
 * gyan.c

 *
 *  Created on: Jul 11, 2013
 *      Author: Tanzima Z. Islam, (islam3@llnl.gov)
 */

/**
* Copyright (C) Huawei Technologies Co., Ltd. 2020.  ALL RIGHTS RESERVED.
*/

#include "utility.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <iostream>
#include <sstream>

#include "mpi_t_sampling.h"
#include "mpi_t_sampling_overrides.h"


/*
  Shuki: Temporary, until we resolve the issue of MPI_Init() called after
		 get_metric_properties() so enumeration cannot take place.
*/
const static mpi_t_counters mpi_t_counters_enum_const[] = {
		{ 0, "mpool_hugepage_bytes_allocated" },
		{ 20, "ompi_mpi_api_allreduce_same_sendbuf" },
		{ 21, "ompi_mpi_api_allreduce_same_recvbuf" },
		{ 22, "ompi_mpi_api_reduce_same_sendbuf" },
		{ 23, "ompi_mpi_api_reduce_same_recvbuf" },
		{ 24, "ompi_mpi_api_p2p_to_local_rank_cnt" },
		{ 25, "ompi_mpi_api_p2p_to_remote_rank_cnt" },
		{ 26, "ompi_mpi_api_bcast_last_buffer_ptr_used" },
		{ 27, "ompi_mpi_api_gatherv_last_send_size_used" }
};


void
mpi_t_sampling::pvars_enumeration_get(const mpi_t_counters **pvars, size_t *n_pvars)
{
	*pvars = mpi_t_counters_enum_const;
	*n_pvars = sizeof(mpi_t_counters_enum_const) / sizeof(mpi_t_counters_enum_const[0]);
}

/* Constructor */
mpi_t_sampling::mpi_t_sampling()
{
}

/* Destructor */
mpi_t_sampling::~mpi_t_sampling()
{
}


void
mpi_t_sampling::stop_watching(){
   int i;
   for (i = 0; i < total_num_of_var; i++) {
	  if (pvar_handles[i] != NULL) {
		  MPI_T_pvar_stop(session, pvar_handles[i]);
		  MPI_T_pvar_handle_free(session, &pvar_handles[i]);
	  }
   }
   MPI_T_pvar_session_free(&session);
}

int
mpi_t_sampling::get_watched_var_index_with_class(
   char *var_name, char *var_class_name )
{
   int i;
   int var_class;
   if( (strcmp(var_class_name, "level") == 0) || (strcmp(var_class_name, "LEVEL") == 0))
      var_class = MPI_T_PVAR_CLASS_LEVEL;
   else if( (strcmp(var_class_name, "highwat") == 0) || (strcmp(var_class_name, "HIGHWAT") == 0))
      var_class = MPI_T_PVAR_CLASS_HIGHWATERMARK;
   else if( (strcmp(var_class_name, "lowwat") == 0) || (strcmp(var_class_name, "LOWWAT") == 0))
      var_class = MPI_T_PVAR_CLASS_LOWWATERMARK;
   else if( (strcmp(var_class_name, "counter") == 0) || (strcmp(var_class_name, "COUNTER") == 0))
      var_class = MPI_T_PVAR_CLASS_COUNTER;
   else if( (strcmp(var_class_name, "state") == 0) || (strcmp(var_class_name, "STATE") == 0))
      var_class = MPI_T_PVAR_CLASS_STATE;
   else if( (strcmp(var_class_name, "size") == 0) || (strcmp(var_class_name, "SIZE") == 0))
      var_class = MPI_T_PVAR_CLASS_SIZE;
   else if( (strcmp(var_class_name, "percent") == 0) || (strcmp(var_class_name, "PERCENT") == 0))
      var_class = MPI_T_PVAR_CLASS_PERCENTAGE;
   else if( (strcmp(var_class_name, "aggr") == 0) || (strcmp(var_class_name, "AGGR") == 0))
      var_class = MPI_T_PVAR_CLASS_AGGREGATE;
   else if( (strcmp(var_class_name, "timer") == 0) || (strcmp(var_class_name, "TIMER") == 0))
      var_class = MPI_T_PVAR_CLASS_TIMER;
   else if( (strcmp(var_class_name, "generic") == 0) || (strcmp(var_class_name, "GENERIC") == 0))
      var_class = MPI_T_PVAR_CLASS_GENERIC;

   //printf("total_num_of_var = %u\n", total_num_of_var);
   for (i = 0; i < total_num_of_var; i++) {
      if (perf_var_all && perf_var_all[i].name) {
         //printf("i = %d: perf_var_all[i].name = %p  ==> ", i, perf_var_all[i].name);
         //printf("perf_var_all[i].name = %s\n", perf_var_all[i].name);
         if ( (var_class == perf_var_all[i].var_class) &&
              (strcmp(var_name, perf_var_all[i].name) == 0) )
            return i;
      }
   }

   return NOT_FOUND;
}


int
mpi_t_sampling::get_watched_var_index(char *var_name)
{
   int i;
   char *pch = strchr(var_name, ':');

   if (pch != NULL) {
      /* If the class is specified */
      (*pch) = 0;
      return get_watched_var_index_with_class(var_name, (pch+1));
   }

   /* No class was specified */
   for(i = 0; i < total_num_of_var; i++) {
      if (strcmp(var_name, perf_var_all[i].name) == 0) {
         return i;
      }
   }

   return NOT_FOUND;
}


void mpi_t_sampling::print_pvar_buffer_all()
{
   int i;
   int j;
   int index;

   printf("%-40s\tType   ", "Variable Name");
   printf(" Minimum(Rank)    Maximum(Rank)       Average\n");
   for (i = 0; i < pvar_num_watched; i++) {
      index = pvar_index[i];
      if (perf_var_all[index].var_class != MPI_T_PVAR_CLASS_TIMER) {
         for (j = 0; j < pvar_count[i]; j++) {
        	/* asuming that pvar_count[i] on all
        	 *  processes was the same */
            printf("%-40s\t", perf_var_all[index].name);
            print_class(perf_var_all[index].var_class);
            printf("\t%8llu(%3d)  %8llu(%3d)  %12.2lf\n", pvar_stat[i][j].min,
               pvar_stat[i][j].min_rank,
               pvar_stat[i][j].max, pvar_stat[i][j].max_rank,
               (pvar_stat[i][j].total / (double)num_mpi_tasks));
         }
      }
   }
}


void mpi_t_sampling::pvar_read_all()
{
   int i, j;
   int size;

   for (i = 0; i < total_num_of_var; i++) {
	  if (pvar_handles[i] != NULL) {
		  MPI_T_pvar_read(session, pvar_handles[i], read_value_buffer);

		  MPI_Type_size(perf_var_all[i].datatype, &size);

		  for (j = 0; j < pvar_count[j]; j++) {
			 pvar_value_buffer[i][j] = 0;
			 memcpy(&(pvar_value_buffer[i][j]), read_value_buffer, size);
		  }
	  }
   }

   return;
}


void mpi_t_sampling::clean_up_perf_var_all(
	int num_of_perf_var)
{
   int i;
   for(i = 0; i < num_of_perf_var; i++){
      free(perf_var_all[i].name);
      free(perf_var_all[i].desc);
   }
   free(perf_var_all);
}


void mpi_t_sampling::clean_up_pvar_value_buffer(
	int num_of_var_watched)
{
   int i;
   for(i = 0; i < num_of_var_watched; i++){
      free(pvar_value_buffer[i]);
   }

   free(pvar_value_buffer);
}


void mpi_t_sampling::clean_up_the_rest()
{
   free(read_value_buffer);
   free(pvar_handles);
   free(pvar_index);
   free(pvar_count);
}


void mpi_t_sampling::collect_sum_from_all_ranks(MPI_Op op)
{
   int i;
   int j;
   int root = 0;

   unsigned long long int *value_in;
   unsigned long long int *value_out;
   value_in = (unsigned long long int*)malloc(sizeof(unsigned long long int) * ( max_num_of_state_per_pvar + 1));
   if(rank == root)
      value_out = (unsigned long long int*)malloc(sizeof(unsigned long long int) * ( max_num_of_state_per_pvar + 1));

   for(i = 0; i < pvar_num_watched; i++){
      for(j = 0; j < pvar_count[i]; j++){
         value_in[j] = pvar_value_buffer[i][j];
      }
      PMPI_Reduce(value_in, value_out, pvar_count[i] /*number_of_elements*/, MPI_UNSIGNED_LONG_LONG, op, root, MPI_COMM_WORLD);
      if(root == rank){
         /**
          * Populate these values into the pvar_stat array of rank 0.
          */
         for(j = 0; j < pvar_count[i]; j++){
            pvar_stat[i][j].total = value_out[j];
         }
      }// root is done extracting information from "out" to pvar_stat
   }

   free(value_in);

   if(root == rank) {
      free(value_out);
   }
}


void mpi_t_sampling::collect_range_with_loc_from_all_ranks(MPI_Op op)
{
   int i;
   int j;
   int root = 0;
   mpi_data *in;
   mpi_data *out;

   /**
    * Pack all variables into a buffer
    */
   /**
    * MPI_Reduce on each element. Each element here is a performance variable
    */
   in = (mpi_data*)malloc(sizeof(mpi_data) * ( max_num_of_state_per_pvar + 1));
   if(rank == root)
      out = (mpi_data*)malloc(sizeof(mpi_data) * (max_num_of_state_per_pvar + 1));
   for(i = 0; i < pvar_num_watched; i++){
      for(j = 0; j < pvar_count[i]; j++){
         in[j].value = 0;
         in[j].value = (double)(pvar_value_buffer[i][j]);
         in[j].rank = rank;
      }
      PMPI_Reduce(in, out, pvar_count[i] /*number_of_elements*/, MPI_DOUBLE_INT, op, root, MPI_COMM_WORLD);
      //printout
      if(root == rank){
         for(j = 0; j < pvar_count[i]; j++){
            if(op == MPI_MINLOC){
               pvar_stat[i][j].min = (unsigned long long int)(out[j].value);
               pvar_stat[i][j].min_rank = out[j].rank;
            }
            else if(op == MPI_MAXLOC){
               pvar_stat[i][j].max = (unsigned long long int)(out[j].value);
               pvar_stat[i][j].max_rank = out[j].rank;
            }
         }
      }
   }
   free(in);
   if(root == rank)
      free(out);
}


/*
  Override / Hook for MPI_Init() function
*/
int mpi_t_sampling::MPI_Init(int *argc, char ***argv)
{
   int mpi_init_return;

   DEBUG_PRINT("********** Interception starts **********\n");

   printf("MPI_Init(): called = %d\n", m_mpi_init_called);
   if (m_mpi_init_called == 0) {
	   m_mpi_init_called = 1;
	   /* Run MPI Initialization */
	   mpi_init_return = PMPI_Init(argc, argv);
	   if (mpi_init_return != MPI_SUCCESS) {
		  printf("PMPI_Init() - Failed err=%d\n", mpi_init_return);
		  return mpi_init_return;
	   }
   }

   return 0;
}


int mpi_t_sampling::MPI_T_pvars_enumerate()
{
   int err, num, i, namelen, verb, varclass, bind;
   int threadsup;
   int index;
   int readonly, continuous, atomic;
   char name[STR_SZ + 1] = "";
   int desc_len;
   char desc[STR_SZ + 1] = "";
   MPI_Datatype datatype;
   MPI_T_enum enumtype;
   int total_length_pvar_name = 0;
   size_t sz;

   /* get global rank */
   PMPI_Comm_rank(MPI_COMM_WORLD, &rank);

   /* get number of tasks*/
   PMPI_Comm_size(MPI_COMM_WORLD, &num_mpi_tasks);

   /* Run MPI_T Initialization */
   err = MPI_T_init_thread(MPI_THREAD_SINGLE, &threadsup);
   if (err != MPI_SUCCESS) {
	  printf("MPI_T_init_thread() - Failed err=%d\n", err);
      return err;
   }

   /* Print thread support for MPI */
   if(!rank) {
	  DEBUG_PRINT("MPI_T Thread support: ");
      switch (threadsup) {
      case MPI_THREAD_SINGLE:
     	 DEBUG_PRINT("MPI_THREAD_SINGLE\n");
         break;
      case MPI_THREAD_FUNNELED:
    	 DEBUG_PRINT("MPI_THREAD_FUNNELED\n");
         break;
      case MPI_THREAD_SERIALIZED:
    	 DEBUG_PRINT("MPI_THREAD_SERIALIZED\n");
         break;
      case MPI_THREAD_MULTIPLE:
    	 DEBUG_PRINT("MPI_THREAD_MULTIPLE\n");
         break;
      default:
         printf("unknown (%i)\n",threadsup);
      }
   }

   /* Create a session */
   err = MPI_T_pvar_session_create(&session);
   if (err != MPI_SUCCESS) {
	  printf("MPI_T_pvar_session_create() - Failed err=%d\n", err);
      return err;
   }

   /* Get number of variables */
   err = MPI_T_pvar_get_num(&num);
   if(!rank) {
      printf("%d performance variables exposed by this MPI library\n",num);
   }
   if (err != MPI_SUCCESS) {
	  printf("MPI_T_pvar_get_num() - Failed err=%d\n", err);
      return err;
   }

   /* Total number of variables */
   total_num_of_var = num;

   /* Get the name of the environment variable to look for */
   env_var_name = getenv("MPIT_VAR_TO_TRACE");
   int set_default = 0;
   if (env_var_name != NULL) {
      DEBUG_PRINT("Environment variable set: %s\n", env_var_name);

      if(strlen(env_var_name) == 0) {
         set_default = 1;
      }
   }
   else{
      set_default = 1;
   }

   /* Allocate handles for all performance variables*/
   pvar_handles = (MPI_T_pvar_handle *)malloc(sizeof(MPI_T_pvar_handle) * (num + 1));
   pvar_index = (int*)malloc(sizeof(int) * (num + 1));
   pvar_count = (int*)malloc(sizeof(int) * (num + 1));
   memset(pvar_count, 0, sizeof(int) * (num + 1));

   sz = sizeof(PERF_VAR) * (num + 1);
   perf_var_all = (PERF_VAR *)malloc(sz);
   memset(perf_var_all, 0x00, sz);

   sz = sizeof(STATISTICS *) * (num + 1);
   pvar_stat = (STATISTICS **)malloc(sz);
   memset(pvar_stat, 0x00, sz);
   
   total_length_pvar_name = 0;
   for (i = 0; i < num; i++) {
      namelen = desc_len = STR_SZ;

      /* Prevent duplication of name due to error */
      name[0] = char(0x00);

      /* Get PVAR in index i */
      err = MPI_T_pvar_get_info(i/*IN*/,
            name /*OUT*/,
            &namelen /*INOUT*/,
            &verb /*OUT*/,
            &varclass /*OUT*/,
            &datatype /*OUT*/,
            &enumtype /*OUT*/,
            desc /*desc: OUT*/,
            &desc_len /*desc_len: INOUT*/,
            &bind /*OUT*/,
            &readonly /*OUT*/,
            &continuous /*OUT*/,
            &atomic/*OUT*/);
      if (err != MPI_SUCCESS) {
    	  perf_var_all[i].pvar_valid = 0;
    	  DEBUG_PRINT("[%d] err=%d\n", i, err);
    	  /* Next iteration */
    	  continue;
      }
      else {
    	  perf_var_all[i].pvar_valid = 1;
      }

      perf_var_all[i].pvar_index = -1; // gets setup later
      perf_var_all[i].name_len = namelen;
      perf_var_all[i].name = (char *)malloc(sizeof(char) * (namelen + 1));
      strcpy(perf_var_all[i].name, name);

      /* Shuki: Debug */
      DEBUG_PRINT("[%d] err=%d pvar name detected: %s, bind=%d\n", i, err, name, bind);

      total_length_pvar_name += namelen;

      perf_var_all[i].verbosity = verb;
      perf_var_all[i].var_class = varclass;
      perf_var_all[i].datatype = datatype;
      perf_var_all[i].enumtype = enumtype;
      perf_var_all[i].desc_len = desc_len;
      perf_var_all[i].desc = (char *)malloc(sizeof(char) * (desc_len + 1));
      strcpy(perf_var_all[i].desc, desc);
      perf_var_all[i].binding = bind;
      perf_var_all[i].readonly = readonly;
      perf_var_all[i].continuous = continuous;
      perf_var_all[i].atomic = atomic;
   }

   /* Enumerate all MPI_T variables */
   if (set_default == 1) {
      /*By default, watch all variables in the list.*/
      //env_var_name = get_pvars_name_list();
      /* Allocate string buffers */
	  size_t size_to_alloc =
		  sizeof(char)* (total_length_pvar_name + num * 8 /*strlen(:CLASS_NAME)*/ +
		  num /*delimiter*/ + 1);
      env_var_name = (char *)malloc(size_to_alloc);
      int index = 0;
      const char *class_name;
      for (i = 0; i < num; i++)
      {
    	 if (perf_var_all[i].pvar_valid) {
            memcpy((env_var_name + index), perf_var_all[i].name,
           		strlen(perf_var_all[i].name));
            index += (strlen(perf_var_all[i].name) );

            memcpy((env_var_name + index), ":", strlen(":"));
            index += (strlen(":"));

            class_name = get_pvar_class(perf_var_all[i].var_class);
            memcpy((env_var_name + index), class_name, strlen(class_name));
            index += (strlen(class_name));

            memcpy((env_var_name + index), ";", strlen(";"));
            index += (strlen(";"));
    	 }
      }
      env_var_name[index] = 0;
   }

   /* Now, start session for those variables in the watchlist*/
   DEBUG_PRINT("Scanning MPI_T counters...\n");
   DEBUG_PRINT("==========================\n");

   pvar_num_watched = 0;
   char *p = strtok(env_var_name, ";");
   pvar_value_buffer =
      (unsigned long long int **)malloc(sizeof(unsigned long long int*) * (num + 1));
   int max_count = -1;
   int k;
   while (p != NULL) {
      index = get_watched_var_index(p);
      DEBUG_PRINT("[%d] p = %s\n", index, p);
      if (index != NOT_FOUND) {
    	 void *object_handle = NULL;
         pvar_index[pvar_num_watched] = index;
         perf_var_all[index].pvar_index = pvar_num_watched;
#if 0 /* Shuki: TODO - Need to check this issue */
         if (perf_var_all[i].binding == MPI_T_BIND_MPI_WIN) {
        	 object_handle = MPI_COMM_WORLD;
         }
#endif
         err = MPI_T_pvar_handle_alloc(session, index, object_handle,
        		 &pvar_handles[index],
        		 &pvar_count[pvar_num_watched]);
         DEBUG_PRINT("MPI_T_pvar_handle_alloc(): err = %d handle=%p\n",
       		 err, pvar_handles[index]);
         if (err == MPI_SUCCESS) {
        	size_t size1;
        	size_t size2;

        	size1 = sizeof(unsigned long long int) * (pvar_count[pvar_num_watched] + 1);
            pvar_value_buffer[pvar_num_watched] =
            	(unsigned long long int *)malloc(size1);

        	size2 = sizeof(STATISTICS) * (pvar_count[pvar_num_watched] + 1);
            pvar_stat[pvar_num_watched] =
            	(STATISTICS *)malloc(size2);

            memset(pvar_value_buffer[pvar_num_watched], 0,
            	sizeof(unsigned long long int) * pvar_count[pvar_num_watched]);

            for (k = 0; k < pvar_count[pvar_num_watched]; k++){
               pvar_value_buffer[pvar_num_watched][k] = 0;
               pvar_stat[pvar_num_watched][k].max = NEG_INF;
               pvar_stat[pvar_num_watched][k].min = POS_INF;
               pvar_stat[pvar_num_watched][k].total = 0;

            }

            if(max_count < pvar_count[pvar_num_watched]) {
               max_count = pvar_count[pvar_num_watched];
            }

            if(perf_var_all[index].continuous == 0){
               err = MPI_T_pvar_start(session, pvar_handles[index]);
            }

            if (err != MPI_SUCCESS) {
               printf("MPI_T_pvar_start() - Failed, err=%d", err);
               return err;
            }

            pvar_num_watched++;
         }
      }
      p = strtok(NULL, ";");
   }

   read_value_buffer = (void *)malloc(sizeof(unsigned long long int) * (max_count + 1));
   max_num_of_state_per_pvar = max_count;

   DEBUG_PRINT("Finished 1\n");

   assert(num >= pvar_num_watched);
   assert(pvar_value_buffer != NULL);
   /* iterate unit variable is found */
   tool_enabled = TRUE;
   num_send = 0;
   num_isend = 0;
   num_recv = 0;
   num_irecv = 0;

   DEBUG_PRINT("Finished 2\n");

   return 0;
}


int mpi_t_sampling::MPI_Finalize(void)
{
   pvar_read_all();
   /**
    * Collect statistics from all ranks onto root
    */
   collect_range_with_loc_from_all_ranks(MPI_MINLOC);
   collect_range_with_loc_from_all_ranks(MPI_MAXLOC);
   collect_sum_from_all_ranks(MPI_SUM);

   DEBUG_PRINT("%s:%d rank = %d\n", __func__, __LINE__, rank);

   if (rank == 0) {
      DEBUG_PRINT("Performance profiling for the complete MPI job:\n");
      print_pvar_buffer_all();
   }
   stop_watching();
   clean_up_perf_var_all(total_num_of_var);
   clean_up_pvar_value_buffer(pvar_num_watched);
   clean_up_the_rest();

   DEBUG_PRINT("%s:%d rank = %d\n", __func__, __LINE__, rank);

   PMPI_Barrier(MPI_COMM_WORLD);

   MPI_T_finalize();

   DEBUG_PRINT("%s:%d rank = %d\n", __func__, __LINE__, rank);

   return PMPI_Finalize();
}


uint64_t
mpi_t_sampling::MPI_T_pvar_current_value_get(int32_t index)
{
#if 0
   int i, j;
   int size;

   for (i = 0; i < pvar_num_watched; i++) {
      MPI_T_pvar_read(session, pvar_handles[i], read_value_buffer);

      MPI_Type_size(perf_var_all[i].datatype, &size);

      for (j = 0; j < pvar_count[j]; j++) {
         pvar_value_buffer[i][j] = 0;
         memcpy(&(pvar_value_buffer[i][j]), read_value_buffer, size);
      }
   }
#else
   //std::cout << "MPI_T_pvar_current_value_get(index = " << index <<
   //	   "), pvar_handles[index]=" <<pvar_handles[index] << "\n";
   DEBUG_PRINT("MPI_T_pvar_current_value_get() - index=%d: pvar_handles[index]=%p\n",
	   index, pvar_handles[index]);

   MPI_T_pvar_read(session, pvar_handles[index], read_value_buffer);

#endif

   return ((uint64_t *)read_value_buffer)[0];
}

int
mpi_t_sampling::MPI_Initialized(int *flag)
{
	return ::MPI_Initialized(flag);
}
