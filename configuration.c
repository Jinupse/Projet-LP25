#include <configuration.h>
#include <stddef.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

typedef enum {DATE_SIZE_ONLY, NO_PARALLEL} long_opt_values;

/*!
 * @brief function display_help displays a brief manual for the program usage
 * @param my_name is the name of the binary file
 * This function is provided with its code, you don't have to implement nor modify it.
 */
void display_help(char *my_name) {
    printf("%s [options] source_dir destination_dir\n", my_name);
    printf("Options: \t-n <processes count>\tnumber of processes for file calculations\n");
    printf("         \t-h display help (this text)\n");
    printf("         \t--date_size_only disables MD5 calculation for files\n");
    printf("         \t--no-parallel disables parallel computing (cancels values of option -n)\n");
    printf("         \t--dry-run lists the changes that would need to be synchronized but doesn't perform them\n");
    printf("         \t-v enables verbose mode\n");
}

/*!
 * @brief init_configuration initializes the configuration with default values
 * @param the_config is a pointer to the configuration to be initialized
 */
void init_configuration(configuration_t *the_config) 
{
    // affectation des paramètres par défaut
    strcpy(the_config->source, "source.txt");
    strcpy(the_config->destination, "destination.txt");
    the_config->processes_count = 1;
    the_config->is_parallel = 0;
    the_config->uses_md5 = 0;
    the_config->is_verbose=0;
    the_config->is_dry_run=0;
}

/*!
 * @brief set_configuration updates a configuration based on options and parameters passed to the program CLI
 * @param the_config is a pointer to the configuration to update
 * @param argc is the number of arguments to be processed
 * @param argv is an array of strings with the program parameters
 * @return -1 if configuration cannot succeed, 0 when ok
 */
int set_configuration(configuration_t *the_config, int argc, char *argv[]) struct option my_opts[] = {
        {.name="processes-count", .has_arg=1, .flag=0, .val='p'},
        {.name="is_parallel", .has_arg=0, .flag=0, .val='i'},
        {.name="uses_md5", .has_arg=0, .flag=0, .val='m'},
        {.name="is_verbose", .has_arg=0, .flag=0, .val='v'},
        {.name="is_dry_run", .has_arg=0, .flag=0, .val='r'},
        
        {.name=0, .has_arg=0, .flag=0, .val=0},
    };
int opt;
    while ((opt = getopt_long(argc, argv, "", my_opts, NULL)) != -1) {
        switch (opt) {
            case 'p':
                // Update processes count in the configuration
                the_config->processes_count = atoi(optarg);
                break;

            case 'i':
                // Update is_parallel flag in the configuration
                the_config->is_parallel = 1;
                break;

            case 'm':
                // Update uses_md5 flag in the configuration
                the_config->uses_md5 = 1;
                break;
            case 'v' :
                // Update is_verbose flag in the configuration
                the_config->is_verbose = 1;
                break;
            case 'r' : 
             // Update is_dry_run flag in the configuration
                the_config->is_dry_run = 1;
                break;

            default:
                // Handle unknown or invalid options
                return -1;
        }
    }
