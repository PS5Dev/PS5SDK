# Payload SDK header generator

I'm using clang-15 from https://apt.llvm.org, earlier versions might also work.

```console
$ sudo apt-get install clang-15 python3-clang-15 python3-pyelftools
$ wget https://raw.githubusercontent.com/astrelsky/GhidraOrbis/master/data/nid_db.xml
$ python3 hgen.py --inc-dir ../include --prx /path_to_elfs/libkernel_web.sprx > libkernel_web.h
```
