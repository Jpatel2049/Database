#ifndef DBHELPER_H_
#define DBHELPER_H_

typedef struct role {
	int id;
	char* name;
} Role;

typedef struct account {
	char* accessnet;
	char* tuid;
	char* full_name;
	char* preferred_name;
	char** role_ids;
} Account;

typedef struct resource {
	int id;
	char* name;
	char* person_name_type;
	char* person_id_type;
	Role* roles_authorized;
} Resource;

typedef struct authz {
	Account account;
	Resource* systems;
} Authz;

/***********************************************************
* Function declarations *
************************************************************/
ssize_t readline(char **, FILE *);
char ** getValue(char *, char *, int, int);

Role * newRole(int, char*);
char * getRoleNameById(int);
int getRoleIdByName(char*);
/*You should not have a need to add, update or
delete rows programmatically in this assignment.
These functions are included for each entity type
to give you a starting point in case you want to
go beyond the requirements of the assignment
either now or later on.*/
int addRole(Role);
int updateRole(Role);
int deleteRoleById(int);
int deleteRoleByName(char*);

Account getAccount(char*);
int addAccount(Account);
int updateAccount(Account);
int deleteAccount(char*);

Resource getResourceById(int);
Resource getResourceByName(char*);
int addResource(Resource);
int updateResource(Resource);
int deleteResourceById(int);
int deleteResourceByName(char*);

Authz selectAuthzInfoForAccount(char*);



int setSyncStatusForAccount(int, char*);

#endif
