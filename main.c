#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "db_helper.h"
#include "my_file.h"


/***************************************************************************
  1. Reads TUID from stdin
  2. Gets the acessnet associated wth tuid
  3. Gets the full name (first + middle + last)
  4. Gets the preferred name
  5. Gets the role ids for that person and checks which ones are expired
  6. Gets the resource ids
  7. Removes any duplicate resource ids
  8. Gets the resource names
  9. Writes the appropriate information to the person_resource file

***************************************************************************/
int main(int argc, char *argv[]) {
    if (argc != 2){
      printf("Please enter a TUID.\n");
  } else {
      if (is_valid(argv[1])) {
        /* Gather the necessary information from tuid. */
        tuid = get_tuid(argv[1]);
        access_net = get_access_net();
        full_name = get_full_name();
        preferred_name = get_prefer_name();
        role_ids = get_role_ids();
        resource_ids = get_resource_ids();

        /* Removes repeated resource ids. */
        final_resource_ids = remove_extras();

        /* Gets the resource names for the resource ids. */
        resource_names = get_resource_names();

        /* Writes the information to a file. */
        write_to_file();

        /* Frees any allocated memory. */
        free_memory();
    } else {
      printf("Invalid TUID.\n");
    }
  }
  return 0;
}
