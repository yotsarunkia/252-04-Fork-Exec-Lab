#include "fork_exec_lab.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int g_counter = 100;

pid_t spawn_child(void)
{
    return fork();
}

int run_child_process(int *heap_counter, int stack_counter)
{
    int child_sum;

    g_counter += 7;
    *heap_counter += 7;
    stack_counter += 7;

    child_sum = g_counter + *heap_counter + stack_counter;

    printf("child: g=%d h=%d s=%d sum=%d\n",
           g_counter, *heap_counter, stack_counter, child_sum);

    if (fflush(stdout) == EOF)
    {
        perror("fflush");
        return 1;
    }

    return child_sum % 256;
}

int wait_for_child(pid_t child_pid, int *heap_counter, int stack_counter)
{
    int status = 0;
    int child_code;
    int unchanged;

    if (waitpid(child_pid, &status, 0) == -1)
    {
        perror("waitpid");
        return 1;
    }

    if (!WIFEXITED(status))
    {
        fprintf(stderr, "parent: child terminated abnormally\n");
        return 1;
    }

    child_code = WEXITSTATUS(status);

    printf("parent: child-exit=%d\n", child_code);
    printf("parent: g=%d h=%d s=%d\n", g_counter, *heap_counter, stack_counter);

    unchanged = (g_counter == 100 && *heap_counter == 200 && stack_counter == 300);

    if (unchanged)
    {
        printf("parent: address-space=isolated\n");
    }
    else
    {
        printf("parent: address-space=unexpected\n");
    }

    if (fflush(stdout) == EOF)
    {
        perror("fflush");
        return 1;
    }

    return (child_code == 109 && unchanged) ? 0 : 1;
}

int main(void)
{
    int *heap_counter = malloc(sizeof *heap_counter);
    int stack_counter = 300;
    pid_t pid;
    int rc;

    if (heap_counter == NULL)
    {
        perror("malloc");
        return 1;
    }
    *heap_counter = 200;
    
    printf("parent: start g=%d h=%d s=%d\n",
           g_counter, *heap_counter, stack_counter);

    if (fflush(stdout) == EOF)
    {
        perror("fflush");
        free(heap_counter);
        return 1;
    }

    pid = spawn_child();
    if (pid == -1)
    {
        perror("fork");
        free(heap_counter);
        return 1;
    }

    if (pid == 0)
    {
        int code = run_child_process(heap_counter, stack_counter);
        free(heap_counter);
        _exit(code);
    }

    rc = wait_for_child(pid, heap_counter, stack_counter);
    free(heap_counter);
    return rc;
}