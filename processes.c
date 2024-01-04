#include "processes.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <stdio.h>
#include <messages.h>
#include <file-properties.h>
#include <sync.h>
#include <string.h>
#include <errno.h>

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
    // Conversion du pointeur vers le type approprié
    lister_configuration_t *lister_config = (lister_configuration_t *)parameters;
    printf("Boucle du processus lister en cours d'exécution avec recipient_id : %d, receiver_id : %d\n",
           lister_config->my_recipient_id, lister_config->my_receiver_id);
}

/*!
 * @brief analyzer_process_loop is the analyzer process function
 * @param parameters is a pointer to its parameters, to be cast to an analyzer_configuration_t
 */
void analyzer_process_loop(void *parameters) {
}

/*!
 * @brief clean_processes cleans the processes by sending them a terminate command and waiting to the confirmation
 * @param the_config is a pointer to the program configuration
 * @param p_context is a pointer to the processes context
 */
void clean_processes(configuration_t *the_config, process_context_t *p_context) {
    // Do nothing if not parallel
    // Send terminate
    // Wait for responses
    // Free allocated memory
    // Free the MQ
}
