/**
 * File              : SQLiteConnect.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 04.09.2021
 * Last Modified Date: 23.04.2022
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef SQLITE_CONNECT
#define SQLITE_CONNECT

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

//create SQLite database - return 0 if the database is opened (and/or created) successfully - Otherwise an error code is returned (https://www.sqlite.org/rescode.html)
int sqlite_connect_create_database(const char *filename);	

//execute SQLite SQL STRING for DATABASE
//user_data - return void in CALLBACK
//to stop callback - return non zero
//return 0 if successfully - Otherwise an error code is returned. Alsow return code if callback
//was interupted
int sqlite_connect_execute_function(
		const char *sql,		//SQL string to execute
		const char *filename,   //SQLite database filename
		void *user_data,        //pointer to transfer throw callback
		int (*callback)(		//callback function. return non zero to stop function 
			void* user_data,    //transfered pointer
			int argc,           //count of arguments(columns)
			char** argv,        //aray of arguments (cell values)
			char** titles		//aray of column titles
		)
);

//execute SQL without callback
int sqlite_connect_execute(
		const char *sql,        //SQL string to execute
		const char *filename    //SQLite database filename
);

//get one String (first row and first column) from SQL Request
int sqlite_connect_get_string(
		const char * sql,       //SQL string to execute 
		const char * filename,  //SQLite database filename
		char * string           //string to return
);

#ifdef __cplusplus
}  /* end of the 'extern "C"' block */
#endif

#endif /* ifndef SQLITE_CONNECT */
