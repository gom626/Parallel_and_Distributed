#include <stdio.h>
#include <stdlib.h>
#include <atring.h>
#include <mpi.h>

int main(int argc, char* argv[])
{
int jumpros, idpros;
    IplImage* image_input = cvLoadImage("test.jpg", CV_LOAD_IMAGE_UNCHANGED);
    IplImage* image_output = 
    cvCreateImage(cvGetSize(image_input),IPL_DEPTH_8U,channels);

    unsigned char *h_out = (unsigned char*)image_output->imageData;
    unsigned char *h_in =  (unsigned char*)image_input->imageData;

    width     = image_input->width;
    height    = image_input->height;
    channels  = image_input->nChannels;
    widthStep = image_input->widthStep;
    widthStepOutput = image_output->widthStep;

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&jumpros);
    MPI_Comm_rank(MPI_COMM_WORLD,&idpros);

   for (int n = idpros; n <=n-1; n+=jumpros){
       for(int i=0;i<height;i++){
         for(int j=0;j<width;j++){
            int index = h_in[i*widthStep + j*channels];
            int gray = 0.3*(index)+0.6*(index+1)+0.1*(index+2);
            h_out[i*widthStepOutput+j]=gray;
             }
           }
   }

    cvShowImage("Original", image_input);
    cvShowImage("CPU", image_output);
    cvReleaseImage(&image_input);
    cvReleaseImage(&image_output);
    waitKey(0);

    MPI_Finalize;
    return 0;
}
