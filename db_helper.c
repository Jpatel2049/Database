#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "db_helper.h"
#include "my_file.h"


/**********************************************************************
  Counts the number of columns in in a database row. The caller should
  pass in the header row of the table, though this function will still
  work if passed a row with data.

  Input arguments:
      char * data - The row of data to use for counting
      char delimiter - The delimiter used in data to separate columns

  Return value:
      0, if data == NULL
      1 plus the count of how many times the delimiter was found,
      otherwise
**********************************************************************/
int getNumColumns(char * data, char delimiter) {
    int count = 1;

    if (data == NULL) {
        return 0;
    }

    while (*data) {
        count += (*(data++) == delimiter);
    }

//    printf ("Count of Columns: %d\n", count);
    return count;
}

/**********************************************************************
  Counts the number of columns in in a database row. The caller should
  pass in the header row of the table, though this function will still
  work if passed a row with data.

  Input arguments:
      char * data - The row of data to use for counting
      char delimiter - The delimiter used in data to separate columns

  Return value:
      The count of how many rows were found

      NOTE: This count will include the header row if there is one
            since it has no way of telling whether or not there is a
            header row in the file given to it.
**********************************************************************/
int getNumRows(FILE *fp) {
  int ch;
  int charsOnCurrentLine = 0;
  int count = 0;

  while ((ch = fgetc(fp)) != EOF) {
    if (ch == '\n') {
      count++;
      charsOnCurrentLine = 0;
    } else {
      charsOnCurrentLine++;
    }
  }
  if (charsOnCurrentLine > 0) {
    count++;
  }

  fseek(fp, 0, SEEK_SET);

//  printf ("Count of Rows (includes header row, if there is one): %d\n", count);
  return count;
}

/**********************************************************************
  This is just a wrapper for the getline library function. The getline
  function on its own does not remove trailing newline characters.
  Because we wish to examine each line of the database file without
  the newline character, we must take an extra step in order to
  do this.

  Input arguments:
      char ** lineptr - Pointer to the c string where the line should
                        be stored when it is read
      FILE * stream - The file from which the line should be read

  Return value:
      ssize_t - The length of the line read, not including the null
      terminating character ('\0')
**********************************************************************/
ssize_t readline(char **lineptr, FILE *stream)
{
  size_t len = 0;  /* Size of the buffer, ignored. */

  /* Call getLine library function to retrieve the next line from the
     file stream (returns number of characters read, including the
     newline character at the end of the line if there was one */
  ssize_t chars = getline(lineptr, &len, stream);

  /* Replace any new line character at the end of the line with the
     null character and decrement the count of characters read */

  char *nl = strchr(*lineptr, '\n');
  if(nl != NULL)
    strncpy(nl, "\0", 1);

  char *cr = strchr(*lineptr, '\r');
  if(cr != NULL)
    strncpy(cr, "\0", 1);

  /* Return the number of characters in the line */
  return chars;
}

/**********************************************************************
  Gets the value stored in a particular column of a particular
  database table based on a search value and the column where it will
  be found if it exists.

  Input arguments:
      char * dbTable - The name of the database table (filename)
      char * searchStr - The string to search for within the table
      int colSearch - The column in which to look for searchStr
      int colResult - If searchStr is found, the column of that row
                      from which to retrive the return value

  Return value:
      NULL - If there was a problem or if searchStr could not be found
      char ** - Returns an array of strings, containing any value
                found in colResult of the row in which searchStr was
                found
**********************************************************************/
char ** getValue(char * dbTable, char * searchStr,
		 int colSearch, int colResult) {
  FILE *fp; /* The file pointer we will use to open the file that
	       holds the requested database table data */
  char *line = NULL; /* C-string that will be used to store the
			current line of the table that is being
			processed */
  char *temp = NULL; /* C-string that will hold a copy of the current
			line (the purpose of doing this is discussed
			in another comment below) */
  char **retVals; /* The list of strings that were found in the
		     specified column in rows where the search string
		     had been found. This is what will be returned to
		     the caller. */
  ssize_t lineLength; /* The length of the current line (not including
			 the null character) */
  int numCols = 0; /* The total number of columns in the table */
  int numRows = 0; /* The total number of rows in the table */
  int currCol = 0; /* The current column being examined */
  int currRow = 0; /* The current count of rows that have the
		      requested string in the column specified; used
		      to keep track of which array index should be
		      used when inserting the result value into the
		      array of C-strings that is to be returned by
		      this function. */
  int searchFound = 0; /* Flag that we will use to keep track of
			  whether or not the string we are looking
			  for was found in the specified column of
			  the current row; initialized to zero since
			  when we are just beginning, the string could
			  not possibly have been found yet */


  /* Attempt to open the file for reading and print a warning and
     return NULL if it failed to open */
  if((fp = fopen(dbTable, "r")) == NULL) {
    fprintf(stderr, "WARNING: Unable to open specified file, returning NULL.\n");
    return NULL;
  }

  /* Determine the total number of rows in the specified file */
  numRows = getNumRows(fp);

  /* Allocate memory for return values. Note that we do not know
     exactly how many return values that we have, but what we do know
     is that at most, we have numRows values to return.

     retVals is a char **, so we need to cast it as such. Then for
     every row that we have, we need a chunk of memory the size of a
     char * to use to point to each potential return value. */
  retVals = (char **) malloc(numRows * sizeof(char *));

  /* Attempt to read the first line of the file. If there was a
     problem doing so or if the number of columns found is zero,
     print a warning and return NULL */
  if (readline(&line, fp) == -1
        || (numCols = getNumColumns(line, '#')) == 0) {
    fprintf(stderr, "WARNING: Unable to open specified file, returning NULL.\n");
    return NULL;
  }

  /* Check to make sure that both the column to search in and the
     column to return actually exist, and print a warning and return
     NULL if either do not exist */
  if (numCols <= colSearch || numCols <= colResult) {
    fprintf(stderr, "WARNING: Search or result column index does not exist, returning NULL.\n");
    return NULL;
  }

  /* While the current row count is less than the total row count and
     when we read the next line in we do not have any issue doing
     so... */
  while (currRow < numRows && (lineLength = readline(&line, fp)) != -1) {
    currCol = 0;

    /* Allocate memory that will be used to store a temporary copy of
       the current row. We need to cast it as a char * (i.e. a
       C-string). Note that we need to request lineLength plus one
       since lineLength does not account for the null character at
       the end of the string. */
    char * tempLine = (char *) malloc((lineLength + 1) * sizeof(char));

    /* Copy the current line that has been read into tempLine */
    strcpy(tempLine, line);

  //  printf("\nline: %s\n", line);
  //  printf("tempLine: %s\n", tempLine);

    /* Separate the delimited string that is the current line, and
       while the current token does not equal NULL and we have not
       found what we are looking for... */
    while((temp = strsep(&line,"#")) != NULL) {
  //    printf("currCol: %d; temp: %s\n", currCol, temp);

      if (!searchFound && currCol < numCols) {
	/* If the current column is the column that we have been asked
	   to search, ...*/
	if (currCol == colSearch) {
	 // printf("searchStr: %s; temp: %s\n", searchStr, temp);
	  /* Compare the string we are searching for to the current
	     token. If they are equal set searchFound to TRUE
	     (i.e. 1). Otherwise, set to FALSE (i.e. 0). */
	  int comp = strcmp(temp, searchStr);
	  searchFound = (strcmp(temp, searchStr) == 0);

//	  printf("searchFound: %d\n", searchFound);
	}
      }
	/* Increment the index of the current column and then go back
	   and once again check the loop condition.*/
	currCol++;

    }

    /* Once we've determined if the value exists in the current row,
       we need to reset the index of the current column to zero. */
    currCol = 0;

    /* Next, if we've found what we were looking for in this row, we
       separate the delimited string again, though this time, we use
       the copy that we created earlier since the original now points
       to the null character at the end of the string (you can
       "rewind" the original string, but I did not do this
       here).

       The only reason we must iterate through a second time is
       because during the first pass, we search each token to see if
       the string we are looking for was found but we did not save
       the value to return. This would be the better way to do it,
       but I broke it out into two separate iterations here to avoid
       the extra layer of complexity (even if it is a very minor
       one).

       If you have been able to understand the code so far, I
       suggest that you at least attempt to implement simple
       improvements like the two I mentioned above ("rewind" the
       original pointer and condense the two iterations through the
       row into one) */
    while( (temp = strsep(&tempLine,"#")) != NULL && searchFound) {
      /* If the current column is the one from which our result value
	 should be selected... */

      if (currCol == colResult) {

	/* Allocate memory for this value.

	   Remember, retVals is declared as char **. Another way to
	   think about it is as an array that contains a char pointer
	   at each index, so whenever we go to set the next array
	   element (i.e. retVals[currRow]), we need to allocate memory
	   for the char pointer that will be stored here. Adding one
	   to the length of temp is necessary here because strlen
	   does not count the null character in the length, but you
	   need to have room for it. */
	retVals[currRow] = (char *) malloc((strlen(temp) + 1) * sizeof(char));

	/* Copy temp into the memory that we just allocated. */
	strcpy(retVals[currRow], temp);
  /* int to keep track of number of matches found. */
  total_found++;

	/* Increment currRow so that if there are additional values
	   to be added to the retVals array, we will have the correct
	   index ready. */
	currRow++;

      }

      /* Increment currCol so that we can check the next column in
	 the row. */
      currCol++;
    }
	/* Reset the searchFound flag to FALSE (i.e. 0) so that we
	   are ready to process the next table row. */
	searchFound = 0;
    /* If line is not equal to NULL, then free the memory allocated
       to it.

       NOTE: If you do not do this, you will likely see a
       "segmentation fault" error. In this case, it would be because
       of a failure to free memory, but in other cases, you might see
       this error if there is a failure to properly allocate memory as
       well. */
    if (line != NULL) {
      free(line);
    }
  }

  /*  printf("Hello Everyone\n"); //This will print Hello Everyone

  printf("Hello Every\rone\n"); //This will print onelo Every

  printf("\n\n");*/

  /* Once we've gone through all of the rows of the table, return
     retVals. */
  return retVals;
}

/**********************************************************************
  Allocates the memory needed for the structure (in this case for the
  Role structure), and initializes and allocates any memory needed for
  the struct members (in this case it is necessary to allocate memry
  for the C-string struct member, name prior to assigning the
  specified value to it, but since the id is just an integer, all that
  is needed here is the value assignment.

  While it is not required that you use this or any of the other
  structs that I defined, if you do want to use any of them and/or any
  structs that you define yourself, you can use this as a template
  for how to ensure that you allocate the memory that is
  needed. Remember, if a struct member variable is a pointer, you need
  to allocate memory for it unless you are setting it to point to
  memory that has already been allocated elsewhere.

  Input arguments:
      int id - The id of the role that we are to represent with the
               resultant struct
      char *name - The name of the role that we are to represent with the
                   resultant struct

  Return value:
     Role * - A pointer to a Role struct for which the memory has
              been allocated and the struct members initialized to the
              specified values.
**********************************************************************/
Role *newRole(int id, char *name) {
  /* Attempt to allocate memory for the role structure. */
  Role *role = (Role *) malloc (sizeof (Role));

  /* If we were not successful in allocating the memory, print a
     warning message and return NULL */
  if (role == NULL) {
    fprintf(stderr, "WARNING: Unable to allocate memory for the role structure, returning NULL.\n");
    return NULL;
  }

  /* Attempt to allocate memory for the name of the role, and if this
     attempt fails, print a warning and free the memory allocated for
     the entire role structure. */
  role->name = (char *) malloc (strlen(name) + 1);
  if (role->name == NULL) {
    fprintf(stderr, "WARNING: Unable to allocate memory for the name member of the role structure, freeing the memory allocated to the role struct and returning NULL.\n");
    free(role);
    return NULL;
  }

  /* Set the id and name and then return the struct pointer. */
  role->id = id;
  role->name = strcpy(role->name, name);

  return role;
}
