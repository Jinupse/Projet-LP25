#include <file-properties.h>

#include <sys/stat.h>
#include <dirent.h>
#include <openssl/evp.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <defines.h>
#include <fcntl.h>
#include <stdio.h>
#include <utility.h>

#include <sys/types.h> 
#include <sync.h>

/*!
 * @brief get_file_stats gets all of the required information for a file (inc. directories)
 * @param the files list entry
 * You must get:
 * - for files:
 *   - mode (permissions)
 *   - mtime (in nanoseconds)
 *   - size
 *   - entry type (FICHIER)
 *   - MD5 sum
 * - for directories:
 *   - mode
 *   - entry type (DOSSIER)
 * @return -1 in case of error, 0 else
 */
int get_file_stats(files_list_entry_t *entry) {

    struct stat infos;

    // Obtention des statistiques du fichier
    if (stat(entry->path_and_name, &infos) == -1) {
        // Erreur lors de l'obtention des statistiques du fichier
        return -1;
    }

    // Obtention des informations communes
    entry->mode = infos.st_mode;
    
    //verifie le type de fichier : ici si c'est un repertoire
    if (S_ISDIR(entry->entry_type)){ 
        entry->entry_type = DOSSIER;
        return 0;
    }else if (S_ISREG(entry->entry_type)){ //verifie le type de fichier : ici si c'est un fichier
        entry->mtime = infos.st_mtim.tv_nsec;//on recupere le temps en nanosecondes
        entry->size = infos.st_size;
        entry->entry_type = FICHIER;
        entry->md5sum = compute_file_md5(entry);//on recupere la somme md5
        return 0;
    }else{
        return -1;
    }
}

/*!
 * @brief compute_file_md5 computes a file's MD5 sum
 * @param the pointer to the files list entry
 * @return -1 in case of error, 0 else
 * Use libcrypto functions from openssl/evp.h
 */

int compute_file_md5(files_list_entry_t *entry) {
    // Ouvrir le fichier en mode binaire
    FILE *file = fopen(entry->path_and_name, "rb");

    // Vérifier si le fichier a pu être ouvert
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier");
        return -1;
    }
     MD5_CTX md5Context; //une structure qui contient les états internes nécessaires lors du calcul du hachage MD5.
    MD5_Init(&md5Context);// initialise le contexte MD5

    // Lire le fichier par blocs et mettre à jour le contexte MD5
    int bytesRead;
    char buffer[1024];
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) != 0) { // lit des données à partir d'un fichier
        MD5_Update(&md5Context, buffer, bytesRead);
    }

    // Finaliser la somme MD5
    MD5_Final(entry->md5sum, &md5Context);

    // Fermer le fichier
    fclose(file);
    return 0;
}

/*!
 * @brief directory_exists tests the existence of a directory
 * @path_to_dir a string with the path to the directory
 * @return true if directory exists, false else
 */
bool directory_exists(char *path_to_dir) {
    DIR *repertoire=opendir(path_to_dir); 
    if (repertoire != NULL){ //Le repertoire existe
        closedir(repertoire);
        return true;
    }else{ //Le repertoire n'existe pas
        return false;
    }
}

/*!
 * @brief is_directory_writable tests if a directory is writable
 * @param path_to_dir the path to the directory to test
 * @return true if dir is writable, false else
 * Hint: try to open a file in write mode in the target directory.
 */
bool is_directory_writable(char *path_to_dir) {
    DIR *repertoire=open_dir(path_to_dir); //on ouvre le répertoire
    FILE *f=fopen(abc,"w"); //on creer et on ouvre un fichier test nommé abc en mode ecriture
    if(f != NULL){//si le fichier a pu etre ouvert
        fclose(f);//on ferme le fichier
        remove(f);//on supprime le fichier
        return true;
    }else{
        return false;
    }
    closedir(repertoire);
}
