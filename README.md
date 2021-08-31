# CircularTest_Sheller
Endless cyclic test for Sheller with noise emulation

## How to use
1) Download zip or clone [this](https://github.com/VNovytskyi/CircularTest_Sheller) repo.
2) Download zip or clone [Sheller](https://github.com/VNovytskyi/Sheller) version what you want to test.
3) Edit path to Sheller files in qt pro-file.

## Note
Be careful, you are compiling a C ++ application with C code, use "extern C" for include file:
```c
extern "C" {
    #include "sheller.h"
}
```
