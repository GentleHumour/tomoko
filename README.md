Tomoko Overview
===============
(A more detailed description will be provided later.)

Tomoko currently uses the Forth source code from Richard W.M. Jones' excellent public domain "JonesForth" implementation, and as such inherits a couple of its idiosyncrasies. Cheng Chang Wu has [modified JonesForth](https://github.com/chengchangwu/jonesforth) to be ANS FORTH compliant, and Tomoko will be updated to use the Forth sources from that project, shortly.

Quick Start
-----------

    cp src/jonesforth.f.txt ~/.tomoko
    cd build
    make
    cd ..
    ./tomoko
    
Tomoko's input routines don't currently treat backslash comments as special (they are defined as a word), so the long comment on line 22 of jonesforth.f.txt will be treated as an error, but things will work fine anyway.
