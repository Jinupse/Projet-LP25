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

    FILE *f=fopen("information.txt","a");//creation d'un fichier pour stocker les informations
    
    if(f != NULL){

        if (S_ISDIR(entry->entry_type)){ //verifie le type de fichier ici si c'est un repertoire
            fprintf(f,"%lo#",(unsigned long)entry->mode);
            fprintf(f,"%s\n",entry->entry_type);
            return 0;
        }else if (S_ISREG(entry->entry_type)){ //verifie le type de fichier ici si c'est un fichier
            fprintf(f,"%lo#",(unsigned long)entry->mode);
            fprintf(f,"%s#",entry->mtime);
            fprintf(f,"%lld#",entry->size);
            fprintf(f,"%s#",entry->entry_type);
            for (int i=0;i<16;i++){
                fprintf(f,"%d",entry->md5sum[i]);
            }
            fprintf(f,"\n");
            return 0;
        }else{
            return -1;
        }
    } 
    
    fclose(f);
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

    DIR *repertoire;
    struct dirent *rep;
    struct stat statistique;

    if ((directory_exists(path_to_dir)) == true){ 
        
        repertoire=opendir(path_to_dir); 
        rep=readdir(repertoire);

        while(rep) { 
            
            if(stat(rep->d_name, &statistique) != 0){ //permet d'obtenir les informations sur le fichier
                return false;
            }else{
                FILE *f=fopen(rep->d_name,"w");
                if(S_ISREG(statistique.st_mode) && f != NULL){ //on regarde si c'est un fichier regulier
                    return true;
                }
            }
            rep = readdir(repertoire);
        }
            
    }else{
        return false; 
    }
    
    closedir(repertoire);
}
