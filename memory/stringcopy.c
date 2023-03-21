#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char *conf = malloc(strlen("test  asdf") + strlen("hallo"));
    sprintf(conf, "test %s asdf", "hallo");

    printf("%s\n", conf);
    return 0;
}
