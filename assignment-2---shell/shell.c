#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

// Simplifed xv6 shell.

#define MAXARGS 10

// All commands have at least a type. Have looked at the type, the code
// typically casts the *cmd to some specific cmd type.
struct cmd {
  int type;          //  ' ' (exec), | (pipe), '<' or '>' for redirection
};

struct execcmd {
  int type;              // ' '
  char *argv[MAXARGS];   // arguments to the command to be exec-ed
};

struct redircmd {
  int type;          // < or > 
  struct cmd *cmd;   // the command to be run (e.g., an execcmd)
  char *file;        // the input/output file
  int mode;          // the mode to open the file with
  int fd;            // the file descriptor number to use for the file
};

struct pipecmd {
  int type;          // |
  struct cmd *left;  // left side of pipe
  struct cmd *right; // right side of pipe
};

int fork1(void);  // Fork but exits on failure.
struct cmd *parsecmd(char*);

// Execute cmd.  Never returns.
void
runcmd(struct cmd *cmd)
{
  int p[2], r;
  struct execcmd *ecmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;

  if(cmd == 0)
    exit(0);
  
  switch(cmd->type){
  default:
    fprintf(stderr, "unknown runcmd\n");
    exit(-1);

  case ' ':
    ecmd = (struct execcmd*)cmd;
    if(ecmd->argv[0] == 0)
      exit(0);
    // Your code here ...

    execvp(ecmd->argv[0], ecmd->argv); 
    // with execution, we will pass to execvp() function the command we want to run as ecmd->argv[0] and the command's arguments as ecmd->argv. 

    break;

  case '>':
  case '<':
    rcmd = (struct redircmd*)cmd;
    // Your code here ...

    // with redirection, we will first use open() to see if the file is accessible, i.e. whether it can be opened or not, and whether it can be created or not
    // we want to check whether the file can be accessed or not and we will give our open() all accesses with 0777 in the permissions parameter
    int my_file;
    my_file = open(rcmd->file, rcmd->mode, 0777);
    if(my_file < 0){
        fprintf(stderr, "Error: opening (%s", rcmd->file);
        exit(1);
    }

    // Next, we will create a duplicate file descriptor which points to my_file. 
    // And depending on the value of rcmd->fd, the value of the file descriptor will be assigned either 0 or 1 such that our process can gain resources to STDIN and STDOUT.
    int my_file2 = dup2(my_file, rcmd->fd);
    if(my_file2 < 0){
        fprintf(stderr, "Error: dup2\n");
        exit(1);
    }
    
    //now we just run the command
    runcmd(rcmd->cmd);
    
    break;

  case '|':
    pcmd = (struct pipecmd*)cmd;
    // fprintf(stderr, "pipe not implemented\n");
    // Your code here ...

    // with setting up our pipe, such as: ping -c 5 something.com | grep something
    // what we want to do here is to take the input from "ping" command and feed it as the input to grep - i.e. left to right.
    
    // first, we will declare a file descriptor array of length 2 and feed it to pipe so pipe knows where to read(p[0]) and write(p[1])) files
    int pipe_descriptors[2];
    int my_pipe = pipe(pipe_descriptors);

    // check if pipe() returned an error
    if(my_pipe == -1){
        fprintf(stderr, "Error: pipe\n");
        exit(1);
    }

    // After calling pipe(), now we call fork() so that the file descriptors in pipe() can be copied over
    int my_process = fork();

    if (my_process == -1){
        fprintf(stderr, "Error: fork");
        exit(1);
    }

    if (my_process == 0){
        // change the file descriptor of the pipe to STDOUT since this will execute left side of the pipe
        if(dup2(pipe_descriptors[1], 1) < 0){
            fprintf(stderr, "Error: dup2\n");
            exit(1);
        }

        // close the read descriptor since it is a child process.
        close(pipe_descriptors[0]);
        // close the write descriptor
        close(pipe_descriptors[1]);

        //run leftside of the pipe
        runcmd(pcmd->left);
    } 
    
    int my_process2 = fork();

    if (my_process2 == -1){
        fprintf(stderr, "Error: fork");
        exit(1);
    }

    if (my_process2 == 0){
        // change the file descriptor of the pipe to STDIN since this child process will run the right side of the pipe
        if(dup2(pipe_descriptors[0], 0) < 0){
            fprintf(stderr, "Error: dup2\n");
            exit(1);
        }

        // close the write descriptor
        close(pipe_descriptors[1]);
        //close the read descriptor
        close(pipe_descriptors[0]);
        
        //run right side of the pipe
        runcmd(pcmd->right);
        wait(&r);
    }

    // closing parent pipes
    close(pipe_descriptors[0]);
    close(pipe_descriptors[1]);

    // wait for children
    waitpid(my_process, NULL, 0);
    waitpid(my_process2, NULL, 0);
    break;
  }    
  exit(0);
}

int
getcmd(char *buf, int nbuf)
{
  
  if (isatty(fileno(stdin)))
    fprintf(stdout, "cs6233> ");
  memset(buf, 0, nbuf);
  fgets(buf, nbuf, stdin);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

int
main(void)
{
  static char buf[100];
  int fd, r;

  // Read and run input commands.
  while(getcmd(buf, sizeof(buf)) >= 0){
    if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
      // Clumsy but will have to do for now.
      // Chdir has no effect on the parent if run in the child.
      buf[strlen(buf)-1] = 0;  // chop \n
      if(chdir(buf+3) < 0)
        fprintf(stderr, "cannot cd %s\n", buf+3);
      continue;
    }
    if(fork1() == 0)
      runcmd(parsecmd(buf));
    wait(&r);
  }
  exit(0);
}

int
fork1(void)
{
  int pid;
  
  pid = fork();
  if(pid == -1)
    perror("fork");
  return pid;
}

struct cmd*
execcmd(void)
{
  struct execcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = ' ';
  return (struct cmd*)cmd;
}

struct cmd*
redircmd(struct cmd *subcmd, char *file, int type)
{
  struct redircmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = type;
  cmd->cmd = subcmd;
  cmd->file = file;
  cmd->mode = (type == '<') ?  O_RDONLY : O_WRONLY|O_CREAT|O_TRUNC;
  cmd->fd = (type == '<') ? 0 : 1;
  return (struct cmd*)cmd;
}

struct cmd*
pipecmd(struct cmd *left, struct cmd *right)
{
  struct pipecmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = '|';
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

// Parsing

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>";

int
gettoken(char **ps, char *es, char **q, char **eq)
{
  char *s;
  int ret;
  
  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  if(q)
    *q = s;
  ret = *s;
  switch(*s){
  case 0:
    break;
  case '|':
  case '<':
    s++;
    break;
  case '>':
    s++;
    break;
  default:
    ret = 'a';
    while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
      s++;
    break;
  }
  if(eq)
    *eq = s;
  
  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return ret;
}

int
peek(char **ps, char *es, char *toks)
{
  char *s;
  
  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return *s && strchr(toks, *s);
}

struct cmd *parseline(char**, char*);
struct cmd *parsepipe(char**, char*);
struct cmd *parseexec(char**, char*);

// make a copy of the characters in the input buffer, starting from s through es.
// null-terminate the copy to make it a string.
char 
*mkcopy(char *s, char *es)
{
  int n = es - s;
  char *c = malloc(n+1);
  assert(c);
  strncpy(c, s, n);
  c[n] = 0;
  return c;
}

struct cmd*
parsecmd(char *s)
{
  char *es;
  struct cmd *cmd;

  es = s + strlen(s);
  cmd = parseline(&s, es);
  peek(&s, es, "");
  if(s != es){
    fprintf(stderr, "leftovers: %s\n", s);
    exit(-1);
  }
  return cmd;
}

struct cmd*
parseline(char **ps, char *es)
{
  struct cmd *cmd;
  cmd = parsepipe(ps, es);
  return cmd;
}

struct cmd*
parsepipe(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parseexec(ps, es);
  if(peek(ps, es, "|")){
    gettoken(ps, es, 0, 0);
    cmd = pipecmd(cmd, parsepipe(ps, es));
  }
  return cmd;
}

struct cmd*
parseredirs(struct cmd *cmd, char **ps, char *es)
{
  int tok;
  char *q, *eq;

  while(peek(ps, es, "<>")){
    tok = gettoken(ps, es, 0, 0);
    if(gettoken(ps, es, &q, &eq) != 'a') {
      fprintf(stderr, "missing file for redirection\n");
      exit(-1);
    }
    switch(tok){
    case '<':
      cmd = redircmd(cmd, mkcopy(q, eq), '<');
      break;
    case '>':
      cmd = redircmd(cmd, mkcopy(q, eq), '>');
      break;
    }
  }
  return cmd;
}

struct cmd*
parseexec(char **ps, char *es)
{
  char *q, *eq;
  int tok, argc;
  struct execcmd *cmd;
  struct cmd *ret;
  
  ret = execcmd();
  cmd = (struct execcmd*)ret;

  argc = 0;
  ret = parseredirs(ret, ps, es);
  while(!peek(ps, es, "|")){
    if((tok=gettoken(ps, es, &q, &eq)) == 0)
      break;
    if(tok != 'a') {
      fprintf(stderr, "syntax error\n");
      exit(-1);
    }
    cmd->argv[argc] = mkcopy(q, eq);
    argc++;
    if(argc >= MAXARGS) {
      fprintf(stderr, "too many args\n");
      exit(-1);
    }
    ret = parseredirs(ret, ps, es);
  }
  cmd->argv[argc] = 0;
  return ret;
}
