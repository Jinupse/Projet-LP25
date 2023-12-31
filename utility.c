#include "defines.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*!
 * @brief concat_path concatenates suffix to prefix into result
 * It checks if prefix ends by / and adds this token if necessary
 * It also checks that result will fit into PATH_SIZE length
 * @param result the result of the concatenation
 * @param prefix the first part of the resulting path
 * @param suffix the second part of the resulting path
 * @return a pointer to the resulting path, NULL when concatenation failed
 */

char *concat_path(char *result, char *prefix, char *suffix) {
        // Vérifier que les paramètres existent et que la taille est compatible
    if (result == NULL || prefix == NULL || (strlen(prefix)+strlen(suffix) >= PATH_SIZE)) {
        return NULL;
    }
   // Copier le préfixe dans le résultat
    strcpy(result, prefix);

    // Vérifier si le préfixe se termine par un '/'
    if (result[strlen(result) - 1] != '/') {
        // Ajouter un '/' si nécessaire
        if (strlen(result) < PATH_SIZE){
            strcat(result, "/");
        }
        else{
            return NULL;
        }
        }

    // Concaténer le suffixe au résultat
    strcat(result, suffix);

    // Vérifier la longueur du résultat
    if (strlen(result) >= PATH_SIZE) {
        return NULL; // Le résultat dépasse la taille maximale
    }

    return result;
}
