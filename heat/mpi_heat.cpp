#include <mpi.h>
#include <math.h>
#include <iostream>

using namespace std;

#ifdef __cplusplus
extern "C" 
{
 	#endif
	double generate2DHeat(long n, long global_i, long global_j);
  	int check2DHeat(double** v, long n, long global_i, long global_j, long k); 
	#ifdef __cplusplus
}
#endif

int main(int argc, char* argv[]) 
{
  	if(argc < 3) 
	{
    		std::cerr<<"usage: mpirun "<<argv[0]<<" <N> <K>"<<std::endl;
    		return -1;
  	}
  
  	MPI_Init(&argc,&argv);
  	long N, K;
  	N=atol(argv[1]);
  	K=atol(argv[2]);
  	int rank,size;
   
   	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
   	MPI_Comm_size(MPI_COMM_WORLD,&size);

   	int p=sqrt(size);
   	long granule=N/p; 
   
   	int p_row=rank/p;
	int p_column=rank%p; 

   	double**  M=new double*[granule];
   	double**  G=new double*[granule];
   	for(long i=0;i<granule;i++)
     	{
     		M[i]=new double[granule];
     		G[i]=new double[granule];
     
   	}
   
   	long start_row=p_row*granule;
	long start_col=p_column*granule;
   	long end_row=start_row+granule;
	long end_col=start_col+granule;

  	for(long row=start_row,x=0;row<end_row;row++,x++) 
	{
    		for(long col=start_col,y=0;col<end_col;col++,y++) 
       			M[x][y]=generate2DHeat(N,row,col);
  	} 
  
  	double *t=new double[granule];
  	double *b=new double[granule];
  	double *r=new double[granule];
  	double *l=new double[granule];
  	double *t1=new double[granule];
  	double *b1=new double[granule];
  	double *r1=new double[granule];
  	double *l1=new double[granule];
  
  	int begin,end,left,right;
  	begin=rank-p;
  	end=rank+p;

	if(p_column==(p-1))
		right=-1;
	else
		right=rank+1;

	if(p_column!=0)
	  	left=rank-1;
	else
		left=-1;

  	MPI_Status status[4];
  	MPI_Request requests[8];
  	long count=0;
  	double start_time=MPI_Wtime();
  	for (long a=0;a<K;a++) 
  	{
     		for(long d=0,c=0;d<granule;d++)
       		{
	 		l[c]=M[d][0];
			l1[c]=M[d][0];
			
	 		r[c]=M[d][granule-1];
			r1[c]=M[d][granule-1];

	 		t[c]=M[0][d];
			t1[c]=M[0][d];

	 		b[c]=M[granule-1][d];
	 		b1[c]=M[granule-1][d];

	 		c++;
       		}
     		count=0; 
     		if(begin>=0)
		{
       			MPI_Isend(t1,granule,MPI_DOUBLE,begin,0,MPI_COMM_WORLD,&requests[count]);
       			count++;
     		}
     		if(right!=-1)
		{
       			MPI_Isend(r1,granule,MPI_DOUBLE,right,1,MPI_COMM_WORLD,&requests[count]);
       			count++;
     		}
     		if(end<size)
		{
       			MPI_Isend(b1,granule,MPI_DOUBLE,end,2,MPI_COMM_WORLD,&requests[count]);
       			count++;
     		}
     		if(left!=-1)
		{
       			MPI_Isend(l1,granule,MPI_DOUBLE,left,3,MPI_COMM_WORLD,&requests[count]);
       			count++;
     		}
     		for(long i=1;(i+1)<granule;i++)
       		{
	 		for(long j=1;(j+1)<granule;j++)
	     			G[i][j]=(M[i][j]+M[i-1][j]+M[i+1][j]+M[i][j-1]+M[i][j+1])/5;
       		}

     		if(begin>=0)
       			MPI_Recv(t,granule,MPI_DOUBLE,begin,2,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

     		if(right!=-1)
       			MPI_Recv(r,granule,MPI_DOUBLE,right,3,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

     		if(end<size)
       			MPI_Recv(b,granule,MPI_DOUBLE,end,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

     		if(left!=-1)
       			MPI_Recv(l,granule,MPI_DOUBLE,left,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

     		G[0][0]=(M[0][0]+t[0]+l[0]+M[0][1]+M[1][0])/5;

     		G[0][granule-1]=(M[0][granule-1]+t[granule-1]+r[0]+M[0][granule-2]+M[1][granule-1])/5;

     		G[granule-1][0]=(M[granule-1][0]+b[0]+l[granule-1]+M[granule-1][1]+M[granule-2][0])/5;

     		G[granule-1][granule-1]=(M[granule-1][granule-1]+b[granule-1]+r[granule-1]+M[granule-1][granule-2]+M[granule-2][granule-1])/5;



     		for(long i=1,j=1;i<granule-1;i++,j++)
	 	{
	    		G[0][i]=(M[0][i]+M[1][i]+t[i]+M[0][i-1]+M[0][i+1])/5;

	    		G[granule-1][i]=(M[granule-1][i]+M[granule-2][i]+b[i]+M[granule-1][i-1]+M[granule-1][i+1])/5;

	    		G[j][0]=(M[j][0]+l[j]+M[j][1]+M[j-1][0]+M[j+1][0])/5;

	    		G[j][granule-1]=(M[j][granule-1]+M[j-1][granule-1]+M[j+1][granule]+r[j]+M[j][granule-2])/5;
	 	}



      		M=G;
      		check2DHeat(M,N,rank,size,a);
      		MPI_Waitall(count,requests,status);
   	}
  	if(rank==0)
    	{
       		double end_time=MPI_Wtime();
       		cerr<<end_time-start_time<<endl; 
    	}
 	for(long i=0;i<granule;i++)
   		delete[] M[i];

  	delete[] M;
  	delete[] t;
  	delete[] b;
  	delete[] r;
  	delete[] l;
  	MPI_Finalize();
	return 0;
}




