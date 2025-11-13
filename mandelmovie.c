#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int opt;
    int number = 0; // number of child processes
    int workload = 50; // 50 images is the workload
    double scaleMultiplier = 2; // scale multiplier


	//get opt testing to make sure it recognizes number
    while ((opt = getopt(argc, argv, "n:")) != -1) {
        switch (opt) {
            case 'n':
                number = atoi(optarg);
                printf("Chosen number: %d\n", number);
                break;
            default:
                printf("Please enter the number of children: -n (1,2,5,10,20)");
                return 1;
        }
    }

    printf("Number = %d\n", number);
    for (int i = 0; i < number; i++){
        printf("ONE\n");
    }

    for (int i = 0; i < number; i++){
        double chunkSize = workload /  number;  // number of items each child proecesses
        int remainder = workload % number;    // remainder if workload doesnt divide evenly
        pid_t pid = fork();

        if (pid==0){
            //child process
            // use i to set output file name for sequencing
			// multiply value by the scale multiplier
			// output with filename accordingly
			// NEED TO ADD INDEX TRACKING


        } else{
            //parent process(usually nothing here)
        }
    }
}