#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <iostream>
#include <vector>
#include <string>
#include <string.h>
using namespace std;


int main()
{
    while (true)
    {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
            printf("%s$ ", cwd);
        else
            perror("getcwd() error");
//         cout << getcwd () << "$ " ;
        char command[1024];
        cin.getline(command, 1024);
        if (cin.eof())
        {
            printf("\nProcess completed\n");
            return 0;
        }
        if (strlen(command) == 0)
        {
            continue;
        }
        vector<char*> tmp2;
        char* cmd = strtok(command, " ");
        char* tmp = cmd;
        while (tmp != NULL)
        {
            tmp2.push_back(tmp);
            tmp = strtok(NULL, " ");
        }

        char** argv = new char*[tmp2.size() + 1];
        for (int k = 0; k < tmp2.size(); k++)
            argv[k] = tmp2[k];

        argv[tmp2.size()] = NULL;

        if (strcmp(command, "exit") == 0)
        {
            delete[] argv;
            return 0;
        }
        if (!strcmp (cmd, "cd"))
        {
            if (argv[1] == NULL)
                chdir ("/");
            else
                chdir (argv[1]);
            perror (command);
        }
        else
        {
            int status;
            pid_t kidpid = fork();

            if (kidpid < 0)
            {
                perror("Internal error: cannot fork.");
                delete[] argv;
                return -1;
            }
            else if (kidpid == 0)
            {
                if (execvp (cmd, argv) < 0)
                {
                    perror(command);
                    delete[] argv;
                    return -1;
                }
                delete[] argv;
            }
            else
            {
                waitpid(kidpid, &status, 0);                
            }
        }
        delete[] argv;
    }

    return 0;
}