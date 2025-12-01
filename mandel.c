/**********************************
* Author: Andrew Gilpatrick
* Assignment: Lab 12
* Date: 12/1/2025
* CPE2600-112
**********************************/
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "jpegrw.h"
#include <math.h>
#include <sys/wait.h>

static int iteration_to_color( int i, int max );
static int iterations_at_point( double x, double y, int max );
static void compute_image( imgRawImage *img, double xmin, double xmax,double ymin, double ymax, int maxIter, int numThreads );
static void show_help();

int opt;
int number = 1; // number of child processes
int workload = 50; // 50 images is the workload
double scale = 0.98; // Zoom multiplier
double zoom_factor = 0.98; // Default zoom
int numThreads = 1;   // default for threading

// Threading structure
typedef struct {
    imgRawImage *img;
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    int maxIter;
    int startRow;
    int endRow;
} ThreadJob;

void *thread_compute(void *arg) {
    ThreadJob *job = (ThreadJob *)arg;
    
    int width  = job->img->width;
    int height = job->img->height;
    
    for (int j = job->startRow; j < job->endRow; j++) {
        for (int i = 0; i < width; i++) {
            double x = job->xmin + i * (job->xmax - job->xmin) / width;
            double y = job->ymin + j * (job->ymax - job->ymin) / height;
            
            int iters = iterations_at_point(x, y, job->maxIter);
            unsigned int color = (0xFFFFFF * iters) / job->maxIter;
            
            setPixelCOLOR(job->img, i, j, color);
        }
    }
    
    return NULL;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/
void compute_image(imgRawImage *bm, double xmin, double xmax, double ymin, double ymax, int maxIter, int numThreads) {
    int height = bm->height;
    
    // Handle single thread case
    if (numThreads == 1) {
        int width = bm->width;
        for (int j = 0; j < height; j++) {
            for (int i = 0; i < width; i++) {
                double x = xmin + i * (xmax - xmin) / width;
                double y = ymin + j * (ymax - ymin) / height;
                int iters = iterations_at_point(x, y, maxIter);
                unsigned int color = (0xFFFFFF * iters) / maxIter;
                setPixelCOLOR(bm, i, j, color);
            }
        }
        return;
    }
    
    // multithreads
    pthread_t threads[numThreads];
    ThreadJob jobs[numThreads];
    int rowsPerThread = height / numThreads;
    int remainder = height % numThreads;
    for (int t = 0; t < numThreads; t++) {
        jobs[t].img = bm;
        jobs[t].xmin = xmin;
        jobs[t].xmax = xmax;
        jobs[t].ymin = ymin;
        jobs[t].ymax = ymax;
        jobs[t].maxIter = maxIter;
        jobs[t].startRow = t * rowsPerThread;
        jobs[t].endRow = (t + 1) * rowsPerThread;
        
        // Distribute remainder to the last thread
        if (t == numThreads - 1) {
            jobs[t].endRow = height;
        }
        
        int ret = pthread_create(&threads[t], NULL, thread_compute, &jobs[t]);
        if (ret != 0) {
            fprintf(stderr, "Error creating thread %d\n", t);
            exit(EXIT_FAILURE);
        }
    }
    
    // wait until finished
    for (int t = 0; t < numThreads; t++) {
        pthread_join(threads[t], NULL);
    }
}

int main( int argc, char *argv[] )
{
    char c;
    const char *outfile = "mandel2%d.jpg"; // changed to mandel 2 because whenever i delete the first ones from last lab they appear again
    char newName[50];
    double xcenter = 0;
    double ycenter = 0;
    double xscale = 4;
    double yscale = 0; // will be calculated for each image
    int    image_width = 1000;
    int    image_height = 1000;
    int    max = 1000;
    

    
    while((c = getopt(argc,argv,"x:y:s:W:H:m:o:n:t:z:h"))!=-1) {
        switch(c) 
        {
            case 'x':
                xcenter = atof(optarg);
                break;
            case 'y':
                ycenter = atof(optarg);
                break;
            case 's':
                xscale = atof(optarg);
                break;
            case 'W':
                image_width = atoi(optarg);
                break;
            case 'H':
                image_height = atoi(optarg);
                break;
            case 'm':
                max = atoi(optarg);
                break;
            case 'o':
                outfile = optarg;
                break;
            case 'n':
                number = atoi(optarg);
                if (number < 1) number = 1;
                break;
            case 't':
                numThreads = atoi(optarg);
                if (numThreads < 1) numThreads = 1;
                if (numThreads > 20) numThreads = 20;
                printf("Using %d thread(s) per process\n", numThreads);
                break;
            case 'z':  // zoom factor to test if its actually creating different images
                zoom_factor = atof(optarg);
                if (zoom_factor <= 0 || zoom_factor >= 1) {
                    fprintf(stderr, "Zoom factor must be between 0 and 1\n");
                    zoom_factor = 0.98;
                }
                break;
            case 'h':
                show_help();
                exit(1);
                break;
        }
    }
    
    int totalImages = 50;
    int chunkSize = totalImages / number;
    
    pid_t pid;
    int p;
    
    for (p = 0; p < number; p++) {
        pid = fork();
        
        if (pid == -1) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        
        if (pid == 0) {
            break;
        }
    }

    // child process
    if (pid == 0) {

        // Each child gets chunkSize images
        for (int imageNumber = 0; imageNumber < chunkSize; imageNumber++) {
            int globalIndex = p * chunkSize + imageNumber;
            sprintf(newName, outfile, globalIndex);
            
            // Apply zoom scale for each image
            double current_xscale = xscale * pow(zoom_factor, globalIndex);
            double current_yscale = (current_xscale * image_height) / image_width;
            
            imgRawImage *img = initRawImage(image_width, image_height);
            setImageCOLOR(img, 0);
            
            compute_image(img, xcenter - current_xscale / 2, xcenter + current_xscale / 2, 
				ycenter - current_yscale / 2, ycenter + current_yscale / 2,
                max, numThreads);
            
            storeJpegImageFile(img, newName);
            freeRawImage(img);
            
        }
        
        exit(0); // child done
    }

    // parent waits for children
    if (pid > 0) {
        for (int i = 0; i < number; i++) {
            wait(NULL);
		}
    }
    
    return 0;
}

/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/
int iterations_at_point( double x, double y, int max ) {
    double x0 = x;
    double y0 = y;
    
    int iter = 0;
    
    while( (x*x + y*y <= 4) && iter < max ) {
        double xt = x*x - y*y + x0;
        double yt = 2*x*y + y0;
        
        x = xt;
        y = yt;
        
        iter++;
    }
    
    return iter;
}

/*
Convert a iteration number to a color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/
int iteration_to_color( int iters, int max ) {
    int color = 0xFFFFFF * iters / (double)max;
    return color;
}

// Show help message
void show_help() {
    printf("Use: mandel [options]\n");
    printf("Where options are:\n");
    printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
    printf("-x <coord>  X coordinate of image center point. (default=0)\n");
    printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
    printf("-s <scale>  Initial scale of the image (X-axis). (default=4)\n");
    printf("-W <pixels> Width of the image in pixels. (default=1000)\n");
    printf("-H <pixels> Height of the image in pixels. (default=1000)\n");
    printf("-o <file>   Set output file pattern with %%d for frame number. (default=mandel%%d.jpg)\n");
    printf("-n <num>    Number of processes. (default=1)\n");
    printf("-t <num>    Number of threads per process (1-20). (default=1)\n");
    printf("-z <factor> Zoom factor per frame (0 < factor < 1). (default=0.98)\n");
    printf("-h          Show this help text.\n");
    printf("\nExamples:\n");
    printf("mandel -x -0.5 -y -0.5 -s 4 -n 2 -t 4 -z 0.95\n");
    printf("  Generate 50-frame zoom movie with 2 processes, 4 threads each, 0.95 zoom factor\n");
    printf("\nTo create a movie from generated frames:\n");
    printf("ffmpeg -framerate 30 -i mandel%%d.jpg -c:v libx264 -pix_fmt yuv420p mandel_movie.mp4\n\n");
}