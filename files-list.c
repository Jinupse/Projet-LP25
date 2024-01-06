#include "file-list.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <stdio.h>

/*!
 * @brief clear_files_list clears a files list
 * @param list is a pointer to the list to be cleared
 * This function is provided, you don't need to implement nor modify it
 */
void clear_files_list(files_list_t *list) {
    while (list->head) {
        files_list_entry_t *tmp = list->head;
        list->head = tmp->next;
        free(tmp);
    }
}

/*!
 *  @brief add_file_entry adds a new file to the files list.
 *  It adds the file in an ordered manner (strcmp) and fills its properties
 *  by calling stat on the file.
 *  Il the file already exists, it does nothing and returns 0
 *  @param list the list to add the file entry into
 *  @param file_path the full path (from the root of the considered tree) of the file
 *  @return 0 if success, -1 else (out of memory)
 */
files_list_entry_t *add_file_entry(files_list_t *list, char *file_path) {
    struct stat infos;
    if (stat(file_path, &infos) != 0) {
        perror("Erreur lors de la récupération des informations sur le fichier");
        return NULL;
    }

    files_list_entry_t *newf = malloc(sizeof(files_list_entry_t));
    if (newf == NULL) {
        perror("Échec de l'allocation mémoire");
        return NULL;
    }

    strncpy(newf->path_and_name, file_path, sizeof(newf->path_and_name) - 1);
    newf->path_and_name[sizeof(newf->path_and_name) - 1] = '\0';

    newf->mtime.tv_sec = infos.st_mtime;
    newf->mtime.tv_nsec = 0;

    newf->size = infos.st_size;

    if (S_ISDIR(infos.st_mode)) {
        newf->entry_type = DOSSIER;
    } else if (S_ISREG(infos.st_mode)) {
        newf->entry_type = FICHIER;
    }

    newf->mode = infos.st_mode;

    newf->next = NULL;
    newf->prev = NULL;

    if (list->head == NULL) {
        list->head = newf;
        list->tail = newf;
    } else {
        newf->prev = list->tail;
        list->tail->next = newf;
        list->tail = newf;
    }

    return newf;
}

/*!
 * @brief add_entry_to_tail adds an entry directly to the tail of the list
 * It supposes that the entries are provided already ordered, e.g. when a lister process sends its list's
 * elements to the main process.
 * @param list is a pointer to the list to which to add the element
 * @param entry is a pointer to the entry to add. The list becomes owner of the entry.
 * @return 0 in case of success, -1 else
 */
int add_entry_to_tail(files_list_t *list, files_list_entry_t *entry) {
    list->tail->next=entry;
    entry->prev=list->tail;
    list->tail=entry;
}

/*!
 *  @brief find_entry_by_name looks up for a file in a list
 *  The function uses the ordering of the entries to interrupt its search
 *  @param list the list to look into
 *  @param file_path the full path of the file to look for
 *  @param start_of_src the position of the name of the file in the source directory (removing the source path)
 *  @param start_of_dest the position of the name of the file in the destination dir (removing the dest path)
 *  @return a pointer to the element found, NULL if none were found.
 */
files_list_entry_t *find_entry_by_name(files_list_t *list, char *file_path, size_t start_of_src, size_t start_of_dest) {
   
                                                      //current est un pointeur qui représente l'état actuel pendant l'itération dans la liste
                                                     //la boucle for permet d'itérer à travers la liste avec current qui commence au début de la liste
    for (files_list_entry_t *current = list->head; current != NULL; current = current->next) {
                                                     //on compare le chemin complet du fichier (file_path) avec le chemin complet du fichier stocké dans l'élément actuel
                                                    // Pour commencer la comparaison au bon endroit on utilise star_of_src
        if (strcmp(file_path, current->path_and_name + start_of_src) == 0) {
                                                    //si un fichier est trouvé la fonction renvoie le pointeur vers l'élément précis dans la liste
            return current;
        }
    }
    return NULL;
}

/*!
 * @brief display_files_list displays a files list
 * @param list is the pointer to the list to be displayed
 * This function is already provided complete.
 */
void display_files_list(files_list_t *list) {
    if (!list)
        return;
    
    for (files_list_entry_t *cursor=list->head; cursor!=NULL; cursor=cursor->next) {
        printf("%s\n", cursor->path_and_name);
    }
}

/*!
 * @brief display_files_list_reversed displays a files list from the end to the beginning
 * @param list is the pointer to the list to be displayed
 * This function is already provided complete.
 */
void display_files_list_reversed(files_list_t *list) {
    if (!list)
        return;
    
    for (files_list_entry_t *cursor=list->tail; cursor!=NULL; cursor=cursor->prev) {
        printf("%s\n", cursor->path_and_name);
    }
}
