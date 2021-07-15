

/* functions to spawn foreground/background jobs */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Some of the functions below fail if the control tty can't be
      * located, or if the calling process isn't in the foreground. In the
      * first case, we are assuming that a foreground process will have the
      * ctty open on either stdin, stdout or stderr, and we return ENOTTY
      * if it isn't. In the second case, we return EPERM if a
      * non-foreground process attempts to put something into the
      * foreground (probably overly paranoid) except for the special case
      * of foreground_self().
      */

/* assign the terminal (open on ctty) to a specific pgrp. This wrapper
      * around tcsetpgrp() is needed only because of extreme bogosity on the
      * part of POSIX; conforming systems deliver STGTTOU if tcsetpgrp is
      * called from a non-foreground process (which it almost invariably is).
      * A triumph of spurious consistency over common sense.
      */

int assign_terminal(int ctty, pid_t pgrp)
{
    sigset_t sigs;
    sigset_t oldsigs;
    int rc;

    sigemptyset(&sigs);
    sigaddset(&sigs, SIGTTOU);
    sigprocmask(SIG_BLOCK, &sigs, &oldsigs);

    rc = tcsetpgrp(ctty, pgrp);

    sigprocmask(SIG_SETMASK, &oldsigs, NULL);

    return rc;
}

/* Like fork(), but does job control. FG is true if the newly-created
      * process is to be placed in the foreground. (This implicitly puts
      * the calling process in the background, so watch out for tty I/O
      * after doing this.) PGRP is -1 to create a new job, in which case
      * the returned pid is also the pgrp of the new job, or specifies an
      * existing job in the same session (normally used only for starting
      * second or subsequent process in a pipeline).  */

pid_t spawn_job(int fg, pid_t pgrp)
{
    int ctty = -1;
    pid_t pid;

    /* If spawning a *new* foreground job, require that at least one
          * of stdin, stdout or stderr refer to the control tty, and that
          * the current process is in the foreground.
          * Only check for controlling tty if starting a new foreground
          * process in an existing job.
          * A session without a control tty can only have background jobs
          */

    if (fg)
    {
        pid_t curpgrp;

        if ((curpgrp = tcgetpgrp(ctty = 2)) < 0 && (curpgrp = tcgetpgrp(ctty = 0)) < 0 && (curpgrp = tcgetpgrp(ctty = 1)) < 0)
            return errno = ENOTTY, (pid_t)-1;

        if (pgrp < 0 && curpgrp != getpgrp())
            return errno = EPERM, (pid_t)-1;
    }

    switch (pid = fork())
    {
    case -1: /* fork failure */
        return pid;

    case 0: /* child */

        /* establish new process group, and put ourselves in
                  * foreground if necessary
                  * unclear what to do if setpgid fails ("can't happen")
                  */

        if (pgrp < 0)
            pgrp = getpid();

        if (setpgid(0, pgrp) == 0 && fg)
            assign_terminal(ctty, pgrp);

        return 0;

    default: /* parent */

        /* establish child process group here too. */

        if (pgrp < 0)
            pgrp = pid;

        setpgid(pid, pgrp);

        return pid;
    }

    /*NOTREACHED*/
}

/* Kill job PGRP with signal SIGNO */

int kill_job(pid_t pgrp, int signo)
{
    return kill(-pgrp, signo);
}

/* Suspend job PGRP */

int suspend_job(pid_t pgrp)
{
    return kill_job(pgrp, SIGSTOP);
}

/* Resume job PGRP in background */

int resume_job_bg(pid_t pgrp)
{
    return kill_job(pgrp, SIGCONT);
}

/* resume job PGRP in foreground */

int resume_job_fg(pid_t pgrp)
{
    pid_t curpgrp;
    int ctty;

    if ((curpgrp = tcgetpgrp(ctty = 2)) < 0 && (curpgrp = tcgetpgrp(ctty = 0)) < 0 && (curpgrp = tcgetpgrp(ctty = 1)) < 0)
        return errno = ENOTTY, (pid_t)-1;

    if (curpgrp != getpgrp())
        return errno = EPERM, (pid_t)-1;

    if (assign_terminal(ctty, pgrp) < 0)
        return -1;

    return kill_job(pgrp, SIGCONT);
}

/* put ourselves in the foreground, e.g. after suspending a foreground
      * job
      */

int foreground_self()
{
    pid_t curpgrp;
    int ctty;

    if ((curpgrp = tcgetpgrp(ctty = 2)) < 0 && (curpgrp = tcgetpgrp(ctty = 0)) < 0 && (curpgrp = tcgetpgrp(ctty = 1)) < 0)
        return errno = ENOTTY, (pid_t)-1;

    return assign_terminal(ctty, getpgrp());
}

/* closeall() - close all FDs >= a specified value */

void closeall(int fd)
{
    int fdlimit = sysconf(_SC_OPEN_MAX);

    while (fd < fdlimit)
        close(fd++);
}

/* like system(), but executes the specified command as a background
      * job, returning the pid of the shell process (which is also the pgrp
      * of the job, suitable for kill_job etc.)
      * If INFD, OUTFD or ERRFD are non-NULL, then a pipe will be opened and
      * a descriptor for the parent end of the relevent pipe stored there.
      * If any of these are NULL, they will be redirected to /dev/null in the
      * child.
      * Also closes all FDs > 2 in the child process (an oft-overlooked task)
      */

pid_t spawn_background_command(const char *cmd,
                               int *infd, int *outfd, int *errfd)
{
    int nullfd = -1;
    int pipefds[3][2];
    int error = 0;

    if (!cmd)
        return errno = EINVAL, -1;

    pipefds[0][0] = pipefds[0][1] = -1;
    pipefds[1][0] = pipefds[1][1] = -1;
    pipefds[2][0] = pipefds[2][1] = -1;

    if (infd && pipe(pipefds[0]) < 0)
        error = errno;
    else if (outfd && pipe(pipefds[1]) < 0)
        error = errno;
    else if (errfd && pipe(pipefds[2]) < 0)
        error = errno;

    if (!error && !(infd && outfd && errfd))
    {
        nullfd = open("/dev/null", O_RDWR);
        if (nullfd < 0)
            error = errno;
    }

    if (!error)
    {
        pid_t pid = spawn_job(0, -1);
        switch (pid)
        {
        case -1: /* fork failure */
            error = errno;
            break;

        case 0: /* child proc */

            dup2(infd ? pipefds[0][0] : nullfd, 0);
            dup2(outfd ? pipefds[1][1] : nullfd, 1);
            dup2(errfd ? pipefds[2][1] : nullfd, 2);
            closeall(3);

            execl(/* "/bin/sh", "sh", "-c",  */ cmd, "", NULL);

            _exit(127);

        default: /* parent proc */

            close(nullfd);
            if (infd)
                close(pipefds[0][0]), *infd = pipefds[0][1];
            if (outfd)
                close(pipefds[1][1]), *outfd = pipefds[1][0];
            if (errfd)
                close(pipefds[2][1]), *errfd = pipefds[2][0];

            return pid;
        }
    }

    /* only reached if error */

    {
        int i, j;
        for (i = 0; i < 3; ++i)
            for (j = 0; j < 2; ++j)
                if (pipefds[i][j] >= 0)
                    close(pipefds[i][j]);
    }

    if (nullfd >= 0)
        close(nullfd);

    return errno = error, (pid_t)-1;
}

/*--------------------------------------------------------------------*/
/* This bit is a somewhat trivial example of using the above.         */

pid_t bgjob = -1;
volatile int signo = 0;

#ifndef WCOREDUMP
/* If WCOREDUMP is missing, you might want to supply a correct
       * definition for your platform (this is usually (status & 0x80) but
       * not always) or punt (as in this example) by assuming no core dumps.
       */
#define WCOREDUMP(status) (0)
#endif

int check_children()
{
    pid_t pid;
    int status;
    int count = 0;

    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0)
    {
        if (pid == bgjob && !WIFSTOPPED(status))
            bgjob = -1;

        ++count;

        if (WIFEXITED(status))
            fprintf(stderr, "Process %ld exited with return code %d\n",
                    (long)pid, WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            fprintf(stderr, "Process %ld killed by signal %d%s\n",
                    (long)pid, WTERMSIG(status),
                    WCOREDUMP(status) ? " (core dumped)" : "");
        else if (WIFSTOPPED(status))
            fprintf(stderr, "Process %ld stopped by signal %d\n",
                    (long)pid, WSTOPSIG(status));
        else
            fprintf(stderr, "Unexpected status - pid=%ld, status=0x%x\n",
                    (long)pid, status);
    }

    return count;
}

void sighandler(int sig)
{
    if (sig != SIGCHLD)
        signo = sig;
}
/* 
int main()
{
    struct sigaction act;
    int sigcount = 0;

    act.sa_handler = sighandler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGTSTP, &act, NULL);
    sigaction(SIGCHLD, &act, NULL);

    fprintf(stderr, "Starting background job 'sleep 60'\n");
    bgjob = spawn_background_command("sleep 60", NULL, NULL, NULL);
    if (bgjob < 0)
    {
        perror("spawn_background_command");
        exit(1);
    }
    fprintf(stderr, "Background job started with id %ld\n", (long)bgjob);
    while (bgjob >= 0)
    {
        if (signo)
        {
            fprintf(stderr, "Signal %d caught\n", signo);
            if (sigcount++)
                kill_job(bgjob, SIGKILL);
            else
            {
                kill_job(bgjob, SIGTERM);
                kill_job(bgjob, SIGCONT);
            }
        }

        if (!check_children())
            pause();
    }

    fprintf(stderr, "Done - exiting\n");
    return 0;
} */

int fd_is_valid(int fd)
{
    return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}