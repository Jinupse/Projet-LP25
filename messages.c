#include "messages.h"
#include <sys/msg.h>
#include <string.h>
#include <errno.h> 
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>


// Functions in this file are required for inter processes communication

/*!
 * @brief send_file_entry sends a file entry, with a given command code
 * @param msg_queue the MQ identifier through which to send the entry
 * @param recipient is the id of the recipient (as specified by mtype)
 * @param file_entry is a pointer to the entry to send (must be copied)
 * @param cmd_code is the cmd code to process the entry.
 * @return the result of the msgsnd function
 * Used by the specialized functions send_analyze*
 */
int send_file_entry(int msg_queue, int recipient, files_list_entry_t *file_entry, int cmd_code) {
    if (file_entry == NULL) {
        fprintf(stderr, "Erreur, entrée invalide.\n");
        return -1;
    }

    files_list_entry_transmit_t msg;
    msg.mtype = recipient;
    msg.op_code = cmd_code; 
    memcpy(&msg.payload, file_entry, sizeof(files_list_entry_t));

    int result_msg = msgsnd(msg_queue, &msg, sizeof(files_list_entry_transmit_t) - sizeof(long), 0);
    
    if (result_msg == -1) {
        perror("msgsnd");
    }

    return result_msg;
}

    //Création d'un message avec sa structure (type de message, code d'opération et données du fichier)
    files_list_entry_transmit_t msg;
    msg.mtype = recipient;
    msg.op_code = cmd_code;
   //permet de copier les données contenues dans la structure file_entry vers le champ payload de la structure message afin
   //que les informations sur le messages soit transmises 
    memcpy(&msg.payload, file_entry, sizeof(files_list_entry_t));

    // Appel de la fonction msgsnd pour envoyer notre message à une file de messages.
   //msg_queue est l'identifiant de la file de message
    int result_msg = msgsnd(msg_queue, &msg, sizeof(files_list_entry_transmit_t), 0);
   //si la fonction renvoie -1 il y a une erreur
    if (result_msg == -1) {
        perror("msgsnd");
    }

    return result_msg;
}

/*!
 * @brief send_analyze_dir_command sends a command to analyze a directory
 * @param msg_queue is the id of the MQ used to send the command
 * @param recipient is the recipient of the message (mtype)
 * @param target_dir is a string containing the path to the directory to analyze
 * @return the result of msgsnd
 */

 int send_analyze_dir_command(int msg_queue, int recipient, char *target_dir) {
    if (target_dir == NULL) {
        fprintf(stderr, "Erreur : pointeur de chaîne NULL\n");
        return -1;
    }

    // Initialiser la structure de la commande d'analyse de répertoire
    analyze_dir_command_t dir_command;
    dir_command.mtype = recipient;
    dir_command.op_code = COMMAND_CODE_ANALYZE_DIR;
    strncpy(dir_command.target, target_dir, sizeof(dir_command.target));

    // Initialiser la structure du message à envoyer
    any_message_t message;
    message.analyze_dir_command = dir_command;

    // Envoyer le message à la file de messages
    int result = msgsnd(msg_queue, &message, sizeof(analyze_dir_command_t) - sizeof(long), 0);

    if (result == -1) {
        // Gérer l'erreur, imprimer un message d'erreur avec errno
        perror("Erreur lors de l'envoi de la commande d'analyse du répertoire");
    }

    return result;
}


// The 3 following functions are one-liners

/*!
 * @brief send_analyze_file_command sends a file entry to be analyzed
 * @param msg_queue the MQ identifier through which to send the entry
 * @param recipient is the id of the recipient (as specified by mtype)
 * @param file_entry is a pointer to the entry to send (must be copied)
 * @return the result of the send_file_entry function
 * Calls send_file_entry function
 */
int send_analyze_file_command(int msg_queue, int recipient, files_list_entry_t *file_entry) {
    char cmd_code = 'A';  // Utiliser 'A' comme code d'opération pour l'analyse de fichier
    return send_file_entry(msg_queue, recipient, file_entry, cmd_code);
}


/*!
 * @brief send_analyze_file_response sends a file entry after analyze
 * @param msg_queue the MQ identifier through which to send the entry
 * @param recipient is the id of the recipient (as specified by mtype)
 * @param file_entry is a pointer to the entry to send (must be copied)
 * @return the result of the send_file_entry function
 * Calls send_file_entry function
 */
int send_analyze_file_response(int msg_queue, int recipient, files_list_entry_t *file_entry) {
    char cmd_code = 'B';  // Utiliser 'B' comme code d'opération pour la réponse de l'analyse de fichier
    return send_file_entry(msg_queue, recipient, file_entry, cmd_code);
}
/*!
 * @brief send_files_list_element sends a files list entry from a complete files list
 * @param msg_queue the MQ identifier through which to send the entry
 * @param recipient is the id of the recipient (as specified by mtype)
 * @param file_entry is a pointer to the entry to send (must be copied)
 * @return the result of the send_file_entry function
 * Calls send_file_entry function
 */
int send_files_list_element(int msg_queue, int recipient, files_list_entry_t *file_entry) {
    char cmd_code = 'C';  // Utiliser 'C' comme code d'opération pour l'élément de liste
    return send_file_entry(msg_queue, recipient, file_entry, cmd_code);
}

/*!
 * @brief send_list_end sends the end of list message to the main process
 * @param msg_queue is the id of the MQ used to send the message
 * @param recipient is the destination of the message
 * @return the result of msgsnd
 */
int send_list_end(int msg_queue, int recipient) {

    // Création d'une structure pour le message de fin de liste
    any_message_t message_fin;
    message_fin.list_entry.mtype = recipient;
    message_fin.list_entry.op_code = COMMAND_CODE_LIST_COMPLETE; 

    // Envoyer le message à la file de messages
    int resultat = msgsnd(msg_queue, &message_fin, sizeof(any_message_t) - sizeof(long), 0);

    if (resultat == -1) {
        // Gérer l'erreur, imprimer un message d'erreur avec perror
        perror("Erreur lors de l'envoi du message de fin de liste");
    }

    return resultat;
}

/*!
 * @brief send_terminate_command sends a terminate command to a child process so it stops
 * @param msg_queue is the MQ id used to send the command
 * @param recipient is the target of the terminate command
 * @return the result of msgsnd
 */
int send_terminate_command(int msg_queue, int recipient) {
    simple_command_t message_terminer;
    message_terminer.mtype = recipient;
    message_terminer.message = 'D';  // Utiliser 'D' comme code d'opération pour la commande de terminaison

    int resultat = msgsnd(msg_queue, &message_terminer, sizeof(simple_command_t) - sizeof(long), 0);

    if (resultat == -1) {
        perror("Erreur lors de l'envoi de la commande de terminaison");
    }

    return resultat;
}

/*!
 * @brief send_terminate_confirm sends a terminate confirmation from a child process to the requesting parent.
 * @param msg_queue is the id of the MQ used to send the message
 * @param recipient is the destination of the message
 * @return the result of msgsnd
 */
int send_terminate_confirm(int msg_queue, int recipient) {
    simple_command_t message_confirmation;
    message_confirmation.mtype = recipient;
    message_confirmation.message = 'E';  // Utiliser 'E' comme code d'opération pour la confirmation de terminaison

    int resultat = msgsnd(msg_queue, &message_confirmation, sizeof(simple_command_t) - sizeof(long), 0);

    if (resultat == -1) {
        perror("Erreur lors de l'envoi de confirmation");
    }

    return resultat;
}
