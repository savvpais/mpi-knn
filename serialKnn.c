#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>


void knn(double **points, int **nearest,double **distance, int k);
double calcDist(double *a, double *b);

int main(int argc,char **argv){
	if(argc!=2){
		printf("Usage: %s k where k is the number of nearest neighbours\n",
				argv[0]);
		exit(1);
	}
	
	int k;
	k=atoi(argv[1]);//k nearest neighbours
	
	
	struct timeval startwtime, endwtime;
    double time;
    int i,j;
	
	//memory allocation for data
	double **points;
	int **nearest;
	double **distance;
	
	points=malloc(sizeof(double*)*60000);
	nearest=malloc(sizeof(int*)*60000);
	distance=malloc(sizeof(double*)*60000);
	
	for(i=0;i<60000;i++){
		nearest[i]=malloc(sizeof(int)*k);
	}
	for(i=0;i<60000;i++){
		points[i]=malloc(sizeof(double)*30);
	}

	for(i=0;i<60000;i++){
		distance[i]=malloc(sizeof(double)*k);
	}
	

	//load data;
	FILE *file;
	file=fopen("points.txt","r");
	int n;
	for(i=0;i<60000;i++){
		for(j=0;j<30;j++){
			n=fscanf(file,"%lf ",&points[i][j]);
		}
	}
	
	fclose(file);
	
	//initialize distance matrix for knn search
	for(i=0;i<60000;i++){
		for(j=0;j<k;j++){
			distance[i][j]=1000;
		}
	}
	
	gettimeofday (&startwtime, NULL);
	knn(points,nearest,distance,k);
	gettimeofday (&endwtime, NULL);
	time = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6
		      + endwtime.tv_sec - startwtime.tv_sec);
	
	printf("Knn search wall clock time:%f\n",time);
	
}


double calcDist(double *a, double *b){
	double dist=0.0;
	int i;
	for(i=0;i<30;i++){
		dist=dist+(a[i]-b[i])*(a[i]-b[i]);
	}
	
	return sqrt(dist);
}

void knn(double **points, int **nearest, double **distance, int k){
	double dist=0;
	int i,j,l,m;
	
	for(i=0;i<60000;i++){
		for(j=0;j<60000;j++){
			if(i!=j){
				dist=calcDist(points[i],points[j]);
				
				for(l=0;l<k;l++){
					if (dist<distance[i][l]){
						for(m=k;m>l;m--){
							distance[i][m]=distance[i][m-1];
							nearest[i][m]=nearest[i][m-1];
						}
						distance[i][l]=dist;
						nearest[i][l]=j;
						break;
					}
				}
			}
			
		}
	}
}




