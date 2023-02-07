#include "systemcalls.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{
    bool status = false;
    int ret = system(cmd);    

    if(ret == 0 && cmd == NULL)
        return false;

    if(ret == -1){
        status = false;
    }
    else{
        status = true;
    }

    return status;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

    va_end(args);
    __pid_t pid;
    int status;
    bool retval = false;

    //fflush(stdout);
    pid = fork();
    if(pid == -1){
        return false;
    }
    else if(pid == 0){
        int ret = execv(command[0], command);
        if(ret == -1){
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);            
    }

    if(pid > 0){
        if (waitpid (pid, &status, 0) == -1) //wait for child 
		{
			retval = false; 
		}
        else{
            if(WIFEXITED(status)){
                if(WEXITSTATUS(status) == EXIT_FAILURE){
                    retval = false;
                }
                else{
                    retval = true;
                }
            }
            else{
                retval = false;
            }
        }
    }

    return retval;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

    pid_t pid;
    int status;
    bool retval = false;
    va_end(args);
    int fd = open(outputfile, O_WRONLY|O_TRUNC|O_CREAT, 0644);
	if(fd < 0){
        return false;
    }
    

    //fflush(stdout);
    pid = fork();
    if(pid == -1){
        return false;
    }
    else if(pid == 0){

        int ret = dup2(fd, 1);
        if(ret == -1)
            exit(EXIT_FAILURE);
        ret = execv(command[0], command);
        if(ret == -1)
            exit(EXIT_FAILURE);

    }

    if(pid > 0){
        if (waitpid (pid, &status, 0) == -1) //wait for child 
		{
			retval = false; 
		}
        else{
            if(WIFEXITED(status)){
                if(WEXITSTATUS(status) == EXIT_FAILURE){
                    retval = false;
                }
                else{
                    retval = true;
                }
            }
            else{
                retval = false;
            }
        }
    }

    
    close(fd);
    return retval;
}
