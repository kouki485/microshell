#include <libc.h>

void error(char *str,char *arg)
{
	int i = 0;
	int n = 0;

	while(str[i])
		write(2,&str[i++],1);
	if(arg)
		while(arg[n])
			write(2,&arg[n++],1);
	write(2,"\n",1);
}

void execute(char **av,int i,int tmp_fd,char **env)
{
	av[i] = NULL;
	dup2(tmp_fd,0);
	close(tmp_fd);
	execve(av[0],av,env);
	error("error: cannot execute ",av[0]);
}

int main(int ac,char **av,char **env)
{
	(void)ac;
	int i = 0;
	int tmp_fd = dup(0);
	int fd[2];

	while(av[i] && av[i + 1])
	{
		av = &av[i + 1];
		i = 0;
		while(av[i] && strcmp(av[i],";") && strcmp(av[i],"|"))
			i++;
		if(strcmp(av[0],"cd") == 0)
		{
			if(i != 2)
				error("error: cd: bad arguments",NULL);
			else if(chdir(av[1]))
				error("error: cd: cannot change directory to ",av[1]);
		}
		else if (i != 0 && (av[i] == NULL || strcmp(av[i],";") == 0))
		{
			if(fork() == 0)
				execute(av,i,tmp_fd,env);
			else
			{
				close(tmp_fd);
				while(waitpid(-1,NULL,2) != -1);
				tmp_fd = dup(0);
			}
		}
		else if (i != 0 && strcmp(av[i],"|") == 0)
		{
			pipe(fd);
			if(fork() == 0)
			{
				dup2(fd[1],1);
				close(fd[0]);
				close(fd[1]);
				execute(av,i,tmp_fd,env);
			}
			else
			{
				close(tmp_fd);
				close(fd[1]);
				tmp_fd = fd[0];
			}
		}
	}
	close(tmp_fd);
}