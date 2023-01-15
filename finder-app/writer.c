#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <libgen.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>

int main( int argc, char *argv[] )  {

    openlog(NULL, 0, LOG_USER);

    if( argc != 3 ) {
        syslog(LOG_ERR, "Invalid number of arguments: %d", argc);
        return 1;
    }

    int fd = open (argv[1], O_WRONLY | O_CREAT);
    if (fd == -1){
        syslog(LOG_ERR, "The file could not be opened");
        return 1;
    } 

    int ret = write (fd, argv[2], strlen (argv[2]));
    if (ret == -1){
        syslog(LOG_ERR, "The string could not be written to the file");
        return 1;
    }

    syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);
    close(fd);
    closelog();

    return 0;
}