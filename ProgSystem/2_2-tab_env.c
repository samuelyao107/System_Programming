/* 
 * Auteur(s): Samuel YAO
 */
#include <stdio.h>
#include <stdlib.h>

 int main(int argc, char **argv){
    const char* env_name = argv[1];

    char* env = getenv(env_name);

    if(env == NULL){
        fprintf(stderr, "Variable d'environnement non trouvée\n");
        return 1;
    }
    printf("%s\n", env);
    return 0;
 }