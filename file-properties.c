#include "file-properties.h"

#include <sys/stat.h>
#include <dirent.h>
#include <openssl/evp.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include "defines.h"
#include <fcntl.h>
#include <stdio.h>
#include "utility.h"

#include <sys/types.h> 
#include <sync.h>

#include <stdbool.h>

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
    // Ajout d'une vérification pour les fichiers vides
    FILE *file_check_empty = fopen(entry->path_and_name, "rb");
    if (file_check_empty != NULL) {
        fseek(file_check_empty, 0, SEEK_END);
        long size = ftell(file_check_empty);
        fclose(file_check_empty);

        if (size == 0) {
            printf("Le fichier est vide.\n");
            return 0;
        }
    }

    struct stat infos;

    // Obtention des statistiques du fichier
    if (stat(entry->path_and_name, &infos) == -1) {
        // Erreur lors de l'obtention des statistiques du fichier
        perror("Erreur lors de l'obtention des statistiques du fichier");
        fprintf(stderr, "Chemin du fichier : %s\n", entry->path_and_name);
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
    EVP_MD_CTX *md5Context;
    const EVP_MD *md5Algorithm;
    unsigned char md5Digest[EVP_MAX_MD_SIZE];
    unsigned int md5DigestLength;
    FILE *file;
    size_t bytesRead;
    unsigned char buffer[4096];

    // Ouvrir le fichier
    file = fopen(entry->path_and_name, "rb");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier");
        return;
    }

    // Initialiser le contexte MD5
    md5Context = EVP_MD_CTX_new();
    md5Algorithm = EVP_md5();

    if (EVP_DigestInit_ex(md5Context, md5Algorithm, NULL) != 1) {
        fprintf(stderr, "Erreur lors de l'initialisation du contexte MD5\n");
        fclose(file);
        EVP_MD_CTX_free(md5Context);
        return;
    }

    // Lire le fichier et mettre à jour le contexte de hachage
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (EVP_DigestUpdate(md5Context, buffer, bytesRead) != 1) {
            fprintf(stderr, "Erreur lors de la mise à jour du contexte MD5\n");
            fclose(file);
            EVP_MD_CTX_free(md5Context);
            return;
        }
    }

    // Finaliser le hachage
    if (EVP_DigestFinal_ex(md5Context, md5Digest, &md5DigestLength) != 1) {
        fprintf(stderr, "Erreur lors de la finalisation du contexte MD5\n");
        fclose(file);
        EVP_MD_CTX_free(md5Context);
        return;
    }

    // Copier le résultat dans entry->md5sum
    memcpy(entry->md5sum, md5Digest, md5DigestLength);

    // Libérer le contexte MD5
    EVP_MD_CTX_free(md5Context);

    // Fermer le fichier
    fclose(file);
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
    char testfile_path[PATH_MAX];
    snprintf(testfile_path, sizeof(testfile_path), "%s/testfile", path_to_dir);

    FILE *f = fopen(testfile_path, "w");
    if (f != NULL) {
        fclose(f);
        remove(testfile_path);
        return true;
    } else {
        return false;
    }
}

