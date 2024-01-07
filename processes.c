#include "processes.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <stdio.h>
#include "messages.h"
#include "file-properties.h"
#include "sync.h"
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

#include <signal.h>  

/*!
 * @brief prepare prepares (only when parallel is enabled) the processes used for the synchronization.
 * @param the_config is a pointer to the program configuration
 * @param p_context is a pointer to the program processes context
 * @return 0 if all went good, -1 else
 */
int prepare(configuration_t *the_config, process_context_t *p_context) {
    // Vérifier si le traitement parallèle est activé
    if (the_config->is_parallel) {
        // Allouer de la mémoire pour les PID des analyseurs
        p_context->source_analyzers_pids = malloc(sizeof(pid_t) * the_config->processes_count);
        p_context->destination_analyzers_pids = malloc(sizeof(pid_t) * the_config->processes_count);

        if (p_context->source_analyzers_pids == NULL || p_context->destination_analyzers_pids == NULL) {
            // Échec de l'allocation mémoire
            perror("Erreur d'allocation mémoire");
            return -1;
        }

        // Initialiser d'autres variables liées aux processus
        p_context->main_process_pid = getpid();
        p_context->source_lister_pid = 0;  // Vous pouvez définir ces valeurs selon vos besoins
        p_context->destination_lister_pid = 0;


        return 0;  // Renvoyer 0 pour indiquer le succès
    } else {
        // Le traitement parallèle n'est pas activé
        return 0;  // Renvoyer 0 pour indiquer le succès
    }
}

/*!
 * @brief make_process creates a process and returns its PID to the parent
 * @param p_context is a pointer to the processes context
 * @param func is the function executed by the new process
 * @param parameters is a pointer to the parameters of func
 * @return the PID of the child process (it never returns in the child process)
 */
int make_process(process_context_t *p_context, process_loop_t func, void *parameters) {
       pid_t child_pid = fork();

    if (child_pid < 0) {
        perror("Erreur lors de la création du processus");
        return -1;  // Échec de la création du processus
    } else if (child_pid == 0) {
        // Appeler la fonction spécifiée avec les paramètres
        func(parameters);

        // Terminer le processus enfant
        exit(EXIT_SUCCESS);
    } else {
        return child_pid;
    }
}

/*!
 * @brief lister_process_loop is the lister process function (@see make_process)
 * @param parameters is a pointer to its parameters, to be cast to a lister_configuration_t
 */
void lister_process_loop(void *parameters) {
    // Conversion du pointeur vers le type
    lister_configuration_t *lister_config = (lister_configuration_t *)parameters;
}

/*!
 * @brief analyzer_process_loop is the analyzer process function
 * @param parameters is a pointer to its parameters, to be cast to an analyzer_configuration_t
 */
void analyzer_process_loop(void *parameters) {

    // Conversion du pointeur vers le type
    analyzer_configuration_t *analyzer_config = (analyzer_configuration_t *)parameters;

}

/*!
 * @brief clean_processes cleans the processes by sending them a terminate command and waiting to the confirmation
 * @param the_config is a pointer to the program configuration
 * @param p_context is a pointer to the processes context
 */
void clean_processes(configuration_t *the_config, process_context_t *p_context) {
    // Ne rien faire si le traitement n'est pas parallèle
    if (!the_config->is_parallel) {
        return;
    }

    // Envoyer une commande de terminaison à tous les processus
    kill(p_context->main_process_pid, SIGTERM);
    kill(p_context->source_lister_pid, SIGTERM);
    kill(p_context->destination_lister_pid, SIGTERM);

    for (int i = 0; i < the_config->processes_count; ++i) {
        kill(p_context->source_analyzers_pids[i], SIGTERM);
        kill(p_context->destination_analyzers_pids[i], SIGTERM);
    }

    // Attendre les réponses de tous les processus fils
    int status;
    pid_t terminated_pid;

    while ((terminated_pid = wait(&status)) > 0) {
        printf("Processus avec PID %d terminé avec le statut %d\n", terminated_pid, WEXITSTATUS(status));
    }

    // Libérer la mémoire allouée
    free(p_context->source_analyzers_pids);
    free(p_context->destination_analyzers_pids);

    // Libérer la file de messages (MQ) ou d'autres ressources si nécessaire
}

void request_element_details(int msg_queue, files_list_entry_t *entry, lister_configuration_t *cfg, int *current_analyzers) {
    // Vérifier s'il y a des analyseurs disponibles
    if (*current_analyzers <= 0) {
        printf("Aucun analyseur disponible pour traiter la demande.\n");
        return;
    }

    // Sélectionner l'analyseur suivant
    int analyzer_id = cfg->my_recipient_id + cfg->my_receiver_id - (*current_analyzers);
    int recipient_id = cfg->my_recipient_id + analyzer_id;

    // Envoyer le message à la file de messages
    int result = msgsnd(msg_queue, entry, sizeof(files_list_entry_t) - sizeof(long), 0);

    if (result == -1) {
        // Gérer l'erreur, imprimer un message d'erreur avec perror
        perror("Erreur lors de l'envoi de la demande de détails");
    } else {
        printf("Demande de détails envoyée à l'analyseur %d avec succès.\n", analyzer_id);
        // Mettre à jour la liste des analyseurs disponibles
        (*current_analyzers)--;
 
       }   }
