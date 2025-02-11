#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include<signal.h>
#include<unistd.h>


static int x = 53;
static int shift = 10;


int pragyan(){
    printf("########  ########     ###     ######   ##    ##    ###    ##    ## \n"
            "##     ## ##     ##   ## ##   ##    ##   ##  ##    ## ##   ###   ## \n"
            "##     ## ##     ##  ##   ##  ##          ####    ##   ##  ####  ## \n"
            "########  ########  ##     ## ##   ####    ##    ##     ## ## ## ## \n"
            "##        ##   ##   ######### ##    ##     ##    ######### ##  #### \n"
            "##        ##    ##  ##     ## ##    ##     ##    ##     ## ##   ### \n"
            "##        ##     ## ##     ##  ######      ##    ##     ## ##    ## \n\n\n");
    
    return 0;
}

void sig_handler(int signum){
	printf("Sorry, you took too long to respond!");
	exit(-1);
}

int pass(){
    unsigned int i;
    unsigned int v1;
    char c;
    char s[32];

    for (i = 0; i < 32; i += 4) {
        *(unsigned int *)&s[i] = 0;
    }    

    puts("Welcome to the digital vault of lost memories! ");
    puts("Enter the passcode to enter the lost memory world: ");

    printf(">>> ");
    fflush(stdout);

    fgets(s, sizeof(s), stdin);
    s[strlen(s) - 1] = '\0';
    for (int i = 0; s[i] != '\0'; i++) {
        c = s[i];
        if (isupper(c)) {
            s[i] = (c - 'A' + shift) % 26 + 'A';
        }
        else if (islower(c)) {
            s[i] = (c - 'a' + shift) % 26 + 'a';
        }
        s[i] ^= x;
    }
    return -(memcmp("cLVQjFMjcFDGQ", s, 0xd) != 0);
}

int init(){
    char s[128];

    memset(s,0,sizeof(s));

    puts("How should we address you? ");
    printf(">>> ");

    fgets(s, 128, stdin);

    printf("hello ");
    printf(s);
    printf("Here are the lost memories:");
    putc(10, stdout);
    system("ls *.pdf");
}

int main(){

    pragyan();

    signal(SIGALRM,sig_handler);
    alarm(4);
    int res;

    setvbuf(stdout, 0, 2, 0);
    setvbuf(stdout, 0, 2, 0);

    if (pass()){
        res = -1;
        fwrite("password mismatch!\n",1u, 0x17u, stderr);
    }
    else{
        res = 0;
        init();
    }
    return res;
}