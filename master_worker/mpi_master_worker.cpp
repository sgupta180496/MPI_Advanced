#include <mpi.h>
#include <iostream>
using namespace std;

#ifdef __cplusplus
extern "C" 
{
	#endif
	float f1(float x, int intensity);
	float f2(float x, int intensity);
	float f3(float x, int intensity);
	float f4(float x, int intensity);
	#ifdef __cplusplus
}
#endif
int main (int argc, char* argv[]) 
{
 	int interval = 0;
	MPI_Init(&argc, &argv);
  	if (argc < 6) 
	{
    		std::cerr<<"usage: mpirun "<<argv[0]<<" <functionid> <a> <b> <n> <intensity>"<<std::endl;
    		return -1;
  	}
  	struct timespec start_time, end_time;
  	clock_gettime(CLOCK_MONOTONIC_RAW, &start_time);
  	int function_id=atoi(argv[1]);
 	float a=atof(argv[2]);
  	float b=atof(argv[3]);
  	int n=atoi(argv[4]);
  	int intensity=atoi(argv[5]);
  	
	int rank;
	int size;
  	float Sum;
  	float result;

  	MPI_Status status;
  	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  	MPI_Comm_size(MPI_COMM_WORLD, &size);

  	int count=0;
  	int chunk=n/size;
  	int* arr=new int[2];
  	int TAG;
  	if(rank==0)
  	{          
     		for(int i=1;i<size;i++)
		{
                	arr[0]=interval;
			arr[1]=interval+chunk;
			TAG=1;
			count++; 
			if(interval>n)
			{
				TAG=0;
				count--;
			}         
        		else if((interval+chunk)>n)
				arr[1]=n;

        		MPI_Send(arr,2,MPI_INT,i,TAG,MPI_COMM_WORLD);
			interval+=chunk;	
		}
      		while(count!=0)
     		{
  			MPI_Recv(&result,1,MPI_FLOAT,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&status);

			if(status.MPI_TAG==0)
        			 count--;
			count--;	
 			Sum+=result;
       			arr[0]=interval;
			arr[1]=interval+chunk;
			TAG=1;
			count++;
 			if(interval>=n)
			{
	 			TAG=0;
				count--;
			}               
        		else if((interval+chunk)>n)
				arr[1]=n;

        		MPI_Send(arr,2,MPI_INT,status.MPI_SOURCE,TAG,MPI_COMM_WORLD);
			interval+=chunk;
		}
  	}
  	else
  	{ 
		int* Array=new int[2];
  		MPI_Recv(Array,2,MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
  		while(&status.MPI_TAG!=0)
  		{
			float res=0.0;
			if(Array[1]>n)
			{
				MPI_Send(&res,1,MPI_FLOAT,0,0,MPI_COMM_WORLD);
				break;
			}
			else if(Array[1]<=n)
			{
	   	   		for(int i=Array[0];i<Array[1];i++)
	   	   		{
	 				float x=a+(i+.5)*((b-a)/n);
     					switch(function_id)
					{
						case 1: res+=f1(x,intensity);
							break;
						case 2: res+=f2(x,intensity);
							break;
						case 3: res+=f3(x,intensity);
							break;
						case 4: res+=f4(x,intensity);
							break;
					}
            	   		}

 				MPI_Send(&res,1,MPI_FLOAT,0,1,MPI_COMM_WORLD);
	 			MPI_Recv(Array,2,MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
			}
    		}
	}
	if(rank == 0)
	{
  		clock_gettime(CLOCK_MONOTONIC_RAW,&end_time);
  		float elapsedTime=(end_time.tv_nsec-start_time.tv_nsec)/1000000000.0+(end_time.tv_sec-start_time.tv_sec);
  		Sum=Sum*((b-a)/n);  
  		cout<<Sum;
		cerr<<elapsedTime<<endl;
	}
 	MPI_Finalize();
	return 0;
}
