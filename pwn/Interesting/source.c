#include <stdio.h>
#include <stdlib.h>

void interesting()
{
    
    printf("Yeah, that's interesting\n");
    FILE *file;
    char flag[256];

    file = fopen("flag.txt", "r");
    if (file == NULL) 
    {
        perror("Error opening file, Contact Admin");
        exit(1);
    }

    if (fgets(flag, sizeof(flag), file) != NULL) 
    {
        printf("%s", flag);
    }
    fclose(file);
}

int fun()
{
    char buf[240];
    char buf2[24];
    fgets(buf, 230, stdin);
    printf("You said: ");
    printf(buf);
    printf("Do you really think that's interesting?\n");
    gets(buf2);
    return 7;
}

int main()
{

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    printf( "########  ########     ###     ######   ##    ##    ###    ##    ## \n"
            "##     ## ##     ##   ## ##   ##    ##   ##  ##    ## ##   ###   ## \n"
            "##     ## ##     ##  ##   ##  ##          ####    ##   ##  ####  ## \n"
            "########  ########  ##     ## ##   ####    ##    ##     ## ## ## ## \n"
            "##        ##   ##   ######### ##    ##     ##    ######### ##  #### \n"
            "##        ##    ##  ##     ## ##    ##     ##    ##     ## ##   ### \n"
            "##        ##     ## ##     ##  ######      ##    ##     ## ##    ## \n\n\n");

    printf("Hey there! Welcome to the challenge\n");
    printf("The rules are simple! Say something interesting, and I will give you the flag.\n");
    
    if (fun() == 7) 
    {
        printf("Nah! You are boring\n");
    }

    return 0;

}