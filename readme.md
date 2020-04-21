## c-parallel

Towards a lightweight, dependency-free alternative of GNU Parallel.
It can work now, but it can just work now.

### Performance

less than 100KB memory allocated.

```
time seq 1 100 | ./parallel -j 4 "echo {}" > /dev/null

real    0m0.466s
user    0m0.016s
sys     0m1.219s

time seq 1 100 | parallel -j 4 "echo {}" > /dev/null
Academic tradition requires you to cite works you base your article on.
When using programs that use GNU Parallel to process data for publication
please cite:

  O. Tange (2011): GNU Parallel - The Command-Line Power Tool,
    ;login: The USENIX Magazine, February 2011:42-47.

	This helps funding further development; AND IT WON'T COST YOU A CENT.
	If you pay 10000 EUR you should feel free to use GNU Parallel without citing.

	To silence this citation notice: run 'parallel --citation'.


real    0m3.409s
user    0m0.281s
sys     0m4.484s
```
