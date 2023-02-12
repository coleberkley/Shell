Shell - Information

This shell is completely created from scratch to simulate functionality of the bash shell. Relative information for implementing this program was learned from my operating systems class. 

Shell Capabilities/Behavior:

Simple execution of commands - ls, ls -l, rm, cat, etc

Shell calls execvp() on commands that do not contain any shell-only strings

File Redirection - cat < some.txt, ls < some.txt, wc > some.txt, cat < in.txt > out.txt, cat < in.txt > out.txt 2> error.txt, etc

Shell interprets <, >, 2> as in, out, error redirection. Modifies fd table and replaces stin, stout, sterror with respective .txt files that are entered
Multiple file redirections can be called in one line.

Piping - ls | wc, ls -l | wc, cat < some.txt | other.txt

Shell interprets the pipe symbol and creates two child processes that share a pipe, rewriting stdin and stdout accordingly.

Signal Control - ignores ^C, ^Z, exits with ^D

Shell ignores ^C and ^Z signals, such signals will only affect child processes just like a real shell. ^D will exit the shell properly.

Job Control - jobs, bg, fg, any command followed by &

Shell has its own job implementation. Jobs are managed with process group IDs, and are stored in a linked list jobs struct. Shell maintains terminal control for background processes and gives terminal control to children that run in the foreground. A command followed by a & will be sent in the background, and jobs, bg, fg all work like they would in a bash shell. 




