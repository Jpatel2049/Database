#ifndef MYFILE_H_
#define MYFILE_H_

#define SIZE 9
#define REMOVED -1

/* Keeps track of the number of roles. */
int num_roles;
/* Keeps track of the number of resources. */
int num_resources;
/* Keeps track of the number of matches found in the table. */
int total_found;

char * tuid;
char * access_net;
char * full_name;
char * preferred_name;
char ** role_ids;
int * resource_ids;
int * final_resource_ids;
char ** resource_names;

int is_valid (char *);
char * get_tuid(char *);
char * get_access_net ();
char * get_full_name ();
char * get_prefer_name ();
char ** get_role_ids ();
int * get_resource_ids();
int * remove_extras();
char ** get_resource_names();
void write_to_file();
void free_memory();

#endif
