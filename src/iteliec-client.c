#include "iteliec-client.h"

#include "system/host.h"
#include "system/load.h"
#include "system/user.h"
#include "system/swap.h"
#include "system/memory.h"
#include "system/cpu.h"
#include "system/network.h"
#include "system/process.h"
#include "system/disk.h"

static int down = 0;
static int update_period = 60;
static char *pid_file = "/var/run/iteliecd.pid";
static const char *cfile = "/usr/local/etc/iteliecd.conf";

static void sig_handler (int sig, siginfo_t *siginfo ATTRIBUTE_UNUSED, void *context ATTRIBUTE_UNUSED) {
    switch (sig) {
        case SIGINT:
            down = 1;
        break;

        case SIGTERM:
        case SIGQUIT:
            down = 1;
        break;

        default:
        break;
    }
}

static int write_pid_file (const char *pfile) {
    int fd;
    FILE *fh;

    if (pfile[0] == '\0')
        return 0;

    if ((fd = open (pfile, O_WRONLY|O_CREAT|O_EXCL, 0644)) < 0) {
        iteliec_log (ITELIEC_ERR, "Failed to open pid file '%s' : %s", pfile, strerror (errno));

        return ITELIEC_FATAL;
    }

    if (!(fh = fdopen (fd, "w"))) {
        iteliec_log (ITELIEC_ERR, "Failed to fdopen pid file '%s' : %s", pfile, strerror (errno));
        close (fd);

        return ITELIEC_FATAL;
    }

    if (fprintf (fh, "%lu\n", (unsigned long)getpid ()) < 0) {
        iteliec_log (ITELIEC_ERR, "Failed to write to pid file '%s' : %s", pfile, strerror (errno));
        close (fd);

        return ITELIEC_FATAL;
    }

    if (fclose (fh) == EOF) {
        iteliec_log (ITELIEC_ERR, "Failed to close pid file '%s' : %s", pfile, strerror (errno));

        return ITELIEC_FATAL;
    }

    return ITELIEC_OK;
}

static int daemonize (void) {
    int pid = fork ();

    switch (pid) {
        case 0: {
            int stdinfd = -1;
            int stdoutfd = -1;
            int nextpid;

            if ((stdinfd = open (_PATH_DEVNULL, O_RDONLY)) < 0){
                if (stdoutfd != -1)
                    close (stdoutfd);

                if (stdinfd != -1)
                    close (stdinfd);

                return ITELIEC_FATAL;
            }

            if ((stdoutfd = open (_PATH_DEVNULL, O_WRONLY)) < 0){
                if (stdoutfd != -1)
                    close (stdoutfd);

                if (stdinfd != -1)
                    close (stdinfd);

                return ITELIEC_FATAL;
            }

            if (dup2 (stdinfd, STDIN_FILENO) != STDIN_FILENO){
                if (stdoutfd != -1)
                    close (stdoutfd);

                if (stdinfd != -1)
                    close (stdinfd);

                return ITELIEC_FATAL;
            }

            if (dup2 (stdoutfd, STDOUT_FILENO) != STDOUT_FILENO) {
                if (stdoutfd != -1)
                    close (stdoutfd);

                if (stdinfd != -1)
                    close (stdinfd);

                return ITELIEC_FATAL;
            }

            if (dup2 (stdoutfd, STDERR_FILENO) != STDERR_FILENO){
                if (stdoutfd != -1)
                    close (stdoutfd);

                if (stdinfd != -1)
                    close (stdinfd);

                return ITELIEC_FATAL;
            }

            if (close (stdinfd) < 0) {
                if (stdoutfd != -1)
                    close (stdoutfd);

                if (stdinfd != -1)
                    close (stdinfd);

                return ITELIEC_FATAL;
            }

            stdinfd = -1;

            if (close (stdoutfd) < 0) {
                if (stdoutfd != -1)
                    close (stdoutfd);

                if (stdinfd != -1)
                    close (stdinfd);

                return ITELIEC_FATAL;
            }

            stdoutfd = -1;

            if (chdir ("/") == -1) {
                if (stdoutfd != -1)
                    close (stdoutfd);

                if (stdinfd != -1)
                    close (stdinfd);

                return ITELIEC_FATAL;
            }

            if (setsid () < 0) {
                if (stdoutfd != -1)
                    close (stdoutfd);

                if (stdinfd != -1)
                    close (stdinfd);

                return ITELIEC_FATAL;
            }

            nextpid = fork ();

            switch (nextpid) {
                case 0:
                    return 0;
                case -1:
                    return ITELIEC_FATAL;
                default:
                    _exit (0);
            }
        }

        case -1:
            return ITELIEC_FATAL;

        default: {
            int got, status = 0;

            /* We wait to make sure the next child forked successfully */
            if ((got = waitpid(pid, &status, 0)) < 0 ||
                got != pid || status != 0) {
                
                return ITELIEC_FATAL;
            }

            _exit (0);
        }
    }
}

/* main run loop */
static int iteliec_run (const char *hash) {
    /* StatsGraph init */
    if (sg_init () != 0) {
        iteliec_log (ITELIEC_ERR, "%s: Error. Failed to initialize libgitstatsgraph", __func__);

        return ITELIEC_ERR;
    }

    if (sg_drop_privileges () != 0) {
        iteliec_log (ITELIEC_ERR, "%s: Error. Failed to drop privileges", __func__);

        return ITELIEC_ERR;
    }

    while (!down) {
        iteliec_log (ITELIEC_INFO, "Sent Statistics");

        iteliec_soap_init (hash);

        sleep (update_period);
    }

    return ITELIEC_OK;
}

static void iteliec_register (const char *argv0) {
    char login[BUFSIZ],  pwd[BUFSIZ], *buf;
    const char *hash;
    int err;

    config_t cfg;
    config_setting_t *setting;
    
    config_init (&cfg);

    /* Read the file. If there is an error, report it and exit. */
    if (!config_read_file (&cfg, cfile)) {
        printf ("Please ensure configuration file %s exists and is valid", cfile);
        printf ("\n%s:%d - %s", config_error_file (&cfg), config_error_line (&cfg), config_error_text (&cfg));
        
        config_destroy (&cfg);
        
        return;
    }

    /* check if server already configured */
    if (config_lookup_string(&cfg, "api.hash", &hash)) {
        printf ("HASH detected: '%s'\n", hash);
        printf ("Server already registered. Exiting...\n");

        return;
    }

    config_destroy (&cfg);

    printf("Stage 1. Login to Iteliec.com\n");

    /* Get the username */
    printf("Login: ");
    scanf("%s", login);    

    /* Copy to a "stable" pointer */
    buf = getpass("Passowrd: ");
    sprintf (pwd, "%s", buf);

    printf("\nStage 2. Register with Iteliec.com\n");
    iteliec_soap_register (login, pwd);
}

static void usage (const char *argv0) {
    fprintf (stderr, "\n\
Usage:\n\
%s [options]\n\
\n\
Options:\n\
    -v | --verbose             Verbose messages\n\
    -r | --register            Register new server\n\
    -d | --daemonize           Process will daemonize\n\
    -f | --config     <file>   Configuration file\n\
    -p | --pid-file   <file>   PID file\n\
    -u | --user       <user>   Drop root privs and run as <user>\n\
\n", argv0);
}

int main(int argc, char *argv[]) {
    struct sigaction sig_action;
    int verbose = 0;
    int no_daemonize = 1;
    int err;
    const char *user = NULL;
    const char *pfile = NULL;
    const char *hash;
    config_t cfg;
    config_setting_t *setting;


    struct option opts[] = {
        { "verbose", no_argument, &verbose, 1},
        { "no-daemonize", no_argument, &no_daemonize, 1},
        { "config", required_argument, NULL, 'f'},
        { "pid-file", required_argument, NULL, 'p'},
        { "user", required_argument, NULL, 'u'},
        { "register", no_argument, NULL, 'r' },
        { "help", no_argument, NULL, '?' },
        {0, 0, 0, 0}
    };

    while (1) {
        int optidx = 0;
        int c;

        c = getopt_long (argc, argv, "c:df:p:u::rf:v", opts, &optidx);

        if (c == -1) {
            break;
        }

        switch (c) {
            case 0:
                /* Got one of the flags */
            break;

            case 'v':
                verbose = 1;
            break;

            case 'd':
                no_daemonize = 0;
            break;

            case 'f':
                cfile = optarg;
            break;

            case 'p':
                pfile = optarg;
            break;

            case 'u':
                user = optarg;
            break;

            case 'r':
                iteliec_register (argv[0]);
                return ITELIEC_EXIT;

            case '?':
                usage (argv[0]);
                return ITELIEC_EXIT;

            default:
                fprintf (stderr, "iteliecd: unknown option: %c\n", c);
                exit (ITELIEC_ERR);
        }
    }

    iteliec_log_init (no_daemonize, verbose);

    /* Init config */
    config_init (&cfg);

    /* Read the file. If there is an error, report it and exit. */
    if (!config_read_file (&cfg, cfile)) {
        printf ("Please ensure configuration file %s exists and is valid", cfile);
        printf ("\n%s:%d - %s", config_error_file (&cfg), config_error_line (&cfg), config_error_text (&cfg));
        
        config_destroy (&cfg);
        
        return ITELIEC_ERR;
    }

    /* Get Server Hash from config */
    if (config_lookup_string (&cfg, "api.hash", &hash)) {
        iteliec_log (ITELIEC_INFO, "HASH loaded: %s", hash);
    } else {
        iteliec_log (ITELIEC_ERR, "No server hash found. Please run configuration.");

        return ITELIEC_ERR;
    }

    config_destroy (&cfg);

    /* Daemonize */
    if (!no_daemonize) {
        if (daemonize () < 0) {
            iteliec_log (ITELIEC_ERR, "Failed to fork as daemon: %s", strerror (errno));

            iteliec_log_close ();
            return ITELIEC_ERR;
        }
    }

    /* If running as root and no PID file is set, use the default */
    if (pfile == NULL && getuid () == 0 && pid_file[0] != '\0') {
        pfile = pid_file;
    }

    /* If we have a pidfile set, claim it now, exiting if already taken */
    if (pfile != NULL && write_pid_file (pfile) < 0) {
        if (pfile) {
            unlink (pfile);
        }

        iteliec_log_close ();

        return ITELIEC_ERR;
    }

    sig_action.sa_sigaction = sig_handler;
    sig_action.sa_flags = SA_SIGINFO;

    sigemptyset (&sig_action.sa_mask);

    sigaction (SIGHUP,  &sig_action, NULL);
    sigaction (SIGINT,  &sig_action, NULL);
    sigaction (SIGQUIT, &sig_action, NULL);
    sigaction (SIGTERM, &sig_action, NULL);

    /* Main Loop */
    iteliec_run (hash);

    config_destroy (&cfg);

    return ITELIEC_OK;
}

const char *iteliec_config_file () {
    return cfile;
}
