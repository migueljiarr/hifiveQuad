This project tries to create a flight controller software. It is designed to run on the SiFive's 
HiFive1 board.

*Build instructions:*

First follow the instrutions at https://github.com/sifive/freedom-e-sdk to download and build the
freedom-e-sdk provided by SiFive and which we are going to use to build the project.

After you have built the SDK you just need to clone this repository and change inside the Makefile
the line with TOOL_DIR with the path to the location where you have installed the SDK.

You also need to create a symbolic link to the bsp from said SDK.

```
ln -l /path/to/SDK bsp
```

Once you have this, you can use the following make commands inside the project directory:

```
`make`: builds the project.
`make upload`: uploads program to hifive1 board.
`make run_openocd` and `make run_debug` in different terminals: opens openocd and gdb for debug.
`make clean`: cleans the build.
```
