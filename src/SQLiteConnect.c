/**
 * File              : SQLiteConnect.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 04.09.2021
 * Last Modified Date: 11.04.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "SQLiteConnect.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"

#define ERROR(...) ({char ___err[BUFSIZ]; sprintf(___err, __VA_ARGS__); perror(___err);})

//create SQLite database
int 
sqlite_connect_create_database(const char *filename)
{
	sqlite3 *db;
	int res = sqlite3_open_v2(filename, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	if (res) 
		ERROR("SQLite: Failed to create '%s': %s\n", filename, sqlite3_errmsg(db));
		return res;
	sqlite3_close(db);
	return 0;
}

//return number of rows for SQL request
int sqlite_connect_execute_function(const char *sql, const char *filename, void *user_data, int (*callback)(void*,int,char**,char**)){
	int res;

	sqlite3 *db;
	sqlite3_stmt *stmt;
	
	res = sqlite3_open(filename, &db);
	if (res != SQLITE_OK) {
		ERROR("SQLite: Failed to open '%s': %s\n", filename, sqlite3_errmsg(db));
		return res;
	}
	
	res = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	if (res != SQLITE_OK) {
		ERROR("SQLite: Failed to execute '%s': %s\n", sql, sqlite3_errmsg(db));
		sqlite3_close(db);
		return 0;
	}	

	int num_cols = sqlite3_column_count(stmt); //number of colums
	char *titles[num_cols]; //names of colums
	
	//fill column titles
	int i;
	for (i = 0; i < num_cols; ++i) {
		titles[i] = NULL;
		if (sqlite3_column_name(stmt, i) != NULL) {
			titles[i] = (char *)sqlite3_column_name(stmt, i);
		}
	}	

	//fill values and exec callback for each row
	char *argv[num_cols]; //values
	while (sqlite3_step(stmt) != SQLITE_DONE) {
		int i;
		for (i = 0; i < num_cols; ++i) {
			argv[i] = NULL;
			if (sqlite3_column_text(stmt, i) != NULL) {
				argv[i] = (char *)sqlite3_column_text(stmt, i);
			}			
		}
		
		if (callback) {
			int c = callback(user_data, num_cols, argv, titles); //run callback
			if (c) { //callback return non zero - stop execution
				ERROR("SQLiteExecute interupted with code: %d", c);
				sqlite3_finalize(stmt);
				sqlite3_close(db);				
				return c;
			}
		}
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);

	return 0;
}

int sqlite_connect_get_string_callback(void *data, int argc, char **argv, char **titles){
	char *string=(char*)data;
	strcpy(string, "");		
	
	if (argv[0]!=NULL){
		strncpy(string, (const char*)argv[0], BUFSIZ - 1); //safe string copy
		string[BUFSIZ - 1] = '\0';
	}	

    return -1; //do not call callback again
}

//return one string from SQL request
int sqlite_connect_get_string(const char *sql, const char *filename, char * string){
	return sqlite_connect_execute_function(sql, filename, string, sqlite_connect_get_string_callback);	
}

//standart callback - print in STDOUT
int sqlite_callback_print(void *data, int argc,  char **argv, char **titles){
    int i;
	for (i=0; i< argc; i++)
		if (argv[i] != NULL){
			printf("%s,\t", argv[i]);
		}
		else {
			printf("%s,\t", "(null)");
		}
    printf("\n");
    return 0;
}

//execute SQL without callback
int sqlite_connect_execute(const char *sql, const char *filename){
	int res = 0;	
    sqlite3 *db;
    res = sqlite3_open(filename, &db);
    
    if (res != SQLITE_OK) {
		ERROR("SQLite: Failed to open '%s': %s\n", filename, sqlite3_errmsg(db));
		return res;
	}

	char *err = NULL; 
	res = sqlite3_exec(db, sql, sqlite_callback_print, 0, &err); 
	
	if (err != NULL){
		ERROR("SQLite returned error: %s\n", err);
		sqlite3_close(db);	
		return res;
	}

    sqlite3_close(db);	
	return 0;
}



