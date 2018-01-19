#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <mpi.h>

void knn(double **points,double **recvPoints, int **nearest,double **distance, int k,int numtasks,int offset);
double calcDist(double *a, double *b);
void test (int **nearest, int rank, int k,int numtasks);

int main(int argc,char **argv){
	if(argc!=2){
		printf("Usage: %s k where k is the number of nearest neighbours\n",
				argv[0]);
		exit(1);
	}
	
	
	
	MPI_Init(&argc,&argv);
	
	int k;
	k=atoi(argv[1]);//k nearest neighbours
	
	int rank, numtasks;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	
	MPI_Request reqs[2];
	MPI_Status stats[2];

	struct timeval startwtime, endwtime;
	double time;
	int i,j,a,b,l;
	
	//memory allocation for data
	double **points;
	int **nearest;
	double **distance;
	double **recvPoints;
	
	points=malloc(sizeof(double*)*60000/numtasks);
	nearest=malloc(sizeof(int*)*60000/numtasks);
	distance=malloc(sizeof(double*)*60000/numtasks);
	recvPoints=malloc(sizeof(double*)*60000/numtasks);
	
	for(i=0;i<60000/numtasks;i++){
		nearest[i]=malloc(sizeof(int)*(k+1));
	}
	for(i=0;i<60000/numtasks;i++){
		points[i]=malloc(sizeof(double)*30);
	}

	for(i=0;i<60000/numtasks;i++){
		distance[i]=malloc(sizeof(double)*(k+1));
	}
	for(i=0;i<60000/numtasks;i++){
		recvPoints[i]=malloc(sizeof(double)*30);
	}

	//load data;
	FILE *file;
	file=fopen("points.txt","r");
	int n,counter=0;
	double temp;
	
	for(i=0;i<60000;i++){
		for(j=0;j<30;j++){
			n=fscanf(file,"%lf ",&temp);
			if(i>=(60000/numtasks*rank) && i<(60000/numtasks*(rank+1))){
				points[counter][j]=temp;
				if(j==29){
					counter++;
				}
			}			
		}
	}
	
	fclose(file);

	MPI_Barrier(MPI_COMM_WORLD);
	if(rank==0)
		gettimeofday (&startwtime, NULL);
	int next=rank+1;
	int prev=rank-1;
	
	if (rank==(numtasks-1)) next=0;
	if (rank==0) prev=numtasks-1;
	
	for(a=0;a<60000/numtasks;a++){
			for(b=0;b<k+1;b++){
				distance[a][b]=1000;
			}
		}
		
	for(i=0;i<numtasks;i++){
		for(j=0;j<60000/numtasks;j++){
			for(l=0;l<30;l++){
				if(i==0){
					MPI_Isend(&points[j][l],1,MPI_DOUBLE,next,0,MPI_COMM_WORLD,&reqs[0]);
					MPI_Irecv(&recvPoints[j][l],1,MPI_DOUBLE,prev,0,MPI_COMM_WORLD,&reqs[1]);
					MPI_Waitall(2,reqs,stats);
				}
				else{
					MPI_Isend(&recvPoints[j][l],1,MPI_DOUBLE,next,0,MPI_COMM_WORLD,&reqs[0]);
					MPI_Irecv(&recvPoints[j][l],1,MPI_DOUBLE,prev,0,MPI_COMM_WORLD,&reqs[1]);
					MPI_Waitall(2,reqs,stats);
				}
			}
		}
		knn(points,recvPoints,nearest,distance,k,numtasks,(rank+numtasks-i-1)%numtasks);
	}
	
	MPI_Barrier(MPI_COMM_WORLD);
	
	if(rank==0){
		gettimeofday (&endwtime, NULL);
		time = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6
				  + endwtime.tv_sec - startwtime.tv_sec);
	
		printf("%f\n",time);
	
		
	}

	//~ printf("Testing in rank %d\n",rank);
	//~ test(nearest, rank, k, numtasks);
	MPI_Finalize();
}


double calcDist(double *a, double *b){
	double dist=0.0;
    int i;
	for(i=0;i<30;i++){
		dist=dist+(a[i]-b[i])*(a[i]-b[i]);
	}
	
	return sqrt(dist);
}

void knn(double **points,double **recvPoints, int **nearest, double **distance, int k,int numtasks,int offset){
	double dist=0;
	int i,j,l,m;
	
	for(i=0;i<60000/numtasks;i++){
		for(j=0;j<60000/numtasks;j++){	
			dist=calcDist(points[i],recvPoints[j]);
			
			for(l=0;l<k+1;l++){
				if (dist<distance[i][l]){
					for(m=k;m>l;m--){
						distance[i][m]=distance[i][m-1];
						nearest[i][m]=nearest[i][m-1];
					}
					distance[i][l]=dist;
					nearest[i][l]=j+offset*60000/numtasks;
					break;
				}
			}
		}
	}
}
void test (int **nearest, int rank, int k,int numtasks){
	int **check;
	int i,j;
	check=malloc(sizeof(int*)*60000/numtasks);
	for (i=0;i<60000/numtasks;i++){
		check[i]=malloc(sizeof(int*)*11);
	}
	
	FILE *file;
	file=fopen("nearest.txt","r");
	int n,temp;
	
	for (i=0;i<60000;i++){
		for (j=0;j<11;j++){
		    n = fscanf(file,"%d",&temp);
		    if (i>=(60000/numtasks*rank) && i<(60000/numtasks*(rank+1))) {
			check[i-(60000/numtasks)*rank][j] = temp;
		    }
		}
	}
	
	fclose(file);
    
	
	
	for(i=0;i<60000/numtasks;i++){
		for(j=0;j<11;j++){
			check[i][j]=check[i][j]-1;	//match matlab indexing with c indexing
		}
	}
	if(k<11)k=11;
	for (i=0;i<60000/numtasks;i++){
		for (j=0;j<k;j++){
		    if ( nearest[i][j] == check[i][j] ){
			continue;
		    }
		    else{
			printf("rank:%d \n",rank);
			printf("%d %d \n",i, j);
			printf("check: %d \n",check[i][j]);
			printf("nearest: %d \n",nearest[i][j]);
			printf("Test failed in process %d \n",rank);   
			return;         
		    }
		}
	    }
    
    printf("Test passed in process %d \n",rank);
    
}





