Experiment with a different build model and see if it provides any benefits.

Jussi Pakkanen has [proposed an idea](https://nibblestew.blogspot.com/2024/12/compiler-daemon-thought-experiment.html)
where you start a compiler "server" that parses a precompiled header and stops.
Then for each input file the "server" will `fork` a new process that will
perform the actual compilation. The compilation should be faster because each
compilation process starts with the expensive-to-parse part already done and
doesn't need to repeat it again. The proposal outlines how different processes
can communicate with each other but we'll ignore that part for now for
simplicity.

Implementing the full proposal will take some time, so I'm suggesting a simple
experiment to evaluate this approach. Instead of building a full project we'll
compile a single file multiple times where each additional compilation starts
after a precompiled header is handled. We can compare this time to compiling
the same file in a loop and see how much state sharing can get us.

## Experimental data

### Small Objective-C file
Tested on Document.m from TextEdit sample. Precompiled header contains only
```
#import <Cocoa/Cocoa.h>
```

Run compilations for 20 times (`-Xclang -experimental-extra-fork-compilations -Xclang 19`). Each number is an average of 7 runs.

|                                                     | Compile in loop | Compile with `fork` |
|-----------------------------------------------------|-----------------|---------------------|
| No boosts                                           |           9.58s |               9.45s |
| With PCH                                            |           2.27s |               1.96s |
| With PCH (PCH creation is included in measurements) |           2.76s |               2.58s |
| With `-include` prefix header                       |           9.56s |               3.03s |

As we can see, there is around 10% speedup when we use `fork` approach. And
this is for the code that benefits from a precompiled header significantly.
The surprising finding for me is that using `-include` with the textual header
is slower than using a precompiled header.

### Bigger C++17 file
Tested on [LTO.cpp](https://github.com/vsapsai/llvm-project/blob/605c7b2c3be4debc4450313fe5e928c915bf3a98/llvm/lib/LTO/LTO.cpp)
with `-O2`. Precompiled header contains
```
#include <memory>
#include <random>
#include <chrono>
#include <algorithm>
#include <optional>
#include <list>
#include <queue>
#include <set>
#include <vector>
#include <thread>
#include <numeric>
```

Run compilations for 5 times (`-Xclang -experimental-extra-fork-compilations -Xclang 4`). Each number is an average of 7 runs.

|                                                     | Compile in loop | Compile with `fork` |
|-----------------------------------------------------|-----------------|---------------------|
| No boosts                                           |          34.12s |              36.09s |
| With PCH                                            |          33.31s |              34.92s |
| With PCH (PCH creation is included in measurements) |          34.10s |              36.53s |
| With `-include` prefix header                       |          34.79s |              35.14s |
| `-O0` with no boosts                                |          24.98s |              25.65s |
| `-O0` with PCH                                      |          23.58s |              24.53s |

This time `fork` approach doesn't give you anything. And a precompiled header
impact is way smaller (despite my efforts to tailor it to LTO.cpp). PCH
generation is pretty fast (~0.5s), so I believe in this case the complexity
comes not from raw parsing but from other activities.
