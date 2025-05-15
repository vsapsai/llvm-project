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
