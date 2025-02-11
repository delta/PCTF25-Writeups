#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

unsigned long *alloc_array[3];
int alloc_count = 0, alloc_check[3] = {0}, alloc_size[3];

void win() {
    system("/bin/cat flag.txt");
}

void menu() {
    printf("\n--- Gates of Valhalla ---\n");
    printf("1. Summon a warrior (Allocate memory)\n");
    printf("2. Send to Valhalla (Free)\n");
    printf("3. Bestow power (Write)\n");
    printf("4. Prove your worth (Check the win condition)\n");
    printf("5. Retreat (Exit)\n");
    printf("-------------------------\n");
}

void alloc() {
    int size;
    if (alloc_count >= 3) {
        printf("[!] The halls are full. No more warriors can be summoned.\n");
        return;
    }
    printf("[*] Maximum strength is 128 units.\n");
    printf("[+] Choose the strength of the warrior:");
    scanf("%d", &size);
    if (size > 128 || size <= 0) {
        printf("[!] Strength is too high or invalid.\n");
        return;
    }
    alloc_array[alloc_count] = malloc(size);
    if (alloc_array[alloc_count] == NULL) {
        printf("[!] Summoning failed.\n");
        exit(1);
    }
    printf("[*] Warrior summoned at %p\n", alloc_array[alloc_count]);
    alloc_check[alloc_count] = 1;
    alloc_size[alloc_count] = size;
    alloc_count++;
}

void nuke() {
    int index;
    printf("[+] Choose the warrior to send to Valhalla (0-2):");
    scanf("%d", &index);
    if (index >= 3 || index < 0) {
        printf("[!] Invalid choice.\n");
        return;
    }
    if (alloc_check[index] == 1) {
        memset(alloc_array[index], 0, alloc_size[index]);
        free((void *)alloc_array[index]);
        printf("[*] Warrior at index %d has ascended to Valhalla.\n", index);
        alloc_check[index] = 0;
        alloc_array[index] = NULL;
        alloc_count--;
    } else {
        printf("[!] No warrior at index %d to ascend.\n", index);
    }
}

void scribble(unsigned long *mem) {
    int index;
    printf("[*] Enter -1 to bestow power on the ancient memory.\n");
    printf("[+] Choose the warrior to empower (-1 to 2):");
    scanf("%d", &index);
    if (index >= 3 || index < -1) {
        printf("[!] Invalid choice.\n");
        return;
    }
    if (index == -1) {
        printf("[+] Share your power:");
        read(0, mem, 0x100);
        printf("[*] Power bestowed upon the ancient memory.\n");
    } else if (alloc_check[index] == 1) {
        printf("[+] Share your power:");
        read(0, alloc_array[index], alloc_size[index]+8);
        printf("[*] Warrior empowered with new strength.\n");
    } else {
        printf("[!] No warrior at index %d to empower.\n", index);
    }
}

void check() {
    unsigned long *ptr = malloc(0x30);

    if (ptr[4] == 0xdeadbeefcafebabe) {
        printf("[*] Glory to you! The gates of Valhalla open for you:\n");
        win();
    } else {
        printf("[!] You have failed to prove your worth.\n");
    }
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    unsigned long *mem = malloc(0xf00);

    printf("Welcome, warrior! The Gates of Valhalla await!\n");
    printf("Ancient memory is located at %p\n", mem);

    int choice, count = 0;
    while (count < 20) {
        menu();
        printf("[*] Choose your fate:");
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                alloc();
                count++;
                break;
            case 2:
                nuke();
                count++;
                break;
            case 3:
                scribble((unsigned long *)mem);
                count++;
                break;
            case 5:
                printf("[!] Retreating...\n");
                exit(0);
            case 4:
                check();
                exit(0);
            default:
                printf("[!] Invalid choice. Try again.\n");
                count++;
        }
    }

    return 0;
}
