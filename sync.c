#include <sync.h>
#include <dirent.h>
#include <string.h>
#include <processes.h>
#include <utility.h>
#include <messages.h>
#include <file-properties.h>
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
  if (lhd->mtime == rhd->mtime){
      if(lhd->size == rhd->size){
          if(lhd->entry_type == rhd->entry_type){
              if(lhd->mode == rhd->mode){
                  return false;
              }
          }
      } 
  }
  
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
    while ((entry = readdir(dir)) != NULL) {
        // Ignore les entrées "." et ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // On construit le chemin complet en combinant target_path et le nom de l'entrée
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", target_path, entry->d_name);

        // On ajoute les informations du fichier à la liste
        files_list_entry_t *added_entry = add_file_entry(list, full_path);
        if (added_entry == NULL) {
            fprintf(stderr, "Erreur lors de l'ajout du fichier : %s\n", full_path);
        }

        // Si l'entrée est un répertoire, on effectue un appel récursif pour traiter son contenu
        if (entry->d_type == DOSSIER) {
            make_files_list(list, entry->d_name);
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
}

/*!
 * @brief copy_entry_to_destination copies a file from the source to the destination
 * It keeps access modes and mtime (@see utimensat)
 * Pay attention to the path so that the prefixes are not repeated from the source to the destination
 * Use sendfile to copy the file, mkdir to create the directory
 */
void copy_entry_to_destination(files_list_entry_t *source_entry, configuration_t *the_config) {  

    FILE *f_source=fopen(source_entry->path_and_name,"r");//ouverture du fichier source en mode lecture
    DIR *destination=opendir(the_config->destination);//ouverture du repertoire de destination

    if(f_source != NULL){
        if(directory_exists(destination) == false){//on verifie si le repertoire de destination existe 
            mkdir(the_config->destination,source_entry->mode);//s'il n'existe pas on le creer en concervant les modes d'acces
        }

        if(!is_directory_writable(the_config->destination)){ //si le repertoire de destination n'est pas ouvert en mode ecriture on ne peut pas copier le fichier  
            perror("le fichier ne peut pas etre copie car le repertoire de destination ne possede pas les droits en ecriture");
        }else{
            //permet que les préfixes ne soient pas répétés de la source à la destination dans le chemin
            concat_path(the_config->destination,the_config->destination,basename(the_config->source));//on ajoute le nom du fichier au chemin de la destination
            FILE *f_destination=fopen(the_config->destination,"w");//creation d'un fichier dans le repertoire de destination
            sendfile(f_destination,f_source,0,source_entry->size);//copie du fichier vers le repertoire de destination
            utimensat (f_source,f_destination,source_entry->mtime,0);//on concerve le mtime 
        }

    }else{
        perror("le fichier n'a pas pu etre ouvert");
    }

    fclose(f_source);
    fclose(f_destination);
    closedir(destination);
}

/*!
 * @brief make_list lists files in a location (it recurses in directories)
 * It doesn't get files properties, only a list of paths
 * This function is used by make_files_list and make_files_list_parallel
 * @param list is a pointer to the list that will be built
 * @param target is the target dir whose content must be listed
 */
void make_list(files_list_t *list, char *target) {
  #include <dirent.h>

/*!
 * @brief make_list lists files in a location (it recurses in directories)
 * It doesn't get files properties, only a list of paths
 * This function is used by make_files_list and make_files_list_parallel
 * @param list is a pointer to the list that will be built
 * @param target is the target dir whose content must be listed
 */
void make_list(files_list_t *list, const char *target) {
    // Ouvre le répertoire spécifié
    DIR *dir = opendir(target);
    if (dir == NULL) {
        perror("Erreur en ouvrant le répertoire");
        return;
    }

    // Structure pour stocker les informations sur les entrées du répertoire
    struct dirent *entry;

    // Parcours de chaque entrée du répertoire
    while ((entry = readdir(dir)) != NULL) {
        // Ignorer les entrées "." et ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Construire le chemin complet de l'entrée
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", target, entry->d_name);

        // Ajouter l'entrée à la liste
        files_list_entry_t *added_entry = add_file_entry(list, full_path);
        if (added_entry == NULL) {
            fprintf(stderr, "Erreur lors de l'ajout du fichier : %s\n", full_path);
        }

        // Si l'entrée est un répertoire, récursivement appeler la fonction pour explorer son contenu
        if (entry->d_type == DT_DIR) {
            make_list(list, full_path);
        }
    }

    // Fermer le répertoire
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
