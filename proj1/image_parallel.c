#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TRUE 1
#define FALSE 0

typedef struct
{
    char M,N;	//매직넘버
    int width;
    int height;
    int max;
	unsigned char ** pixels;
}PPMImage;

struct Pixel{
	int x;
	int y;
	unsigned char RGB[3];
}

int size;
int rank;
MPI_Datatype t;

void fnClosePPM(PPMImage* img)
{
	int i=0;
	for(i=0; i<img->height; i++){
		free(img->pixels[i]);
	}

	free(img->pixels);
}
int fnReadPPM(char* fileNm, PPMImage* img)
{
	FILE* fp;
	int i,j;
	if(fileNm == NULL){
		fprintf(stderr, "fnReadPPM 호출 에러\n");
		return FALSE;
	}
	
	fp = fopen(fileNm, "rb");	// binary mode
	if(fp == NULL){
		fprintf(stderr, "파일을 열 수 없습니다 : %s\n", fileNm);
		return FALSE;
	}

	fscanf(fp, "%c%c\n", &img->M, &img->N);	// 매직넘버 읽기

	if(img->M != 'P' || img->N != '6'){
		fprintf(stderr, "PPM 이미지 포멧이 아닙니다 : %c%c\n", img->M, img->N);
		return FALSE;
	}

	fscanf(fp, "%d %d\n", &img->width, &img->height);	// 가로, 세로 읽기
	fscanf(fp, "%d\n"   , &img->max                );	// 최대명암도 값

	if(img->max != 255){
		fprintf(stderr, "올바른 이미지 포멧이 아닙니다.\n");
		return FALSE;
	}


	// <-- 메모리 할당
	img->pixels = (unsigned char**)calloc(img->height, sizeof(unsigned char*));

	for(i=0; i<img->height; i++){
	   // 1개의 픽셀을 위해 R, G, B 3byte가 필요
	   img->pixels[i] = (unsigned char*)calloc(img->width * 3, sizeof(unsigned char));
	}
	// -->


	// <-- ppm 파일로부터 픽셀값을 읽어서 할당한 메모리에 load
	for(i=0; i<img->height; i++){
		for(j=0; j<img->width * 3; j++){
			fread(&img->pixels[i][j], sizeof(unsigned char), 1, fp);
		}
	}
	// -->


	fclose(fp);	// 더 이상 사용하지 않는 파일을 닫아 줌

	return TRUE;
}
int fnWritePPM(char* fileNm, PPMImage* img)
{
	FILE* fp;
	int i,j;
	fp = fopen(fileNm, "w");
	if(fp == NULL){
		fprintf(stderr, "파일 생성에 실패하였습니다.\n");
		return FALSE;
	}

	fprintf(fp, "%c%c\n", 'P', '3');
	fprintf(fp, "%d %d\n" , img->width, img->height);
	fprintf(fp, "%d\n", 255);


	for(i=0; i<img->height; i++){
		for(j=0; j<img->width * 3; j+=3){

			fprintf(fp, "%d ", img->pixels[i][j]);
			fprintf(fp, "%d ", img->pixels[i][j+1]);
			fprintf(fp, "%d ", img->pixels[i][j+2]);

		}

		fprintf(fp, "\n");	// 생략가능
	}

	fclose(fp);
	
	return TRUE;
}
int fnWritePPM_AVG(char* fileNm, PPMImage* img)
{
    FILE* fp;
    int i,j,k,l,tmp,cnt=0;
	int height=img->height;
	int width=img->width;

    fp = fopen(fileNm, "w");
    if(fp == NULL){
        fprintf(stderr, "파일 생성에 실패하였습니다.\n");
        return FALSE;
    }

    fprintf(fp, "%c%c\n", 'P', '3');
    fprintf(fp, "%d %d\n" , img->width, img->height);
    fprintf(fp, "%d\n", 255);


    for(i=0; i<img->height; i++){
        for(j=0; j<img->width * 3; j+=3){
			tmp=0;
			cnt=0;
			for(k=-1;k<=1;k++){
				for(l=-1;l<1;l++){
					if(0<=(i+k)&&(i+k)<height){
						if(0<=(j+l)&&(j+l)<width*3){
							cnt++;
							tmp+=img->pixels[i+k][j+l];
						}
					}
				}
			}
			//img->pixels[i][j]=tmp/cnt;
			fprintf(fp,"%d ",tmp/cnt);
			tmp=0;
			cnt=0;
			for(k=-1;k<=1;k++){
                for(l=-1;l<1;l++){
                    if(0<=(i+k)&&(i+k)<height){
                        if(0<=(j+1+l)&&(j+1+l)<width*3){
                            cnt++;
                            tmp+=img->pixels[i+k][j+1+l];
                        }
                    }
                }
            }			
			//img->pixels[i][j+1]=tmp/cnt;
			fprintf(fp,"%d ",tmp/cnt);
			tmp=0;
            cnt=0;
            for(k=-1;k<=1;k++){
                for(l=-1;l<1;l++){
                    if(0<=(i+k)&&(i+k)<height){
                        if(0<=(j+2+l)&&(j+2+l)<width*3){
                            cnt++;
                            tmp+=img->pixels[i+k][j+2+l];
                        }
                    }
                }
            }
			fprintf(fp,"%d ",tmp/cnt);
        }
        fprintf(fp, "\n");  // 생략가능
    }

    fclose(fp);

    return TRUE;
}

int fnWritePPM_mirror(char* fileNm, PPMImage* img)
{
    FILE* fp;
    int i,j;
	int lala=(img->width*3)-1;
    fp = fopen(fileNm, "w");
    if(fp == NULL){
        fprintf(stderr, "파일 생성에 실패하였습니다.\n");
        return FALSE;
    }

    fprintf(fp, "%c%c\n", 'P', '3');
    fprintf(fp, "%d %d\n" , img->width, img->height);
    fprintf(fp, "%d\n", 255);


    for(i=0; i<img->height; i++){
        for(j=lala; j>=0 ; j-=3){
			
			fprintf(fp, "%d ", img->pixels[i][j-2]);
            fprintf(fp, "%d ", img->pixels[i][j-1]);
            fprintf(fp, "%d ", img->pixels[i][j]);
        }

        fprintf(fp, "\n");  // 생략가능
    }

    fclose(fp);

    return TRUE;
}

int fnWritePPM_grayScale(char* fileNm, PPMImage* img)
{
    FILE* fp;
    int i,j;
    fp = fopen(fileNm, "w");
    if(fp == NULL){
        fprintf(stderr, "파일 생성에 실패하였습니다.\n");
        return FALSE;
    }

    fprintf(fp, "%c%c\n", 'P', '2');
    fprintf(fp, "%d %d\n" , img->width, img->height);
    fprintf(fp, "%d\n", 255);


    for(i=0; i<img->height; i++){
        for(j=0; j<img->width * 3; j+=3){
			unsigned char R = img -> pixels[i][j];
			unsigned char G = img -> pixels[i][j+1];
			unsigned char B = img -> pixels[i][j+2];
			fprintf(fp, "%d ", (R+G+B)/3);
        }

        fprintf(fp, "\n");  // 생략가능
    }
    fclose(fp);

    return TRUE;
}

int main(int argc, char** argv)
{
	PPMImage img;
	int width,height;
	struct Pixel sample;
	int blklen[3];
	int i,j,tmp;
	unsigned char ** sen;
	unsigned char ** rec;
	int data;

	MPI_Aint displ[3],off,base;
	MPI_Datatype types[3];
	MPI_Datatype row;
	


	MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);

	blklen[0]=1;
	blklen[1]=1;
	blklen[2]=3;
	
	types[0]=MPI_INT;
	tpyes[1]=MPI_INT;
	types[2]=MPI_UNSIGNED_CHAR;

	displ[0]=0;
	
	MPI_Get_address(&(sample.x),&base);
	MPI_Get_address(&(sample.y),&off);
	disl[1]=off-base;
	MPI_Get_address(&(sample.RGB[0]),&off);
	displ[2]=off-base;
	MPI_Type_create_struct(3,blklen,displ,types,&t);
	MPI_Type_commit(&t);	
	MPI_Type_vector(1,width,1,t,&row);
	MPI_TYpe_commit(&row);
  
		

	if(argc != 2){
		fprintf(stderr, "사용법 : %s <filename>\n", argv[0]);
		return -1;
	}

	
	if(rank==0){
		if(fnReadPPM(argv[1], &img) != TRUE) return -1;
		width=img->width;
		height=img->height;
		sen=(unsigned char **)malloc(sizeof(unsigned char *)*width);
		rec(unsigned char **)malloc(sizeof(unsigned char *)* width);
		for(i=0;i<width;i++){
			(*rec)[i]=tmp;
			if(i<send_block % width)
				(*sen)[i]==height/size+1;
			else
				(*sen)[i]=height/size;
			tmp+=(*sen)[i];
		}
	}
	data = (rank<height%size) ? height/size+1 : height/size;
	
	MPI_Bcast(&width,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&height,1,MPI_INT,0,MPI_COMM_WORLD);
	
	// <-- 결과 확인은 파일로 대체
	if(fnWritePPM("result_sequential.ppm", &img) == TRUE){
		printf("파일 저장완료!\n");
	}
	// -->
	//grayScale	
	if(fnWritePPM_grayScale("result_sequential_grayScale.ppm", &img) == TRUE){
		printf("파일 저장완료!\n");
	}
	if(fnWritePPM_mirror("result_sequential_mirror.ppm", &img) == TRUE){
		printf("파일 저장완료!\n");
	}
	
	if(fnWritePPM_AVG("result_sequential_AVG.ppm", &img) == TRUE){
		printf("파일 저장완료!\n");
	}
	fnClosePPM(&img);

	return 0;
}
