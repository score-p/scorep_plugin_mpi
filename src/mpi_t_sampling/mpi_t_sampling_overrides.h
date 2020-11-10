#if !defined(_MPI_T_SAMPLING_OVERRIDES_H_)
#define _MPI_T_SAMPLING_OVERRIDES_H_

/*
  Override / Hook for MPI_Init() function
*/
#ifdef __cplusplus
extern "C" {
#endif
int __MPI_Init(int *argc, char ***argv);
#ifdef __cplusplus
}
#endif

/*
  Override / Hook for MPI_Finalize() function
*/
#ifdef __cplusplus
extern "C" {
#endif
int __MPI_Finalize(void);
#ifdef __cplusplus
}
#endif

#endif /* _MPI_T_SAMPLING_OVERRIDES_H_ */
