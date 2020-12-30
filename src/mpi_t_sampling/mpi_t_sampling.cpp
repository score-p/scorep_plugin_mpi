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

//#define MPI_T_STATIC_COUNTERS_ENABLE

#if defined(MPI_T_STATIC_COUNTERS_ENABLE)
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
#endif


void
mpi_t_sampling::pvars_enumeration_get(const mpi_t_counters **pvars, size_t *n_pvars)
{
    /* Enumeration was successful? */
    if (m_mpi_t_counters_enum_n_objects > 0) {
        *pvars = m_mpi_t_counters_enum;
        *n_pvars = m_mpi_t_counters_enum_n_objects;
    }
#if defined(MPI_T_STATIC_COUNTERS_ENABLE)
    else {
        *pvars = mpi_t_counters_enum_const;
        *n_pvars = sizeof(mpi_t_counters_enum_const) / sizeof(mpi_t_counters_enum_const[0]);
    }
#else
    else {
        *pvars = NULL;
        *n_pvars = 0;
    }
#endif
}

/* Constructor */
mpi_t_sampling::mpi_t_sampling()
{
    m_mpi_t_counters_enum = NULL;
    m_mpi_t_counters_enum_n_objects = 0;

    /* PVAR handles */
    m_pvar_handles = NULL;

    m_pvar_index = NULL;
    m_pvar_count = NULL;

    m_pvar_value_buffer = NULL;

    m_read_value_buffer = NULL;

    m_pvar_num_watched = 0;

    m_total_num_of_var = 0;

    m_max_num_of_state_per_pvar = -1;

    /* Number of MPI tasks */
    m_num_mpi_tasks = 0;

    m_perf_var_all = NULL;
    m_pvar_stat = NULL;
}

#define MPI_T_MALLOC_FREE(iDENTIFIER) \
    if (iDENTIFIER) { free(iDENTIFIER); iDENTIFIER = NULL; }

/* Destructor */
mpi_t_sampling::~mpi_t_sampling()
{
    int i;

    MPI_T_MALLOC_FREE(m_mpi_t_counters_enum);
    MPI_T_MALLOC_FREE(m_pvar_handles);

    MPI_T_MALLOC_FREE(m_pvar_index);
    MPI_T_MALLOC_FREE(m_pvar_count);

    if (m_perf_var_all) {
        for (i = 0; i < m_total_num_of_var; i++) {
            MPI_T_MALLOC_FREE(m_perf_var_all[i].name);
            MPI_T_MALLOC_FREE(m_perf_var_all[i].desc);
        }
        MPI_T_MALLOC_FREE(m_perf_var_all);
    }

    MPI_T_MALLOC_FREE(m_env_var_name);
    MPI_T_MALLOC_FREE(m_read_value_buffer);

    if (m_pvar_value_buffer) {
        for (i = 0; i < m_pvar_num_watched; i++) {
            MPI_T_MALLOC_FREE(m_pvar_value_buffer[i]);
        }
        MPI_T_MALLOC_FREE(m_pvar_value_buffer);
    }

    if (m_pvar_stat) {
        for (i = 0; i < m_pvar_num_watched; i++) {
            MPI_T_MALLOC_FREE(m_pvar_stat[i]);
        }
        MPI_T_MALLOC_FREE(m_pvar_stat);
    }

    if (m_mpi_t_counters_enum) {
        for (i = 0; i < m_mpi_t_counters_enum_n_objects; i++) {
            MPI_T_MALLOC_FREE(m_mpi_t_counters_enum[i].counter_name);
        }
        MPI_T_MALLOC_FREE(m_mpi_t_counters_enum);
    }
}


void
mpi_t_sampling::stop_watching(){
   int i;
   for (i = 0; i < m_total_num_of_var; i++) {
	  if (m_pvar_handles[i] != NULL) {
		  MPI_T_pvar_stop(m_session, m_pvar_handles[i]);
		  MPI_T_pvar_handle_free(m_session, &m_pvar_handles[i]);
	  }
   }
   MPI_T_pvar_session_free(&m_session);
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

   //printf("m_total_num_of_var = %u\n", m_total_num_of_var);
   for (i = 0; i < m_total_num_of_var; i++) {
      if (m_perf_var_all && m_perf_var_all[i].name) {
         //printf("i = %d: m_perf_var_all[i].name = %p  ==> ", i, m_perf_var_all[i].name);
         //printf("m_perf_var_all[i].name = %s\n", m_perf_var_all[i].name);
         if ( (var_class == m_perf_var_all[i].var_class) &&
              (strcmp(var_name, m_perf_var_all[i].name) == 0) )
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
   for(i = 0; i < m_total_num_of_var; i++) {
      if (strcmp(var_name, m_perf_var_all[i].name) == 0) {
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
   for (i = 0; i < m_pvar_num_watched; i++) {
      index = m_pvar_index[i];
      if (m_perf_var_all[index].var_class != MPI_T_PVAR_CLASS_TIMER) {
         for (j = 0; j < m_pvar_count[i]; j++) {
        	/* asuming that m_pvar_count[i] on all
        	 *  processes was the same */
            printf("%-40s\t", m_perf_var_all[index].name);
            print_class(m_perf_var_all[index].var_class);
            printf("\t%8llu(%3d)  %8llu(%3d)  %12.2lf\n", m_pvar_stat[i][j].min,
               m_pvar_stat[i][j].min_rank,
               m_pvar_stat[i][j].max, m_pvar_stat[i][j].max_rank,
               (m_pvar_stat[i][j].total / (double)m_num_mpi_tasks));
         }
      }
   }
}


void mpi_t_sampling::pvar_read_all()
{
   int i, j;
   int size;

   for (i = 0; i < m_total_num_of_var; i++) {
	  if (m_pvar_handles[i] != NULL) {
		  MPI_T_pvar_read(m_session, m_pvar_handles[i], m_read_value_buffer);

		  MPI_Type_size(m_perf_var_all[i].datatype, &size);

		  for (j = 0; j < m_pvar_count[j]; j++) {
			 m_pvar_value_buffer[i][j] = 0;
			 memcpy(&(m_pvar_value_buffer[i][j]), m_read_value_buffer, size);
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
      free(m_perf_var_all[i].name);
      free(m_perf_var_all[i].desc);
   }
   free(m_perf_var_all);
}


void mpi_t_sampling::clean_up_pvar_value_buffer(
	int num_of_var_watched)
{
   int i;
   for(i = 0; i < num_of_var_watched; i++){
      free(m_pvar_value_buffer[i]);
   }

   free(m_pvar_value_buffer);
}


void mpi_t_sampling::clean_up_the_rest()
{
   free(m_read_value_buffer);
   free(m_pvar_handles);
   free(m_pvar_index);
   free(m_pvar_count);
}


void mpi_t_sampling::collect_sum_from_all_ranks(MPI_Op op)
{
   int i;
   int j;
   int root = 0;

   unsigned long long int *value_in;
   unsigned long long int *value_out;
   value_in = (unsigned long long int*)malloc(sizeof(unsigned long long int) * ( m_max_num_of_state_per_pvar + 1));
   if(m_rank == root)
      value_out = (unsigned long long int*)malloc(sizeof(unsigned long long int) * ( m_max_num_of_state_per_pvar + 1));

   for(i = 0; i < m_pvar_num_watched; i++){
      for(j = 0; j < m_pvar_count[i]; j++){
         value_in[j] = m_pvar_value_buffer[i][j];
      }
      PMPI_Reduce(value_in, value_out, m_pvar_count[i] /*number_of_elements*/, MPI_UNSIGNED_LONG_LONG, op, root, MPI_COMM_WORLD);
      if(root == m_rank){
         /**
          * Populate these values into the m_pvar_stat array of rank 0.
          */
         for(j = 0; j < m_pvar_count[i]; j++){
            m_pvar_stat[i][j].total = value_out[j];
         }
      }// root is done extracting information from "out" to m_pvar_stat
   }

   free(value_in);

   if(root == m_rank) {
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
   in = (mpi_data*)malloc(sizeof(mpi_data) * ( m_max_num_of_state_per_pvar + 1));
   if(m_rank == root)
      out = (mpi_data*)malloc(sizeof(mpi_data) * (m_max_num_of_state_per_pvar + 1));
   for(i = 0; i < m_pvar_num_watched; i++){
      for(j = 0; j < m_pvar_count[i]; j++){
         in[j].value = 0;
         in[j].value = (double)(m_pvar_value_buffer[i][j]);
         in[j].rank = m_rank;
      }
      PMPI_Reduce(in, out, m_pvar_count[i] /*number_of_elements*/, MPI_DOUBLE_INT, op, root, MPI_COMM_WORLD);
      //printout
      if(root == m_rank){
         for(j = 0; j < m_pvar_count[i]; j++){
            if(op == MPI_MINLOC){
               m_pvar_stat[i][j].min = (unsigned long long int)(out[j].value);
               m_pvar_stat[i][j].min_rank = out[j].rank;
            }
            else if(op == MPI_MAXLOC){
               m_pvar_stat[i][j].max = (unsigned long long int)(out[j].value);
               m_pvar_stat[i][j].max_rank = out[j].rank;
            }
         }
      }
   }
   free(in);
   if(root == m_rank)
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
   PMPI_Comm_rank(MPI_COMM_WORLD, &m_rank);

   /* get number of tasks*/
   PMPI_Comm_size(MPI_COMM_WORLD, &m_num_mpi_tasks);

   /* Run MPI_T Initialization */
   err = MPI_T_init_thread(MPI_THREAD_SINGLE, &threadsup);
   if (err != MPI_SUCCESS) {
	  printf("MPI_T_init_thread() - Failed err=%d\n", err);
      return err;
   }

   /* Print thread support for MPI */
   if(!m_rank) {
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

   /* Create a m_session */
   err = MPI_T_pvar_session_create(&m_session);
   if (err != MPI_SUCCESS) {
	  printf("MPI_T_pvar_m_session_create() - Failed err=%d\n", err);
      return err;
   }

   /* Get number of variables */
   err = MPI_T_pvar_get_num(&num);
   if(!m_rank) {
      printf("%d performance variables exposed by this MPI library\n",num);
   }
   if (err != MPI_SUCCESS) {
	  printf("MPI_T_pvar_get_num() - Failed err=%d\n", err);
      return err;
   }

   /* Total number of variables */
   m_total_num_of_var = num;

   /* Get the name of the environment variable to look for */
   m_env_var_name = getenv("MPIT_VAR_TO_TRACE");
   int set_default = 0;
   if (m_env_var_name != NULL) {
      DEBUG_PRINT("Environment variable set: %s\n", m_env_var_name);

      if(strlen(m_env_var_name) == 0) {
         set_default = 1;
      }
   }
   else{
      set_default = 1;
   }

   /* Allocate handles for all performance variables*/
   m_pvar_handles = (MPI_T_pvar_handle *)malloc(sizeof(MPI_T_pvar_handle) * (num + 1));
   m_pvar_index = (int*)malloc(sizeof(int) * (num + 1));
   m_pvar_count = (int*)malloc(sizeof(int) * (num + 1));
   memset(m_pvar_count, 0, sizeof(int) * (num + 1));

   sz = sizeof(PERF_VAR) * (num + 1);
   m_perf_var_all = (PERF_VAR *)malloc(sz);
   memset(m_perf_var_all, 0x00, sz);

   sz = sizeof(STATISTICS *) * (num + 1);
   m_pvar_stat = (STATISTICS **)malloc(sz);
   memset(m_pvar_stat, 0x00, sz);
   
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
    	  m_perf_var_all[i].pvar_valid = 0;
    	  DEBUG_PRINT("[%d] err=%d\n", i, err);
    	  /* Next iteration */
    	  continue;
      }
      else {
    	  m_perf_var_all[i].pvar_valid = 1;
      }

      m_perf_var_all[i].pvar_index = -1; // gets setup later
      m_perf_var_all[i].name_len = namelen;
      m_perf_var_all[i].name = (char *)malloc(sizeof(char) * (namelen + 1));
      strcpy(m_perf_var_all[i].name, name);

      /* Shuki: Debug */
      DEBUG_PRINT("[%d] err=%d pvar name detected: %s, bind=%d\n", i, err, name, bind);

      total_length_pvar_name += namelen;

      m_perf_var_all[i].verbosity = verb;
      m_perf_var_all[i].var_class = varclass;
      m_perf_var_all[i].datatype = datatype;
      m_perf_var_all[i].enumtype = enumtype;
      m_perf_var_all[i].desc_len = desc_len;
      m_perf_var_all[i].desc = (char *)malloc(sizeof(char) * (desc_len + 1));
      strcpy(m_perf_var_all[i].desc, desc);
      m_perf_var_all[i].binding = bind;
      m_perf_var_all[i].readonly = readonly;
      m_perf_var_all[i].continuous = continuous;
      m_perf_var_all[i].atomic = atomic;
   }

   /* Enumerate all MPI_T variables */
   if (set_default == 1) {
      /*By default, watch all variables in the list.*/
      //m_env_var_name = get_pvars_name_list();
      /* Allocate string buffers */
	  size_t size_to_alloc =
		  sizeof(char)* (total_length_pvar_name + num * 8 /*strlen(:CLASS_NAME)*/ +
		  num /*delimiter*/ + 1);
      m_env_var_name = (char *)malloc(size_to_alloc);
      int index = 0;
      const char *class_name;
      for (i = 0; i < num; i++)
      {
    	 if (m_perf_var_all[i].pvar_valid) {
            memcpy((m_env_var_name + index), m_perf_var_all[i].name,
           		strlen(m_perf_var_all[i].name));
            index += (strlen(m_perf_var_all[i].name) );

            memcpy((m_env_var_name + index), ":", strlen(":"));
            index += (strlen(":"));

            class_name = get_pvar_class(m_perf_var_all[i].var_class);
            memcpy((m_env_var_name + index), class_name, strlen(class_name));
            index += (strlen(class_name));

            memcpy((m_env_var_name + index), ";", strlen(";"));
            index += (strlen(";"));
    	 }
      }
      m_env_var_name[index] = 0;
   }

   /* Now, start m_session for those variables in the watchlist*/
   DEBUG_PRINT("Scanning MPI_T counters...\n");
   DEBUG_PRINT("==========================\n");

   m_pvar_num_watched = 0;
   char *p = strtok(m_env_var_name, ";");
   m_pvar_value_buffer =
      (unsigned long long int **)malloc(sizeof(unsigned long long int*) * (num + 1));
   int max_count = -1;
   int k;
   size_t slen;

   m_mpi_t_counters_enum =
       (mpi_t_counters *)malloc(sizeof(*m_mpi_t_counters_enum) * (num + 1));

   while (p != NULL) {
      index = get_watched_var_index(p);
      DEBUG_PRINT("[%d] p = %s\n", index, p);

      slen = strlen(p);

      /* Add counter to list */
      if (slen > 0) {
          m_mpi_t_counters_enum[m_pvar_num_watched].counter_index = index;
          m_mpi_t_counters_enum[m_pvar_num_watched].counter_name = (char *)malloc(slen+1);
          if (m_mpi_t_counters_enum[m_pvar_num_watched].counter_name != NULL) {
              strcpy(m_mpi_t_counters_enum[m_pvar_num_watched].counter_name, p);
          }
      }

      if (index != NOT_FOUND) {
    	 void *object_handle = NULL;
         m_pvar_index[m_pvar_num_watched] = index;
         m_perf_var_all[index].pvar_index = m_pvar_num_watched;
#if 0 /* Shuki: TODO - Need to check this issue */
         if (m_perf_var_all[i].binding == MPI_T_BIND_MPI_WIN) {
        	 object_handle = MPI_COMM_WORLD;
         }
#endif
         err = MPI_T_pvar_handle_alloc(m_session, index, object_handle,
        		 &m_pvar_handles[index],
        		 &m_pvar_count[m_pvar_num_watched]);
         DEBUG_PRINT("MPI_T_pvar_handle_alloc(): err = %d handle=%p\n",
       		 err, m_pvar_handles[index]);
         if (err == MPI_SUCCESS) {
        	size_t size1;
        	size_t size2;

        	size1 = sizeof(unsigned long long int) * (m_pvar_count[m_pvar_num_watched] + 1);
            m_pvar_value_buffer[m_pvar_num_watched] =
            	(unsigned long long int *)malloc(size1);

        	size2 = sizeof(STATISTICS) * (m_pvar_count[m_pvar_num_watched] + 1);
            m_pvar_stat[m_pvar_num_watched] =
            	(STATISTICS *)malloc(size2);

            memset(m_pvar_value_buffer[m_pvar_num_watched], 0,
            	sizeof(unsigned long long int) * m_pvar_count[m_pvar_num_watched]);

            for (k = 0; k < m_pvar_count[m_pvar_num_watched]; k++){
               m_pvar_value_buffer[m_pvar_num_watched][k] = 0;
               m_pvar_stat[m_pvar_num_watched][k].max = NEG_INF;
               m_pvar_stat[m_pvar_num_watched][k].min = POS_INF;
               m_pvar_stat[m_pvar_num_watched][k].total = 0;

            }

            if(max_count < m_pvar_count[m_pvar_num_watched]) {
               max_count = m_pvar_count[m_pvar_num_watched];
            }

            if(m_perf_var_all[index].continuous == 0){
               err = MPI_T_pvar_start(m_session, m_pvar_handles[index]);
            }

            if (err != MPI_SUCCESS) {
               printf("MPI_T_pvar_start() - Failed, err=%d", err);
               return err;
            }

            m_pvar_num_watched++;
         }
      }
      p = strtok(NULL, ";");
   }

   m_read_value_buffer = (void *)malloc(sizeof(unsigned long long int) * (max_count + 1));
   m_max_num_of_state_per_pvar = max_count;

   DEBUG_PRINT("Finished\n");

   m_mpi_t_counters_enum_n_objects = (size_t)m_pvar_num_watched;

   assert(num >= m_pvar_num_watched);
   assert(m_pvar_value_buffer != NULL);
   /* iterate unit variable is found */
   m_tool_enabled = TRUE;
   m_num_send = 0;
   m_num_isend = 0;
   m_num_recv = 0;
   m_num_irecv = 0;

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

   DEBUG_PRINT("%s:%d rank = %d\n", __func__, __LINE__, m_rank);

   if (m_rank == 0) {
      DEBUG_PRINT("Performance profiling for the complete MPI job:\n");
      print_pvar_buffer_all();
   }
   stop_watching();
   clean_up_perf_var_all(m_total_num_of_var);
   clean_up_pvar_value_buffer(m_pvar_num_watched);
   clean_up_the_rest();

   DEBUG_PRINT("%s:%d rank = %d\n", __func__, __LINE__, m_rank);

   PMPI_Barrier(MPI_COMM_WORLD);

   MPI_T_finalize();

   DEBUG_PRINT("%s:%d rank = %d\n", __func__, __LINE__, m_rank);

   return PMPI_Finalize();
}


uint64_t
mpi_t_sampling::MPI_T_pvar_current_value_get(int32_t index)
{
#if 0
   int i, j;
   int size;

   for (i = 0; i < m_pvar_num_watched; i++) {
      MPI_T_pvar_read(m_session, m_pvar_handles[i], m_read_value_buffer);

      MPI_Type_size(m_perf_var_all[i].datatype, &size);

      for (j = 0; j < m_pvar_count[j]; j++) {
         m_pvar_value_buffer[i][j] = 0;
         memcpy(&(m_pvar_value_buffer[i][j]), m_read_value_buffer, size);
      }
   }
#else
   //std::cout << "MPI_T_pvar_current_value_get(index = " << index <<
   //	   "), m_pvar_handles[index]=" <<m_pvar_handles[index] << "\n";
   DEBUG_PRINT("MPI_T_pvar_current_value_get() - index=%d: m_pvar_handles[index]=%p\n",
	   index, m_pvar_handles[index]);

   MPI_T_pvar_read(m_session, m_pvar_handles[index], m_read_value_buffer);

#endif

   return ((uint64_t *)m_read_value_buffer)[0];
}

int
mpi_t_sampling::MPI_Initialized(int *flag)
{
	return ::MPI_Initialized(flag);
}
