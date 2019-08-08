#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "my_file.h"
#include "db_helper.h"

/* Loops through the given tuid and checks if
it is the correct length and contains only
letters and numbers.  */
int is_valid(char * input) {
  int count;
  for(count = 0; *input != '\0'; input++) {
    /* Increases count if letter or number. */
    if(isdigit(*input) || isalpha(*input)) {
      count++;
    } else {
      return 0;
    }
  }
  /* Checks if tuid is correct length. */
  if (count != SIZE) {
    return 0;
  }
  return 1;
}

/* Allocates space for tuid. */
char * get_tuid(char * input) {
  /* Allocate space for output. */
  char * output = malloc(sizeof(char *) * (SIZE + 1));
  if(output == NULL) {
    fprintf(stderr, "WARNING: Unable to allocate memory for tuid, returning NULL.\n");
    return NULL;
  }
  strcpy(output,input);
  return output;
}

/* Gets a user's accessnet from their tuid. */
char * get_access_net() {
  /* Accessnet. */
  char ** val = getValue("person_ids", tuid, 0, 1);
  if(val != NULL) {
    /* Allocate space for output. */
    char * output = malloc(((strlen(val[0])) + 1) * sizeof(char *));
    if(output == NULL) {
      fprintf(stderr, "WARNING: Unable to allocate memory for accessnet, returning NULL.\n");
      return NULL;
    }
    strcpy(output,val[0]);
    return output;
  }
  return NULL;
}

/* Gets user's full name (first + middle + last). */
char * get_full_name() {
  char ** first_name = getValue("person_names", tuid, 0, 1);
  char ** middle_name = getValue("person_names", tuid, 0, 2);
  char ** last_name = getValue("person_names", tuid, 0, 3);

  /* Allocate space for full name (first + middle + last). */
  char * output = malloc((strlen(first_name[0]) + strlen(middle_name[0])
     + (strlen(last_name[0]) + 1) * sizeof(char *)));

   if(output == NULL) {
     fprintf(stderr, "WARNING: Unable to allocate memory for full name, returning NULL.\n");
     return NULL;
    }

  /* Combines frist, middle, and last into full name. */
  sprintf(output,"%s%s%s",first_name[0],middle_name[0],last_name[0]);
  return output;
}

/* Gets user's preferred name. */
char * get_prefer_name() {
  /* Preferred name. */
  char ** name = getValue("person_names", tuid, 0, 4);
  if(name != NULL) {
    /* Allocate space for output. */
    char * output = malloc(sizeof(char *) * ((strlen(name[0])) + 1));
    if(output == NULL) {
      fprintf(stderr, "WARNING: Unable to allocate memory for preferred name, returning NULL.\n");
      return NULL;
    }
    strcpy(output,name[0]);
    return output;
  }
  return NULL;
}

/* Gets user's role ids and checks to see which role ids
have expired. If all roles are expired then it exits the
program. */
char ** get_role_ids() {
  int length, i, index = 0;
  /* Sets the number of matches and roles to 0. */
  total_found = 0;
  num_roles = 0;
  /* Role ids. */
  char ** val = getValue("person_roles", tuid, 0, 1);
  /* Amount of role ids found. */
  length = total_found;
  /* Expiration dates. */
  char ** expiration_dates = getValue("person_roles", tuid, 0, 2);
    if (expiration_dates != NULL && val != NULL) {
      /* Allocate space for the output. */
      char ** output = malloc(sizeof(char *) * (length + 1));
      if(output == NULL) {
        fprintf(stderr, "WARNING: Unable to allocate memory for role ids, returning NULL.\n");
        return NULL;
      }
      for(i = 0; i < length; i++){
        output[i] = malloc(((strlen(val[i])) + 1) * sizeof(char *));
        if(output[i] == NULL) {
          fprintf(stderr, "WARNING: Unable to allocate memory for role ids, returning NULL.\n");
          free(output);
          return NULL;
        }
      }

      /* Gets the current date. */
      char date[9] = "";
      int year, day, month;
      time_t now = time(NULL);
      struct tm *local = localtime(&now);
      year = local->tm_year + 1900;
      month = local->tm_mon + 1;
      day = local->tm_mday;
      /* Combines the year, month, and day into one string. */
      sprintf(date,"%d%d%d",year,month,day);

      /* Finds out which roles are not expired and puts them in the output. */

        while (index != -1) {
          /* Checks if expiration date is passed current date. */
          if (atoi(date) < atoi(expiration_dates[index])) {
            strcpy(output[num_roles], val[index]);
            num_roles++;
          }
          index = expiration_dates[index+1] != NULL && val[index+1] != NULL ? index + 1 : -1;
        }

      /* Exit program if all roles expired. */
      if (num_roles == 0) {
        printf("All role ids for tuid %s have expired.\n", tuid);
        exit(0);
      }
      return output;
  }
  return NULL;
}

/* Loops through the role ids and gets the respective
resource ids. */
int * get_resource_ids() {
  int length = num_roles, i, j, temp;
  num_resources = 0;
  /* Allocate space for output. */
  int * output = malloc(sizeof(int) * 25);

  if(output == NULL) {
    fprintf(stderr, "WARNING: Unable to allocate memory for resource ids, returning NULL.\n");
    free(output);
    return NULL;
  }

  for(i = 0; i < length; i++) {
    total_found = 0;
    /* Resource ids. */
    char ** val = getValue("resources_roles", role_ids[i], 1, 0);
    j = 0;
    if (val != NULL) {
      /* Amount of resource ids found. */
      temp = total_found;
      for(j = 0; j < temp; j++) {
        output[num_resources] = atoi(val[j]);
        num_resources++;
      }
    }
  }
  if(num_resources == 0) {
    printf("No resources found.\n");
    exit(0);
  }
  return output;
}

/* Removes any extra resource ids. */
int * remove_extras() {
  int i, j, length = num_resources;
  for(i = 0; i < length; i++) {
    for(j = i + 1; j < length; j++) {
      /* If a repeated value is found the second value is replaced with REMOVED. */
      if(resource_ids[i] == resource_ids[j]) {
        resource_ids[j] = REMOVED;
      }
    }
  }
  return resource_ids;
}

/* Gets the resource names. */
char ** get_resource_names() {
  int i, length = num_resources, index = 0;
  /* Allocate space for the output. */
  char ** output = malloc(sizeof(char *) * (length + 1));
  if(output == NULL) {
    fprintf(stderr, "WARNING: Unable to allocate memory for resource names, returning NULL.\n");
    return NULL;
  }

  for(i = 0; i < length; i++) {
    /* If current id is not a repeated value. */
    if(final_resource_ids[i] != REMOVED) {
      char temp[5];
      /* Converts int to a string. */
      sprintf(temp, "%d", final_resource_ids[i]);
      /* Resource name. */
      char ** val = getValue("resources", temp, 0, 1);
      if(val != NULL) {
        /* Allocate space for output[i]. */
        output[i] = malloc(sizeof(char *) * (strlen(val[0])));
        if(output[i] == NULL) {
          fprintf(stderr, "WARNING: Unable to allocate memory for resource names, returning NULL.\n");
          free(output);
          return NULL;
        }
        strcpy(output[index], val[0]);
        index++;
      }
    }
  }
  return output;
}

/* Writes appropriate infomation to file. */
void write_to_file() {
  FILE *fp;
  int i, length = num_resources;

  /* Opens file. */
  fp = fopen("person_resource", "a");
    if(fp == NULL) {
      fprintf(stderr, "WARNING: Unable to open file.\n");
      exit(1);
    }

  for(i = 0; i < length; i++) {
    if(final_resource_ids[i] != REMOVED) {
      char temp[5];
      /* Converts int to a string. */
      sprintf(temp, "%d", final_resource_ids[i]);
      /* Name type (full or preferred). */
      char ** name_type = getValue("resource_requirements", temp, 0, 1);
      /* Id type (tuid or accessnet). */
      char ** id_type = getValue("resource_requirements", temp, 0, 2);

      /* Sends preferred name or full name to file. */
      if(strcmp(name_type[0],"preferred")) {
        fputs(preferred_name, fp);
      } else if(strcmp(name_type[0],"full")) {
        fputs(full_name, fp);
      }

      fputs("#", fp);

      /* Send tuid or accessnet to file. */
      if(strcmp(id_type[0],"accessnet")) {
        fputs(access_net, fp);
      } else if(strcmp(id_type[0],"tuid")) {
        fputs(tuid, fp);
      }

      fputs("#", fp);

      /* Sends resource id and name to file. */
      fputs(temp, fp);
      fputs("#", fp);
      fputs(resource_names[i], fp);
      fputs("\n", fp);
    }
  }
  /* Closes the file. */
  fclose(fp);
}

/* Frees all alocated memory. */
void free_memory() {
  int i;
  free(tuid);
  free(access_net);
  free(full_name);
  free(preferred_name);
  for(i = 0; i < num_roles; i++) {
    free(role_ids[i]);
  }
  free(role_ids);
  free(final_resource_ids);
  for(i = 0; i < num_resources; i++) {
    free(resource_names[i]);
  }
  free(resource_names);
}
