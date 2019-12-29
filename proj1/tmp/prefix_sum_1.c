#include "mpi.h"
#include <stdio.h>

int main(int argc,char *argv[])
{
	int numtasks,rank;
	int local_data , sum=0;
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_rank(MPI_COMM_WORLD,&numtasks);
	
	local_data=rank;
	//printf("[Process %d]: has local data %d\n", rank, local_data);
    MPI_Scan(&local_data,&sum, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    printf("[Process %d]: has received data: %d \n", rank,sum);

    MPI_Finalize();
    
	//printf("%d\n",sum);
	return 0;
}
