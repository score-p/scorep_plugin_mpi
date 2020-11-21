/**
* Copyright (C) Huawei Technologies Co., Ltd. 2020.  ALL RIGHTS RESERVED.
*/
#if !defined(_MPI_T_SAMPLING_H_)
#define _MPI_T_SAMPLING_H_

#include <stdint.h>

#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <mpi.h> /* Adds MPI_T definitions as well */
#ifdef __cplusplus
}
#endif

#define THRESHOLD 						0
#define NOT_FOUND 						-1
#define FALSE 							0
#define TRUE 							1
#define STR_SZ 							256
#define NUM_PERF_VAR_SUPPORTED			50

#define NEG_INF							-10000000
#define POS_INF							10000000

/**********************************/
/* Performance variable structure */
/**********************************/
typedef struct{
   int pvar_index;
   char *name;
   int name_len;
   int verbosity;
   int var_class;
   MPI_Datatype datatype;
   MPI_T_enum enumtype;
   char *desc;
   int desc_len;
   int binding;
   int readonly;
   int continuous;
   int atomic;
   int pvar_valid;
} PERF_VAR;


/************************/
/* Statistics structure */
/************************/
typedef struct{
   /* max_rank : rank that resulted in this max value */
   int max_rank;
   /* min_rank: rank that resulted in this min value */
   int min_rank;
   /* the actual max value */
   unsigned long long int max;
   /* the actual min value */
   unsigned long long int min;
   /* summation of values across all MPI ranks for this particular variable */
   unsigned long long int total;
   /* how many ranks had this value? */
   //   unsigned long long int freq;
} STATISTICS;


/**********************/
/* MPI data structure */
/**********************/
typedef struct {
   double value;
   int rank;
} mpi_data;

/* A structure for enumerating the MPI_T counters */
typedef struct {
	/* Index of the MPI_T counter */
	unsigned int counter_index;

	/* Name of the counter*/
	const char   *counter_name;

} mpi_t_counters;


/*********************************/
/* Main class for MPI_T Sampling */
/*********************************/
class mpi_t_sampling {
public:
   /* Constructor */
   mpi_t_sampling();

   /* Destructor */
   ~mpi_t_sampling();

   /* Stop watching all counters */
   void stop_watching();

   /* Get the variable index from var_name + class */
   int
   get_watched_var_index_with_class(char *var_name,
      char *var_class_name);

   /* Get the variable index from var_name */
   int get_watched_var_index(char *var_name);

   /**
    * This function prints statistics of all
    * performance variables monitored
    * @param none
    * @return none
    */
   void print_pvar_buffer_all();

   /* Reads all PVAR variables */
   void pvar_read_all();

   /* Cleans up all variables and frees them */
   void clean_up_perf_var_all(int num_of_perf_var);

   /* Cleans up and frees pvar_value_buffer */
   void
   clean_up_pvar_value_buffer(int num_of_var_watched);

   /* Cleans up the rest of the variables */
   void clean_up_the_rest();

   /* Employ AllReduce to sum MPI variables values
    * from all ranks */
   void collect_sum_from_all_ranks(MPI_Op op);

   void collect_range_with_loc_from_all_ranks(MPI_Op op);

   /* Overrides MPI_Init() and calls MPI_T_pvars_enumerate() */
   int MPI_Init(int *argc, char ***argv);

   /* Enumerates & Initializes measurement of MPI_T
       * variables */
   int MPI_T_pvars_enumerate();

   /* Final clean-up */
   int MPI_Finalize();

   /* Get enumeration of MPI_T counters */
   void pvars_enumeration_get(const mpi_t_counters **pvars, size_t *n_pvars);

   /* Read current value of a PVAR */
   uint64_t MPI_T_pvar_current_value_get(int32_t index);

   int MPI_Initialized(int *flag);

   int mpi_rank_get();

private:
   /* PVAR session */
   MPI_T_pvar_session session;

   /* PVAR handles */
   MPI_T_pvar_handle *pvar_handles;

   int *pvar_index;
   int *pvar_count;

   /* cumulative values */
   unsigned long long int **pvar_value_buffer;

   /* values are read into this buffer */
   void *read_value_buffer;

   /* Number of watched variables */
   int pvar_num_watched;

   /* Total number of MPI variables */
   int total_num_of_var;

   int max_num_of_state_per_pvar = -1; //  max_num_of_state_per_pvar = max(pvar_count[performance_variable]) for all performance_variables

   /* Number of MPI tasks */
   int num_mpi_tasks;

   PERF_VAR *perf_var_all;
   STATISTICS **pvar_stat;

   char *env_var_name;
   int tool_enabled = FALSE;
   int num_send, num_isend, num_recv, num_irecv;

   /* Rank */
   int rank = 0;

   /* MPI init already called */
   int m_mpi_init_called;
};

inline int
mpi_t_sampling::mpi_rank_get()
{
    return this->rank;
}

#endif /* _MPI_T_SAMPLING_H_ */
