#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <signal.h>


char** parseString(char *inString, char* token_array[64]){

    char *parsedcmd = strtok(inString, " ");
    int i = 0;
    while(parsedcmd != NULL){

        token_array[i] = parsedcmd;
        parsedcmd = strtok(NULL, " ");
        i++;

    }
    token_array[i] = NULL;
    return token_array;
}

//jobs struct

struct JobsList {
    int job_id;
    char* status;
    int pgid;
    char* title;
    char* d1;
    char* title2;
    char* d2;
    struct JobsList* next;
};

struct JobsList* head = NULL;

// void test_handler(int s){
//     printf("test\n");
// }
void SigChildHandler(int s){
    //printf("called\n");

    struct JobsList* ptr = head;
    int j_status;
    //loop through not-null nodes that aren't marked for removal
    while(ptr != NULL){
        if(strcmp(ptr->status, "Done") == 0){
            ptr = ptr->next;
            continue;
        }

        int this_pgid = ptr->pgid;
        int res = waitpid(this_pgid, &j_status, WNOHANG);
        if(res != 0){
            //printf("exited properly\n");
            ptr->status = "Done";
        }
        ptr = ptr->next;


    }

}


int main(){

    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGCHLD, &SigChildHandler);

    //Setting parent's pgid = pid
    setpgid(0, 0);
    int parent_pgid = getpgid(0);

    while(1){

        int cpid;
        char *cmd;
        char *token_array[64] = {NULL};
        int ret1;
        int ret2;

        //check for done jobs

        //if head is done, set head ptr to head->next and free head
        struct JobsList* pptr = head;

        struct JobsList* temp;
        while(pptr != NULL && strcmp(pptr->status, "Done") == 0){
                head = head->next;
                //printf("Freed (head) %d %s\n", pptr->job_id, pptr->title);
                char* jsign;
                if(strcmp(pptr->status, "Stopped") == 0) jsign = "-";
                else jsign = "+";
                if(pptr->title2 != NULL){
                    if(pptr->d2 != NULL && pptr->d1 != NULL) 
                    printf("[%d]%s      %s      %s %s | %s %s\n", pptr->job_id, jsign, pptr->status, pptr->title, pptr->d1, pptr->title2, pptr->d2);
                    else if(pptr-> d1 != NULL)
                    printf("[%d]%s      %s      %s %s | %s\n", pptr->job_id, jsign, pptr->status, pptr->title, pptr->d1, pptr->title2);
                    else if(pptr-> d2 != NULL)
                    printf("[%d]%s      %s      %s | %s %s\n", pptr->job_id, jsign, pptr->status, pptr->title, pptr->title2, pptr->d2);
                    else
                    printf("[%d]%s      %s      %s | %s\n", pptr->job_id, jsign, pptr->status, pptr->title, pptr->title2);
                }
                else{
                    if(pptr->d1 != NULL)
                    printf("[%d]%s      %s      %s %s\n", pptr->job_id, jsign, pptr->status, pptr->title, pptr->d1);
                    else
                    printf("[%d]%s      %s      %s\n", pptr->job_id, jsign, pptr->status, pptr->title);
                }
                free(pptr);
                pptr = head;
        }
        
        //if head isnt null, iterate through other processes
        if(head != NULL && head->next != NULL){
            struct JobsList* dptr = head->next;
            struct JobsList* prev = head;
            while(dptr != NULL){

                if(strcmp(dptr->status, "Done") == 0){
                    prev->next = dptr->next;
                    //printf("Freed (iter) %d %s\n", dptr->job_id, dptr->title);
                    char* jsign;
                    if(strcmp(dptr->status, "Stopped") == 0) jsign = "-";
                    else jsign = "+";
                    if(dptr->title2 != NULL){
                        if(dptr->d2 != NULL && dptr->d1 != NULL) 
                        printf("[%d]%s      %s      %s %s | %s %s\n", dptr->job_id, jsign, dptr->status, dptr->title, dptr->d1, dptr->title2, dptr->d2);
                        else if(dptr-> d1 != NULL)
                        printf("[%d]%s      %s      %s %s | %s\n", dptr->job_id, jsign, dptr->status, dptr->title, dptr->d1, dptr->title2);
                        else if(dptr-> d2 != NULL)
                        printf("[%d]%s      %s      %s | %s %s\n", dptr->job_id, jsign, dptr->status, dptr->title, dptr->title2, dptr->d2);
                        else
                        printf("[%d]%s      %s      %s | %s\n", dptr->job_id, jsign, dptr->status, dptr->title, dptr->title2);
                    }
                    else{
                        if(dptr->d1 != NULL)
                        printf("[%d]%s      %s      %s %s\n", dptr->job_id, jsign, dptr->status, dptr->title, dptr->d1);
                        else
                        printf("[%d]%s      %s      %s\n", dptr->job_id, jsign, dptr->status, dptr->title);
                    }
                    free(dptr);
                    dptr = dptr->next;
                }else{
                    //printf("iter next\n");
                    prev = dptr;
                    dptr = dptr->next;
                    
                }
            }
        }

        cmd = readline("# ");

        //^D (EOF) is the same as a NULL value. Quit if NULL
        if(cmd == NULL){
            break;
        }
        *token_array = *parseString(cmd, token_array);

        if(token_array[0] == NULL){
            continue;
        }

        //check for job commands
        if(strcmp(token_array[0], "bg") == 0){
            //send most recent stopped job to background
            struct JobsList* ptr = head;
            int fifo[32];
            int pfifo = 0;
            while(ptr != NULL){
                if(strcmp(ptr->status, "Stopped") == 0){
                    //if this process is eligible for fg queue, save pgid
                    fifo[pfifo] = ptr->pgid;
                    pfifo++;
                }
                ptr = ptr->next;
            }

            //ptr->status = "running";
            ptr = head;
            int this_pgid = fifo[pfifo-1];
            while(ptr->pgid != this_pgid && ptr != NULL) ptr = ptr->next;
            ptr->status = "Running";
            //get the last stopped process to be continued
            int st;
            kill(this_pgid, 19);
            continue;


        }
        if(strcmp(token_array[0], "fg") == 0){
            //send most recent job put in background to foreground
            //give job terminal control
            struct JobsList* ptr = head;
            int filo[32];
            int pfilo = 0;
            while(ptr != NULL){
                if(strcmp(ptr->status, "Stopped") == 0){
                    filo[pfilo] = ptr->pgid;
                    pfilo++;
                }
                ptr = ptr->next;
            }
            ptr = head;
            int this_pgid = filo[pfilo-1];
            while(ptr->pgid != this_pgid && ptr != NULL) ptr = ptr->next;
            ptr->status = "Running";
            int st;
            tcsetpgrp(0, this_pgid);
            kill(this_pgid, 19);
            waitpid(this_pgid, &st, WUNTRACED);
            tcsetpgrp(0, parent_pgid);
            if(WIFSTOPPED(st) == 1){
                //printf("Stopped! \n");
                ptr->status = "Stopped";
            }
            else{
                ptr->status = "Done";
            }
            continue;

        }
        if(strcmp(token_array[0], "jobs") == 0){
            //list all jobs
            struct JobsList* ptr = head;
            while(ptr != NULL){
                char* jsign;
                if(strcmp(ptr->status, "Stopped") == 0) jsign = "-";
                else jsign = "+";
                if(ptr->title2 != NULL){
                    if(ptr->d2 != NULL && ptr->d1 != NULL) 
                    printf("[%d]%s      %s      %s %s | %s %s\n", ptr->job_id, jsign, ptr->status, ptr->title, ptr->d1, ptr->title2, ptr->d2);
                    else if(ptr-> d1 != NULL)
                    printf("[%d]%s      %s      %s %s | %s\n", ptr->job_id, jsign, ptr->status, ptr->title, ptr->d1, ptr->title2);
                    else if(ptr-> d2 != NULL)
                    printf("[%d]%s      %s      %s | %s %s\n", ptr->job_id, jsign, ptr->status, ptr->title, ptr->title2, ptr->d2);
                    else
                    printf("[%d]%s      %s      %s | %s\n", ptr->job_id, jsign, ptr->status, ptr->title, ptr->title2);
                }
                else{
                    if(ptr->d1 != NULL)
                    printf("[%d]%s      %s      %s %s\n", ptr->job_id, jsign, ptr->status, ptr->title, ptr->d1);
                    else
                    printf("[%d]%s      %s      %s\n", ptr->job_id, jsign, ptr->status, ptr->title);
                }
                ptr = ptr->next;
            }
            continue;

            
        }

        //check for presence of | and &
        // | --> split processes into two token arrays
        //nullify end & if present, keep note it was present
        int pipe_fd[2];
        int p = 0;
        int picheck = -1;
        int bgcheck = -1;
        int right;
        char *token_array2[64] = {NULL};

        //first check for &, null terminate it to remove 
        int b = 0;
        while(token_array[b] != NULL){
            if(strcmp(token_array[b], "&") == 0){
                bgcheck = 1;
                token_array[b] = NULL;
                break;
            }
            b++;
        }

        while(token_array[p] != NULL){
            
            //if pipe
            if(strcmp(token_array[p], "|") == 0){

                token_array[p] = NULL;
                right = p + 1;
                picheck = 1;

                //copy right side into its own command array
                int k = 0;
                while(token_array[right] != NULL){
                    token_array2[k] = token_array[right];
                    k++;
                    right++;
                }
                //null terminate the command array when finished
                token_array2[k] = NULL;
                //p++;
                break;
            }

            p++;

        }

        if(picheck == 1){
            pipe(pipe_fd);
        }

        cpid = fork();

        if(cpid == 0){
            //left child

            //Set left child's signal control
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            int i = 0;
            while(token_array[i] != NULL){

                if(strcmp(token_array[i], "<") == 0){
                    //redirect standard input

                    if(i==0){
                        printf("Cannot start with < \n");
                        break;
                    }
                    int fd = open(token_array[i+1], S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                    token_array[i] = NULL;

                    i++;
                    continue;
                }

                if(strcmp(token_array[i], ">") == 0){
                    //redirect standard output

                    if(i==0){
                        printf("Cannot start with > \n");
                        break;
                    }

                    int fd = open(token_array[i+1], O_WRONLY | O_CREAT, 0777);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    token_array[i] = NULL;
                    
                    i++;
                    continue;
                    
                }

                if(strcmp(token_array[i], "2>") == 0){

                    char* err_file;
                    int fd = open(token_array[i+1], O_WRONLY | O_CREAT, 0777);
                    dup2(fd, STDERR_FILENO);
                    close(fd);
                    token_array[i] = NULL;

                    i++;
                    continue;
                    
                }

                i++;

            }

            //child is left side of a pipe (writing side), shouldn't have rewrote stdout to anything else
            if(picheck == 1){
                dup2(pipe_fd[1], STDOUT_FILENO);
                close(pipe_fd[1]);
                close(pipe_fd[0]);
            }

            ret1 = execvp(token_array[0], token_array);
            if (ret1 == -1){
                //printf("Nah\n");
                exit(30);
            }
        }else{
            setpgid(cpid, 0);
        }

        int cpid2;
        if(picheck == 1){
            
            cpid2 = fork();


            if(cpid2 == 0){
                //right child code
                
                //set right child's signal control
                signal(SIGINT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
                setpgid(0, cpid);

                int i = 0;
                while(token_array2[i] != NULL){

                    if(strcmp(token_array2[i], "<") == 0){
                        //redirect standard input

                        if(i==0){
                            printf("Cannot start with < \n");
                            break;
                        }

                        int fd = open(token_array2[i+1], S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
                        dup2(fd, STDIN_FILENO);
                        close(fd);
                        token_array2[i] = NULL;

                        i++;
                        continue;
                    }

                    if(strcmp(token_array2[i], ">") == 0){
                        //redirect standard output

                        if(i==0){
                            printf("Cannot start with > \n");
                            break;
                        }

                        int fd = open(token_array2[i+1], O_WRONLY | O_CREAT, 0777);
                        dup2(fd, STDOUT_FILENO);
                        close(fd);
                        token_array2[i] = NULL;
                        
                        i++;
                        continue;

                        
                    }

                    if(strcmp(token_array2[i], "2>") == 0){

                        int fd = open(token_array2[i+1], S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
                        dup2(fd, STDERR_FILENO);
                        close(fd);
                        token_array2[i] = NULL;

                        i++;
                        continue;
                        
                    }

                    i++;

                }
                dup2(pipe_fd[0], STDIN_FILENO);
                close(pipe_fd[0]);
                close(pipe_fd[1]);

                ret2 = execvp(token_array2[0], token_array2);
                if (ret2 == -1){
                    exit(30);
                }
            }
            else{
                setpgid(cpid2, cpid);
            }
            
        }
        if(picheck == 1){
            close(pipe_fd[0]);
            close(pipe_fd[1]);
        }

        int status;
        int job_stopped = 0;
        int job_background = 0;
        if(bgcheck != 1){
            tcsetpgrp(0, cpid);
            waitpid(-cpid, &status, WUNTRACED);
            tcsetpgrp(0, parent_pgid);
            if (WEXITSTATUS(status) != 30){
                //valid command
                //if command was stopped and not terminated
                if(WIFSTOPPED(status)) job_stopped = 1;
            }

        }
        else job_background = 1;
        
        //if group process is valid and stopped or in background, make a job for it
        if(job_background == 1 || job_stopped == 1){
            //create job
            struct JobsList* newNode = (struct JobsList*)malloc(sizeof(struct JobsList));
            if(picheck == 1){
                if(token_array2[1] != NULL) newNode->d2 = token_array2[1];
                else newNode->d2 = NULL;
                newNode->title2 = token_array2[0];
            }
            else newNode->title2 = NULL;
            if(token_array[1] != NULL) newNode->d1 = token_array[1];
            else newNode->d1 = NULL;
            newNode -> title = token_array[0];
            if(job_background == 1) newNode -> status = "Running";
            else newNode -> status = "Stopped";
            newNode -> pgid = cpid;
            int job_id = 1;
            struct JobsList* temp;
            if(head == NULL){
                newNode->job_id = job_id;
                head = newNode;
            }
            else{
                temp = head;
                job_id++;
                while(temp->next != NULL){
                    temp = temp->next;
                    job_id++;
                }
                newNode->job_id = job_id;
                temp->next = newNode;
            }
        }

    }
    printf("\n");
    struct JobsList* fptr = head;
    struct JobsList* prev = fptr;
    while(fptr != NULL){
        prev = fptr;
        fptr = fptr->next;
        free(prev);
    }

}
