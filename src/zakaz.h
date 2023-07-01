/**
 * File              : zakaz.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 05.04.2023
 * Last Modified Date: 12.04.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef ZAKAZ_H
#define ZAKAZ_H

#include <gtk/gtk.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "sqlite3.h"
#include "getbundle.h"
#include "strfind.h"
#include "strsplit.h"
#include "SQLiteConnect.h"

#define DATABASE "zakaz.db"
#define STR(...) ({char s[BUFSIZ]; sprintf(s, __VA_ARGS__); s;})

static GtkWidget * mainWindow;

struct zakaz {
	time_t id;            // id
	char title[256];      // title
	char parentname[256]; // parent title
	int c;                // count
	char kod[16];         // kod
	int n;				  // number
};

struct data {
	int id;          // id
	int parent;      // parent
	char title[256]; // title
	char kod[16];    // kod
};

static int 
zakaz_init(char *argv[]){

	//get bundle directory
	char *bundle = getbundle(argv);
	if (!bundle){
		g_error("can't get application bundle\n");
		return 1;
	}
	
#ifdef _WIN32
	//#include <direct.h>
	//chworkdir
	chdir(bundle);	
#endif

	//free bundle var
	free(bundle);

	//create database
	if (sqlite_connect_create_database(DATABASE)){
		printf("can't open/create database\n");
		return 1;
	}

	char *SQL;
	//create zakazi table
	SQL = 
		"CREATE TABLE IF NOT EXISTS "
		"zakazi "
		"( "
		"id INT, "
		"title TEXT, "
		"parentname TEXT, "
		"count INT, "
		"kod TEXT "
		")"
		;	
	if (sqlite_connect_execute(SQL, DATABASE)){
		printf("can't create table 'zakazi'\n");
	}	
	
	//check data table
	char ptr[BUFSIZ] = {0};
	if (sqlite_connect_get_string("SELECT 1 FROM data", DATABASE, ptr)){
		fprintf(stderr, "DATABASE EXISTS!: %s\n", ptr);
		return 0;
	} else 
		fprintf(stderr, "No DATABASE - create new: %s\n", ptr);

	// create data table if not exists
	SQL = 
		"CREATE TABLE IF NOT EXISTS "
		"data "
		"( "
		"id INT, "
		"parent INT, "
		"title TEXT, "
		"kod TEXT "
		")"
		;	
	if (sqlite_connect_execute(SQL, DATABASE)){
		printf("can't create table 'data'\n");
	}	

	//fill data table with data
	FILE *fp = fopen("catalog.csv", "r");
	if (!fp){
		printf("can't open 'catalog.csv'\n");
	}
	char buf[BUFSIZ];
	while (fgets(buf, BUFSIZ, fp) != NULL){
		//remove new line char
		int len = strlen(buf);
		buf[len-1] = 0;

		char **tokens; 
		int c = strsplit(buf, ";", &tokens);
		if (c != 4){
			printf("error to read line from 'catalog.csv': %s\n", buf);
			continue;
		}
		SQL = STR(
			"INSERT INTO data (id, parent, title, kod) "
			"VALUES ('%s', '%s', '%s', '%s'); "
			, tokens[0], tokens[1], tokens[2], tokens[3]);
		if (sqlite_connect_execute(SQL, DATABASE)){
			printf("can't add line to table 'data': %s\n", SQL);
		}	
	}
	
	return 0;
}

struct get_zakazi_d {
	void *user_data;
	int (*callback)(void *user_data, struct zakaz *zakaz);
};

static int 
get_zakazi_cb(void *user_data, int argc, char **argv, char **titles){

	struct get_zakazi_d *d = user_data;
	struct zakaz z;
	int i;
	for (i = 0; i < argc; ++i) {
		char *buf = argv[i];
		if (!buf)
			buf = "";
		switch (i) {
			case 0: z.id = atol(buf); break; 
			case 1: {
						strncpy(z.title, buf, sizeof(z.title) - 1);
						z.title[sizeof(z.title) - 1] = 0;
						break;
					} 
			case 2: z.c = atoi(buf); break; 
			case 3: {
						strncpy(z.kod, buf, sizeof(z.kod) - 1);
						z.kod[sizeof(z.kod) - 1] = 0;
						break;
					}
		}
	}
	if (d->callback)
		if (d->callback(d->user_data, &z))
			return 1;

	return 0;
}

static int 
get_zakazi(
			void *user_data,
			int (*callback)(void *user_data, struct zakaz *zakaz)
		)
{	
	struct get_zakazi_d d = {user_data, callback};
	return sqlite_connect_execute_function(
			"SELECT id, title, count, kod FROM zakazi", DATABASE, &d, get_zakazi_cb);
}

static time_t 
add_zakaz(const char *title, int count, const char *kod){
	time_t id = time(NULL);
	char *SQL = STR(
			"INSERT INTO zakazi (id, title, count, kod) "
			"VALUES (%ld, '%s', %d, '%s'); "
			, id, title, count, kod
			);
	if (sqlite_connect_execute(SQL, DATABASE))
		return 0;
	return id;
}

static int 
set_zakaz(struct zakaz *zakaz){
	char *SQL = STR(
			"UPDATE zakazi SET "
			"title = '%s', "
			"count = %d, "
			"kod = '%s' "
			"WHERE id = %ld"
			, zakaz->title, zakaz->c, zakaz->kod
			, zakaz->id
			);	
	return sqlite_connect_execute(SQL, DATABASE);
}

static int 
remove_zakaz(struct zakaz *zakaz){
	char *SQL = STR(
			"DELETE FROM zakazi "
			"WHERE id = %ld"
			, zakaz->id
			);
	return sqlite_connect_execute(SQL, DATABASE);
}

struct get_data_d {
	void *user_data;
	int (*callback)(void *user_data, struct data *data);
};

static int 
get_data_cb(void *user_data, int argc, char **argv, char **titles){
	struct get_data_d *d = user_data;
	struct data z;
	
	int i;
	for (i = 0; i < argc; ++i) {
		char *buf = argv[i];
		if (!buf)
			buf = "";
		switch (i) {
			case 0: z.id = atoi(buf); break; 
			case 1: z.parent = atoi(buf); break; 
			case 2: {
						strncpy(z.title, buf, sizeof(z.title) - 1);
						z.title[sizeof(z.title) - 1] = 0;
						break;
					} 
			case 3: {
						strncpy(z.kod, buf, sizeof(z.kod) - 1);
						z.kod[sizeof(z.kod) - 1] = 0;
						break;
					}
		}
	}
	if (d->callback)
		if (d->callback(d->user_data, &z))
			return 1;

	return 0;
}

static int 
get_data(
			int parent,
			void *user_data,
			int (*callback)(void *user_data, struct data *data)
		)
{	
	char SQL[BUFSIZ];
	sprintf(SQL,
			"SELECT id, parent, title, kod FROM data "
			"WHERE parent = %d"
			, parent
			);	
	struct get_data_d d = {user_data, callback};
	fprintf(stderr, "SQL: %s\n", SQL);
	return sqlite_connect_execute_function(SQL, DATABASE, &d, get_data_cb);
}

#endif /* ifndef ZAKAZ_H */
