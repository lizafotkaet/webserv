#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h> 

int ft_popen(const char *file, char *const argv[], char type)
{
    int   fd[2];
    pid_t pid;

    if (!file || !argv || (type != 'r' && type != 'w'))
        return (-1);
    if (pipe(fd) == -1)
        return (-1);
    pid = fork();
    if (pid == -1)
    {
        close(fd[0]);
        close(fd[1]);
        return (-1);
    }
    if (pid == 0) /* child */
    {
        if (type == 'r')
            dup2(fd[1], STDOUT_FILENO); /* command's output goes into pipe */
        else
            dup2(fd[0], STDIN_FILENO);  /* command reads its input from pipe */
        close(fd[0]);
        close(fd[1]);
        execvp(file, argv);
        exit(1); /* only reached if execvp fails */
    }
    /* parent */
    if (type == 'r')
    {
        close(fd[1]);   /* we don't write, only read */
        return (fd[0]);
    }
	//	CHECK when/why we need fcntl LATER!!!
	fcntl(fd[0], F_SETFL, O_NONBLOCK); 
    close(fd[0]);       /* we don't read, only write */
    return (fd[1]);
}
