#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define TRUE 1
#define FALSE 0		
#define PPMREADBUFLEN 256

typedef struct {
	unsigned char R, G, B;
} RGB;

typedef struct {

	char M, N;
	int width, height;
	int max;
	RGB* pixels;

} PPMImage;

int rank, size, root = 0;
MPI_Datatype rgb, rowLine;

int fnReadPPM(char* fileNm, PPMImage* img)
{
	FILE* fp;
	int i,j;
	char buf[PPMREADBUFLEN], * t;

	if (fileNm == NULL) {
		fprintf(stderr, "fnReadPPM 호출 에러\n");
		return FALSE;
	}

	fp = fopen(fileNm, "rb");	// binary mode
	if (fp == NULL) {
		fprintf(stderr, "파일을 열 수 없습니다 : %s\n", fileNm);
		return FALSE;
	}

	fscanf(fp, "%c%c\n", &img->M, &img->N);	// 매직넘버 읽기

	if (img->M != 'P' || img->N != '6') {
		fprintf(stderr, "PPM 이미지 포멧이 아닙니다 : %c%c\n", img->M, img->N);
		return FALSE;
	}

	do
	{ /* Px formats can have # comments after first line */
		t = fgets(buf, PPMREADBUFLEN, fp);
		if (t == NULL) return FALSE;
	} while (strncmp(buf, "#", 1) == 0);

	sscanf(buf, "%d %d\n", &img->width, &img->height);	// 가로, 세로 읽기
	fscanf(fp, "%d\n", &img->max);	// 최대명암도 값

	if (img->max != 255) {
		fprintf(stderr, "올바른 이미지 포멧이 아닙니다. : (%d,%d), %d\n", img->width, img->height, img->max);
		return FALSE;
	}


	// <-- 메모리 할당
	img->pixels = calloc(img->height * img->width, sizeof(*img->pixels));


	// <-- ppm 파일로부터 픽셀값을 읽어서 할당한 메모리에 load
	for (i = 0; i < img->height; i++) {
		for (j = 0; j < img->width; j++) {
			fread(&img->pixels[i * img->width + j], sizeof(unsigned char), 3, fp);
		}
	}

	fclose(fp);	// 더 이상 사용하지 않는 파일을 닫아 줌

	return TRUE;
}

int fnWritePPM(char* fileNm, PPMImage* img)
{
	FILE* fp;
	int i,j;
	fp = fopen(fileNm, "wb");
	if (fp == NULL) {
		fprintf(stderr, "파일 생성에 실패하였습니다.\n");
		return FALSE;
	}

	fprintf(fp, "%c%c\n", 'P', '6');
	fprintf(fp, "%d %d\n", img->width, img->height);
	fprintf(fp, "%d\n", 255);


	for (i = 0; i < img->height; i++) {
		for (j = 0; j < img->width; j++) {
			fwrite(&img->pixels[i * img->width + j], sizeof(unsigned char), 3, fp);
		}
	}

	fclose(fp);

	return TRUE;
}

void fnClosePPM(PPMImage* img)
{
	free(img->pixels);
}

void derivedType()
{
	RGB sample;

	int blklen[3];
	MPI_Aint displ[3], off, base;
	MPI_Datatype types[3];

	blklen[0] = blklen[1] = blklen[2] = 1;
	types[0] = types[1] = types[2] = MPI_UNSIGNED_CHAR;

	displ[0] = 0;
	MPI_Get_address(&sample.R, &base);
	MPI_Get_address(&sample.G, &off);
	displ[1] = off - base;
	MPI_Get_address(&sample.B, &off);
	displ[2] = off - base;

	MPI_Type_create_struct(3, blklen, displ, types, &rgb);
	MPI_Type_commit(&rgb);
}

void merge_data(RGB* merged, int* recvcounts, int* displs,
	RGB* results, int block_cnt)
{
	int err = MPI_Gatherv(results, block_cnt, rowLine,
		merged, recvcounts, displs,
		rowLine, root, MPI_COMM_WORLD);

	if (err != MPI_SUCCESS)
		printf("error occur!\n");
}

void scatter_img_block(RGB* sendbuf, int send_block_cnt,
	int** p_sendcounts, int** p_displs,
	RGB** p_recvbuf, int* p_recv_block_cnt, int block_size)
{
	int i;
	int idx = 0;
	if (rank == root)
	{
		*p_sendcounts = malloc(sizeof(int) * size);
		*p_displs = malloc(sizeof(int) * size);


		for (i = 0; i < size; ++i)
		{
			(*p_displs)[i] = idx;

			if (i < send_block_cnt % size)
				(*p_sendcounts)[i] = send_block_cnt / size + 1;
			else
				(*p_sendcounts)[i] = send_block_cnt / size;

			idx += (*p_sendcounts)[i];
		}
		
		for(i=0;i<size;i++){
			printf("%d %d\n",(*p_sendcounts)[i],(*p_displs)[i]);
		}
	}
	if (rank < send_block_cnt % size)
		*p_recv_block_cnt = send_block_cnt / size + 1;
	else
		*p_recv_block_cnt = send_block_cnt / size;

	*p_recvbuf = malloc(sizeof(RGB) * block_size * (*p_recv_block_cnt));
	MPI_Scatterv(sendbuf,
		*p_sendcounts,
		*p_displs,
		rowLine,
		*p_recvbuf,
		*p_recv_block_cnt,
		rowLine,
		root, MPI_COMM_WORLD);
}

void flip_and_grey(RGB* input, int blk_cnt, int blk_size)
{
	RGB* blk_ptr = input;
	int i,j;
	int avg;

	for (i = 0; i < blk_cnt; ++i, blk_ptr += blk_size)
	{
		for (j = 0; j < blk_size / 2; ++j)
		{
			RGB tmp = blk_ptr[j];
			blk_ptr[j] = blk_ptr[blk_size - j - 1];
			blk_ptr[blk_size - j - 1] = tmp;
		}


		for (j = 0; j < blk_size; ++j)
		{
			avg = ((int)blk_ptr[j].R + blk_ptr[j].G + blk_ptr[j].B) / 3;
			blk_ptr[j].R = avg;
			blk_ptr[j].B = avg;
			blk_ptr[j].G = avg;
		}

	}

}

int main(int argc, char* argv[])
{
	PPMImage img;
	int width, height;
	RGB* data = NULL;
	int data_block_cnt;
	int i;
	int* counts = NULL, * displs = NULL;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (argc != 2) {
		fprintf(stderr, "사용법 : %s <filename>\n , argv[0]");
	}

	if (rank == root) {
		fnReadPPM(argv[1], &img);
		width = img.width;
		height = img.height;
	}

	MPI_Bcast(&width, 1, MPI_INT, root, MPI_COMM_WORLD);
	MPI_Bcast(&height, 1, MPI_INT, root, MPI_COMM_WORLD);

	derivedType(&rgb);
	MPI_Type_vector(1, width, 1, rgb, &rowLine);
	MPI_Type_commit(&rowLine);
	
	scatter_img_block(img.pixels, height,
		&counts, &displs,
		&data, &data_block_cnt, width);
/*	for(i=0;i<size;i++){
		printf("%d %d\n",counts[i],displs[i]);
	}*/
	printf("[%d] received block counts = %d\n", rank, data_block_cnt);


	flip_and_grey(data, data_block_cnt, width);
	merge_data(img.pixels, counts, displs, data, data_block_cnt);

	if (rank == root)
	{

		fnWritePPM("parallel.ppm", &img);
		fnClosePPM(&img);
	}

	MPI_Finalize();

	return 0;
}
