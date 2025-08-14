#include <stdio.h>
#include <syslog.h>

int main(int argc, char *argv[]) {

    char *writefile = argv[1];
    char *writestr = argv[2];

    openlog("writerstat", LOG_PID | LOG_CONS, LOG_USER);


    if (argc != 3) {
        syslog(LOG_ERR, "arguments not specified.");
        return 1;
    }


    FILE *f = fopen(writefile, "w");
    if (f == NULL) {
        syslog(LOG_ERR, "cannot open file %s", writefile);
        return 1;
    } else {
        syslog(LOG_DEBUG, "Writing %s to %s", writestr, writefile);
        fprintf(f, "%s", writestr);
    }

    closelog();

    fclose(f);
    return 0;

}
