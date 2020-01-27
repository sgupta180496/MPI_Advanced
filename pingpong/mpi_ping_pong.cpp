#include <mpi.h>
#include <iostream>

using namespace std;

int main (int argc, char* argv[]) 
{
  	if (argc < 2) 
	{
    		std::cerr<<"usage: mpirun "<<argv[0]<<" <value>"<<std::endl;
    		return -1;
  	}
  	int rank;
  	int* n=new int[1];
  	int* result=new int[1];
  	MPI_Init(&argc, &argv);
  	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  	if(rank==0)
	{
    		n[0]=atoi(argv[1]);
    		cout<<"Process 0 received user input: "<<"("<<n[0]<<")"<<". Process 0 sending it to process 1"<<endl;
    		MPI_Send(&n[0],1,MPI_INT,1,0,MPI_COMM_WORLD);
    		MPI_Recv(result,1,MPI_INT,1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    		cout<<"Process 0 received sum from process 1 "<<"("<<result[0]<<")"<<endl;;
  	}
  	if(rank == 1)
	{
    		MPI_Recv(n,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    		result[0]=n[0]+2;
    		MPI_Send(&result[0],1,MPI_INT,0,0,MPI_COMM_WORLD);
  	}
  	MPI_Finalize();
  	return 0;
}
