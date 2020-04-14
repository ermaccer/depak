# depak
Extracts both encrypted and clear .pak archives created by PakMaker for Blitz3D.

You'll require 2 XOR keys, one for header mask and one for file contents.

# Usage
Execute without any arguments to print usage.


# Example

`depak  -e -k ABCDEF -m 123456 -o output_data Data.Pak`


# Keys

Used in K-Tech games such as Maluch Racer 2/2Fast Driver/Czeski Rajd and various clones.

```
Key: 46A78C95
HeaderMask: BC1558BC
```

