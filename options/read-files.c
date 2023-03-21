/*
https://www.programiz.com/c-programming/c-file-input-output
https://www.guru99.com/c-file-input-output.html
https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c
*/

#include <stdio.h>
#include <stdlib.h>
int main()
{
    char ch, file_name[25];
    FILE *fp;

    printf("Enter name of a file you wish to see: ");
    scanf("%s", file_name);

    printf(" - open: %s\n", file_name);

    fp = fopen(file_name, "r"); // read mode

    if (fp == NULL)
    {
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }

    fseek(fp, 0L, SEEK_END);

    // calculating the size of the file
    long int res = ftell(fp);
    printf("Size of the file is %ld bytes \n", res);

    rewind(fp);

    printf("The contents of %s file are:\n", file_name);

    while ((ch = fgetc(fp)) != EOF)
        printf("%c", ch);

    fclose(fp);
    return 0;
}
