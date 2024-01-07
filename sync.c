#include "sync.h"
#include <dirent.h>
#include <string.h>
#include "processes.h"
#include "utility.h"
#include "messages.h"
#include "file-properties.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <sys/msg.h>
#include <stdbool.h>

#include <stdio.h>

/*!
 * @brief synchronize is the main function for synchronization
 * It will build the lists (source and destination), then make a third list with differences, and apply differences to the destination
 * It must adapt to the parallel or not operation of the program.
 * @param the_config is a pointer to the configuration
 * @param p_context is a pointer to the processes context
 */
void synchronize(configuration_t *the_config, process_context_t *p_context) {
    files_list_t liste_source;                                              // On crée les 2 listes de fichiers contenus dans sources et localisation
    files_list_t liste_dest;
    make_files_list(liste_source,the_config->source);                       // On Remplit les 2 listes
    make_files_list(liste_dest,the_config->destination);
    files_list_entry_t *taille_d = malloc(sizeof(files_list_entry_t));      // On cherche la taille de la liste destination pour la suite du programme
    taille_d=liste_dest->head;
    int taille_ld=1;
    while(taille_d!=liste_d->tail){
        taille_d=taille_d->next;
        taille_ld+=1;
    }
    files_list_entry_t *tmp_s = malloc(sizeof(files_list_entry_t));         // tmp_s et tmp_d vont nous servir à parcourir les 2 listes sources et destination
    files_list_entry_t *tmp_d = malloc(sizeof(files_list_entry_t));
    files_list_t differences;
    tmp_s=liste_source->head;
    int compteur=0;
    while (tmp_s){                                                          // Pour chaque élément de la liste source on parcourt tous les éléments de la liste destination et si
        tmp_d=liste_dest->head;                                             // il n'y a aucune correspondance quand on a comparé le ficheir de la source aux fichiers de la
        while(tmp_d){                                                       // destination, on rajoute le fichier de la source dans la liste de differences
            if (mismatch(tmp_s,tmp_d,the_config->uses_md5)==False){
                compteur+=1
            }
            tmp_d->next;
        }
        if (compteur==(taille_d)){
            add_file_entry(differences,tmp_s->path_and_name);
        }
        tmp_s=tmp_s->next;
    }
    files_list_entry_t *tmp_diff = malloc(sizeof(files_list_entry_t));      // Ensuite on parcourt la liste de différences et on copie les fichiers dans la destination
    tmp_diff=differences->head;
    while (tmp_diff){
        copy_entry_to_destination(tmp_diff,the_config);
        tmp_diff=tmp_diff->next;
    }
}

/*!
 * @brief mismatch tests if two files with the same name (one in source, one in destination) are equal
 * @param lhd a files list entry from the source
 * @param rhd a files list entry from the destination
 * @has_md5 a value to enable or disable MD5 sum check
 * @return true if both files are not equal, false else
 */
bool mismatch(files_list_entry_t *lhd, files_list_entry_t *rhd, bool has_md5) {

    if(has_md5 == 1){ //active la vérification de md5
        for (int i=0;i<16;i++){
            if (lhd->md5sum[i] != rhd->md5sum[i]){
                return true;
            } 
        }
    }
    if (lhd->mtime.tv_sec != rhd->mtime.tv_sec || lhd->mtime.tv_nsec != rhd->mtime.tv_nsec){
        return true;
    }
    if(lhd->size != rhd->size){
        return true;
    }    
    if(lhd->entry_type != rhd->entry_type){
        return true;
    }
    if(lhd->mode != rhd->mode){
        return true;
    }
    return false;
}

/*!
 * @brief make_files_list buils a files list in no parallel mode
 * @param list is a pointer to the list that will be built
 * @param target_path is the path whose files to list
 */
void make_files_list(files_list_t *list, char *target_path) {
    // Ouvre le répertoire
    DIR *dir = opendir(target_path);
    if (dir == NULL) {
        perror("Erreur en ouvrant le répertoire");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;

    // se realise sur chaque entrée du répertoire
    while ((entry = get_next_entry(dir)) != NULL) {
        // On construit le chemin complet en combinant target_path et le nom de l'entrée
        char full_path[PATH_MAX];
        concat_path(full_path,target_path, entry->d_name);

        // On ajoute les informations du fichier à la liste
        files_list_entry_t *added_entry = add_file_entry(list, full_path);
        if (added_entry == NULL) {
            fprintf(stderr, "Erreur lors de l'ajout du fichier : %s\n", full_path);
        }

        // Si l'entrée est un répertoire, on effectue un appel récursif pour traiter son contenu
         if (S_ISDIR(entry->d_type)) {
            make_files_list(list, full_path);
        }

        // On affiche des informations sur le fichier actuel
        printf("Fichier : %s\n", full_path);
    }

    // On ferme le répertoire
    closedir(dir);
}

/*!
 * @brief make_files_lists_parallel makes both (src and dest) files list with parallel processing
 * @param src_list is a pointer to the source list to build
 * @param dst_list is a pointer to the destination list to build
 * @param the_config is a pointer to the program configuration
 * @param msg_queue is the id of the MQ used for communication
 */
void make_files_lists_parallel(files_list_t *src_list, files_list_t *dst_list, configuration_t *the_config, int msg_queue) {
     // Vérifier si le traitement parallèle est activé
    if (!the_config->is_parallel) {
        return;
    }

    // Créer le processus source_lister
    pid_t source_lister_pid = make_process(the_config, lister_process_loop, src_list);

    // Créer le processus destination_lister
    pid_t destination_lister_pid = make_process(the_config, lister_process_loop, dst_list);

    // Créer les processus source_analyzers et destination_analyzers en parallèle
    for (int i = 0; i < the_config->processes_count; ++i) {
        make_process(the_config, analyzer_process_loop, &src_list->analyzer_config[i]);
        make_process(the_config, analyzer_process_loop, &dst_list->analyzer_config[i]);
    }

    // Attendre la fin du processus source_lister
    waitpid(source_lister_pid, NULL, 0);

    // Attendre la fin du processus destination_lister
    waitpid(destination_lister_pid, NULL, 0);

    // Attendre la fin de tous les processus source_analyzers et destination_analyzers
    for (int i = 0; i < the_config->processes_count; ++i) {
        wait(NULL);
    }
}

/*!
 * @brief copy_entry_to_destination copies a file from the source to the destination
 * It keeps access modes and mtime (@see utimensat)
 * Pay attention to the path so that the prefixes are not repeated from the source to the destination
 * Use sendfile to copy the file, mkdir to create the directory
 */
void copy_entry_to_destination(files_list_entry_t *source_entry, configuration_t *the_config) {
    char chemin_fichier[PATH_SIZE];
    //supprime le prefixe de file_list_entry
    if (S_ISREG(source_entry->mode)) {
        concat_path(chemin_fichier, the_config->destination, source_entry->path_and_name + strlen(the_config->source)+1);
        if(the_config->verbose) {
            printf("Copie %s dans %s\n", chemin_fichier, the_config->destination);
        }
        int source_fd = open(source_entry->path_and_name, O_RDONLY);
        if (source_fd == -1) {
            perror("Erreur lors de l'ouverture du fichier source\n");
            return;
        }
        //Creation d'un dossier intermediaire
        char *separateur="/";
        char *token = strtok(source_entry->path_and_name + strlen(the_config->source)+1,separateur);
        char chemin_repertoire[256] = "";
        strcat(chemin_repertoire,the_config->destination);
        while(strcmp(chemin_repertoire,chemin_fichier) != 0){
            strcat(chemin_repertoire,separateur);
            strcat(chemin_repertoire,token);
            if(strcmp(chemin_repertoire,chemin_fichier)!=0) {
                if(mkdir(chemin_repertoire,0777) != 0) {
                    if(the_config->verbose) {
                        printf("Erreur \n");
                    }
                    perror("Le chemin ne peut pas être ouvert\n");
                    return;
                }
            }
            token = strtok(NULL,separateur);
        }
        int destination_fd = open(chemin_fichier, O_WRONLY | O_CREAT | O_TRUNC, source_entry->mode);
        if (destination_fd == -1) {
            if(the_config->verbose) {
                printf("Erreur\n");
            }
            perror("Erreur lors de l'ouverture du fichier de destination\n");
            return;
        }
    }
}

/*!
 * @brief make_list lists files in a location (it recurses in directories)
 * It doesn't get files properties, only a list of paths
 * This function is used by make_files_list and make_files_list_parallel
 * @param list is a pointer to the list that will be built
 * @param target is the target dir whose content must be listed
 */
void make_list(files_list_t *list, const char *target) {
    // Ouvre le répertoire
    DIR *dir = opendir(target_path);
    if (dir == NULL) {
        perror("Erreur en ouvrant le répertoire");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;

    // se realise sur chaque entrée du répertoire
    while ((entry = get_next_entry(dir)) != NULL) {
        // On construit le chemin complet en combinant target_path et le nom de l'entrée
        char full_path[PATH_MAX];
        concat_path(full_path,target_path, entry->d_name);

        // On ajoute les informations du fichier à la liste
        files_list_entry_t *added_entry = add_file_entry(list, full_path);
        if (added_entry == NULL) {
            fprintf(stderr, "Erreur lors de l'ajout du fichier : %s\n", full_path);
        }

        // Si l'entrée est un répertoire, on effectue un appel récursif pour traiter son contenu
         if (S_ISDIR(entry->d_type)) {
            make_files_list(list, full_path);
        }

        // On affiche des informations sur le fichier actuel
        printf("Fichier : %s\n", full_path);
    }

    // On ferme le répertoire
    closedir(dir);
}
}

/*!
 * @brief open_dir opens a dir
 * @param path is the path to the dir
 * @return a pointer to a dir, NULL if it cannot be opened
 */
DIR *open_dir(char *path) {
                            // ouverture du répertoire dont le chemin est path et stockage de la valeur qui pointe vers le répertoire ouvert rep
    DIR *rep = opendir(path);

                            // vérification de l'ouverture du répertoire
    if (rep == NULL) {
                            // Affichage de la variable errno qui contient l'erreur
        perror("Impossible d'ouvrir le répertoire, vérifiez le nom.");
    }
                            // on retourne la valeur qui est soit NULL soit pointe vers le répertoire qui a été ouvert
    return rep;
}

/*!
 * @brief get_next_entry returns the next entry in an already opened dir
 * @param dir is a pointer to the dir (as a result of opendir, @see open_dir)
 * @return a struct dirent pointer to the next relevant entry, NULL if none found (use it to stop iterating)
 * Relevant entries are all regular files and dir, except . and ..
 */
struct dirent *get_next_entry(DIR *dir) {
    struct dirent *next_entry;
    next_entry= readdir(dir);
    if (!next_entry){
        return NULL;
    }else{
        if (S_ISDIR(next_entry->d_type) || S_ISREG(next_entry->d_type)){
            return next_entry;
        }
        return NULL;
    }
}
