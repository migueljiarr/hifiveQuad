# README #

This project tries to create a flight controller software. It is designed to run on the SiFive's 
HiFive1 board.

### Build instructions: ###

First follow the instrutions at https://github.com/sifive/freedom-e-sdk to download and build the
freedom-e-sdk provided by SiFive and which we are going to use to build the project.

After you have built the SDK you just need to clone this repository into the software folder.

```
cd /path/to/freedom-e-sdk
cd software
git clone https://github.com/migueljiarr/hifiveQuad
```

Now, from the root from the SDK you can use the following commands:

```
make software PROGRAM=hifiveQuad	: builds the project.
make upload PROGRAM=hifiveQuad		: uploads program to hifive1 board.
make run_openocd 
make run_gdb PROGRAM=hifiveQuad		: opens openocd and gdb for debugging.
make software_clean PROGRAM=hifiveQuad	: cleans the build.
```
