# Lab0 CProgrammingLab

One note for the CMU labs, we did not write the instruction PDFs. Not everything in the instructions will apply to what you need to do. If you are unsure about any of the instructions, ask in Teams.

Lab files are in `cprogramminglab-handout` and the directions are in
`cprogramminglab.pdf`.

Follow the instructions in `cprogramming.pdf`. You can run `make` directly, but you
cannot run `make test`, since `./driver.py` is used. 

- To run `make test`:
1. Install python2 in your environment;
2. Change `./driver.py` in `Makefile` to `python2 ./driver.py`;
3. Run `make test`;

- To use `driver.py` directly:
1. Install python2 in your environment;
2. Run `python2 driver.py`.

A copy of the linked list reference mentioned in the PDF is also provided here since the link in the PDF is broken.

The most tricky part in this lab is to pass trace case 11 and 12. You can try
commentting out the 2 lines in `queue.c` marked as "IMPORTANT", which would cause
memory leak. 

Lesson: when a function relies on the success of a series of memory allocation,
remember to free all previous allocated memory, in case any one of the memory
allocation step fails.
