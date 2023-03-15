#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

char *read_line(void);
char **parse_line(char *);
int execute(char **);
int ifexec(char **args);

char *paths[256] = {"/bin"};
int numPaths = 1;

void run(void)
{
  char *line;
  char **args;
  int status;

  do
  {
    printf("wish> ");
    line = read_line();
    args = parse_line(line);
    
    status = execute(args);

    free(line);
    free(args);
  } while (status);
}

void run_batch(char *argv)
{
  char *line = NULL;
  size_t bufsize = 0;
  int num_lines = 0;
  ssize_t line_size;

  char **args;

  FILE *fp = fopen(argv, "r");
  if (!fp)
  {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(1);
  }

  line_size = getline(&line, &bufsize, fp);

  while (line_size >= 0)
  {
    char whitespace[] = " \t\r\n\v";
    for (; *line && strchr(whitespace, *line); line++);
    if (*line != '\0' && *line != '\n') {

    num_lines++;

    char *newLine = (char *)malloc((strlen(line)) * sizeof(char));
    int i = 0, j = 0;
    while (line[i] != '\0')
    {
      if (line[i] == '>')
      {
        if (line[i - 1] != ' ')
        {
          newLine[j++] = ' ';
        }
        newLine[j] = line[i];
        if (line[i + 1] != ' ')
        {
          newLine[++j] = ' ';
        }
        i++;
        j++;
      }
      else
      {
        newLine[j++] = line[i++];
      }
    }
    
    size_t size;
    char* end;

    size = strlen(line);

    end = line + size - 1;
    while (end >= line && isspace(*end))
        end--;
    *(end + 1) = '\0';

    while (*line && isspace(*line))
        line++;

    line[strcspn(line, "\n")] = '\0';
    args = parse_line(line);
    execute(args);
  }

    line_size = getline(&line, &bufsize, fp);
  }

  fclose(fp);
  exit(0);
}

char *read_line(void)
{
  char *line = NULL;
  size_t bufsize = 0;

  if (getline(&line, &bufsize, stdin) == -1)
  {
    if (feof(stdin))
    {
      exit(0);
    }
    else
    {
      char error_message[30] = "An error has occurred\n";
      write(STDERR_FILENO, error_message, strlen(error_message));
    }
  }

  char *newLine = (char *)malloc((strlen(line)) * sizeof(char));
  int i = 0, j = 0;
//  char whitespace[] = " \t\r\n\v";
//  while (strchr(whitespace, *line))
//    i++;
  
  while (line[i] != '\0')
  {
//    if (*line == '\0')
//      continue;
    if (line[i] == '>')
    {
      if (line[i - 1] != ' ')
      {
        newLine[j++] = ' ';
      }
      newLine[j] = line[i];
      if (line[i + 1] != ' ')
      {
        newLine[++j] = ' ';
      }
      i++;
      j++;
    }
    else
    {
      newLine[j++] = line[i++];
    }
  }
  return newLine;
}

char **parse_line(char *line)
{
  int bufsize = 256;
  char *args;
  char **arguments = malloc(bufsize * sizeof(char *));
  int temp = 0;

  while ((args = strsep(&line, " ")) != NULL)
  {
    args[strcspn(args, "\n")] = '\0';
    arguments[temp] = args;
    temp++;

    if (temp >= bufsize)
    {
      bufsize += bufsize;
      arguments = realloc(arguments, bufsize * sizeof(char *));
    }
  }
  arguments[temp] = NULL;

  return arguments;
}

int execute(char **args)
{
  // Built-in commands
  int numArgs = 0;
  while (args[numArgs] != NULL)
  {
    numArgs++;
  }

  if (!strcmp(args[0], "exit"))
  {
    if (numArgs == 1)
    {
      exit(0);
    }
    else
    {
      char error_message[30] = "An error has occurred\n";
      write(STDERR_FILENO, error_message, strlen(error_message));
      return 1;
    }
  }
  else if (!strcmp(args[0], "cd"))
  {
    if (numArgs == 2)
    {
      if (!chdir(args[1]))
      {
        return 1;
      }
    }
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
    return 1;
  }
  else if (!strcmp(args[0], "path"))
  {
    memset(paths, 0, 256);
    numPaths = 0;
    int i = 0;
    int j = 1;
    while (j < numArgs)
    {
      paths[i++] = strdup(args[j++]);
      numPaths++;
    }
    paths[i] = NULL;

    return 1;
  }

//  if (numPaths != 0) {
  if (!strcmp(args[0], "if"))
  {
    return ifexec(args);
  }

  char fp[256];
  int rc = fork();
  if (rc < 0)
  { // fork failed
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(0);
  }
  else if (rc == 0)
  { // child
    int i = 0;
    while (paths[i] != NULL)
    {
      char* fp = malloc(256*sizeof(char));

      char c[256] = {};
      char o[256] = {};

      int a = 0;
      while (a < numArgs)
      {
	if (!strcmp(args[a], ">")) {
	  break;
	}
	a++;
      }

      if (a < numArgs) {
	if ((a == 0) || (a == numArgs-1)) {
	  char error_message[30] = "An error has occurred\n";
	  write(STDERR_FILENO, error_message, strlen(error_message));
	  exit(0);
	}
      }

      a = 0;
      while (a < numArgs && strcmp(args[a], ">"))
      { // Split up command and output file
        strcat(c, args[a]);
        a++;
      }

      if (a < numArgs)
      { // File redirect

        args[a] = NULL;
        a++;
        while (a < numArgs)
        {
          strcat(o, args[a]);
          if (++a < numArgs)
          {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(0);
          }
        }
	memset(fp, 0, strlen(fp));
	strcat(fp, paths[i]);
        if (fp[strlen(fp)-1] != '/') {
            fp[strlen(fp)] = '/';
	}
        strcat(fp, args[0]);
	fp[strlen(fp)+1] = '\0';

        if (!access(fp, X_OK))
        {
          int fd = open(o, O_WRONLY | O_CREAT | O_TRUNC, 0644);
          dup2(fd, fileno(stdout));
          close(fd);
          execv(fp, args);
        }
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(0);
      }
      else
      { // Normal command
	memset(fp, 0, strlen(fp));
        strcat(fp, paths[i]);
        
	if (fp[strlen(fp)-1] != '/') {
            fp[strlen(fp)] = '/';
        }   
        strcat(fp, args[0]); 
        fp[strlen(fp)+1] = '\0';


        if (!access(fp, X_OK))
        {
          execv(fp, args);
        }
      }
      i++;
    }
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(0);
  }
  else
  {
    int rc_wait = wait(NULL);
  }
  return 1;
}

int ifexec(char **args)
{
  int numArgs = 0;
  int ifcount = 0, thencount = 0, ficount = 0;
  while (args[numArgs] != NULL)
  {
    if (strcmp(args[numArgs], "if") == 0) {
      ifcount++;
    } else if (strcmp(args[numArgs], "then") == 0) {
      thencount++;
      if (args[numArgs] +1 == NULL)
	exit(1);
    } else if (strcmp(args[numArgs], "fi") == 0) {
      ficount++;
    }
    numArgs++;
  }

  if (!strcmp(args[0], "if")) {
    if (strcmp(args[numArgs-1], "fi")) {
      char error_message[30] = "An error has occurred\n";
      exit(1);
      return 0;
    }
  }

  // Check input is correct
  if ((ifcount != thencount) || (thencount != ficount) || (ifcount != ficount)) {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(1);
    return 0;
  }

  char *ifstatement[256];
  int i = 0, j = 1;
  while (strcmp(args[j], "then"))
  {
    ifstatement[i++] = args[j++];
  }

  // IF CONDITION (BEFORE EQUALITY SIGN)
  i = 0;
  j = 0;
  char *ifcondition[256];
  while (ifstatement[i])
  {
    if ((strcmp(ifstatement[i], "==") == 0) || (strcmp(ifstatement[i], "!=") == 0))
    {
      break;
    }
    ifcondition[j++] = ifstatement[i++];
  }
  ifcondition[j] = NULL;

  // EQUALITY SIGN
  j = 0;
  char *equality = ifstatement[i++];

  // IF RESULT (AFTER EQUALITY SIGN)
  char *ifresult[256];

  while (ifstatement[i])
  {
    if (strcmp(ifstatement[i], "then") == 0)
    {
      break;
    }
    ifresult[j++] = ifstatement[i++];
  }
  ifresult[j] = NULL;

  // THEN STATEMENT (AFTER "THEN")
  char *thenstatement[256];
  i = 0, j = 0;
  while (strcmp(args[j], "then"))
  {
    j++;
  }
  j++;
  while (j < numArgs-1)
  {
    thenstatement[i++] = args[j++];
  }
  thenstatement[i] = NULL;

  int rc = fork();
  if (rc < 0)
  { // fork failed
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(0);
  }
  else if (rc == 0)
  { // child
    char fp[256];
    strcpy(fp, "./");
    strcat(fp, ifcondition[0]);
    
    execv(fp, ifcondition);
  }
  else
  {
    int status;
    waitpid(rc, &status, 0);

    if (WIFEXITED(status))
    {
      int exit_status = WEXITSTATUS(status);
      char *ptr = NULL;
      if (!strcmp(equality, "==") && (exit_status == atoi(ifresult[0])))
      {
        execute(thenstatement);
        return 1;
      }
      else if (!strcmp(equality, "!=") && (exit_status != atoi(ifresult[0])))
      {
        execute(thenstatement);
        return 1;
      }
      else
      {
        return 0;
      }
    }
  }
  return 0;
}

int main(int argc, char *argv[])
{
  if (argc == 1) {
    run();
    return 0;
  } else if (argc == 2) {
    run_batch(argv[1]);
    return 0;
  } else {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
    return(1);
  }
  return 0;
}
