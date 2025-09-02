# Watch Me

A networked live streaming application with motion detection.

## Features

- Non-blocking server to clients system
- Realtime, live GPU accelerated OpenCL motion detection
- OpenCV camera and window managing
- Renders the motion in red over the video capture

## Building

Ensure CMake is installed. Then, install OpenCL drivers and OpenCV. Modify build.sh to the appropriate Debug/Release:

```
./build.sh
```

### Running the Server

```
./build/watch_me
```

Gracefully exit with CTRL-C since SIGINT is handled.

### Running the Client

```
./build/client
```

Gracefully exit with ESC.

## Resources

- [Networking: Slightly Advanced Techniques](https://beej.us/guide/bgnet/html/split/slightly-advanced-techniques.html)
- [Grayscale algorithms](https://tannerhelland.com/2011/10/01/grayscale-image-algorithm-vb6.html)
