# System Programming Lab 12


# Overview
This program generates a series of images using multiple processes and threads. This speeds up the rendering of each frame. Each child process renders its assigned frames in parallel using threads.



# Data
|**Processes/Threads**| **1 Thread** | **2 Threads** | **4 Threads** | **8 Threads**|
|                     |              |               |               |              |
|**1 Process**        |  78.19s      |  42.98s       |  39.99s       |  28.02s      |
|**2 Processes**      |  54.58s      |  34.05s       |  31.44s       |  23.01s      |
|**4 Processes**      |  32.34s      |  22.80s       |  20.48s       |  15.05s      |
|**8 Processes**      |  24.25s      |  16.54s       |  15.09s       |  15.01s      |

## Result Discussion
- Going from one thread to multiple had the biggest impact on speed but more processes per thread will always make it faster. I think this is due to the fact that we need 50 images regardless, so more "instances" of image generation will be more efficient then using multiple threads makes calculating a single image faster.
- The "sweet spot" was 4 threads with 8 processes, and 8 threads with 4 or 8 processes