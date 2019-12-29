#include <stdio.h>
#include <mpi.h>
#define MESSTAG 1
#define MAX_LEN 1

int arr[MAX_LEN]={0,};
int main(int argc,char * argv[]){
	int numprocs, rank, namelen;
	int num=10;
	MPI_Status stat;
	int i,rc=0;
	int sum=0;
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	if(rank==0){
		for(i=1;i<numprocs;i++){
			MPI_Send(&sum,MAX_LEN,MPI_INT,i,MESSTAG,MPI_COMM_WORLD);
			//printf("send -> %d",i);
			MPI_Recv(&sum,MAX_LEN,MPI_INT,i,MESSTAG,MPI_COMM_WORLD,&stat);	
		}
	}
	else{
		//MPI_Status stat;
		MPI_Recv(&sum,MAX_LEN,MPI_INT,0,MESSTAG,MPI_COMM_WORLD,&stat);
		//printf("My rank is %d \n",i);
		sum=sum+rank;
		MPI_Send(&sum,MAX_LEN,MPI_INT,0,MESSTAG,MPI_COMM_WORLD);	
	}
	printf("%d\n",sum);
	MPI_Finalize();
	//printf("%d\n",sum);
}
