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
}

/*!
 * @brief compute_file_md5 computes a file's MD5 sum
 * @param the pointer to the files list entry
 * @return -1 in case of error, 0 else
 * Use libcrypto functions from openssl/evp.h
 */
int compute_file_md5(files_list_entry_t *entry) {
}

/*!
 * @brief directory_exists tests the existence of a directory
 * @path_to_dir a string with the path to the directory
 * @return true if directory exists, false else
 */
bool directory_exists(char *path_to_dir) {
    DIR *repertoire;
    repertoire=opendir(path_to_dir); 
    if (repertoire != NULL){
        printf("Le repertoire existe\n");
        return true;
    }else{
        printf("Le repertoire n'existe pas\n");
        return false;
    }
    closedir(repertoire);

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
