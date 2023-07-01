/**
 * File              : excel.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 07.04.2023
 * Last Modified Date: 23.04.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef EXCEL_H
#define EXCEL_H

#include <stdio.h>
#include <stdlib.h>

#include "zakaz.h"
#include "xlsxwriter.h"
#include "SQLiteConnect.h"
#include "libxlsxwriter_open_template.h"

#define STARTLINE_ZAKAZ 5
#define STARTLINE_AKT 6
#define COLUMN_NUMBER  1
#define COLUMN_PROD  2
#define COLUMN_KOD  3
#define COLUMN_TITLE  4
#define COLUMN_COUNT  5

struct make_excel_d {
	lxw_workbook  *wb;
	lxw_worksheet *ws;
	lxw_format *format;
	int start;
	int line;
};

static int 
make_excel_cb(void *user_data, int argc, char **argv, char **titles){
	struct make_excel_d *d = user_data;

	lxw_format *format = NULL;	
	int row_num = d->start + d->line;

	lxw_row *row = lxw_worksheet_find_row(d->ws, row_num);

	int i;
	for (i = 1; i < 10; ++i) {
		if (row) {
			lxw_cell *cell = lxw_worksheet_find_cell_in_row(row, i); 
			if (cell)
				format = cell->format;
		}			
		switch (i) {
			case COLUMN_NUMBER:
				worksheet_write_number(d->ws, row_num, COLUMN_NUMBER, 
						d->line + 1, format);	
				break;

			case COLUMN_PROD:
				worksheet_write_string(d->ws, row_num, COLUMN_PROD, 
						"Конмет", format);	
				break;				

			case COLUMN_TITLE:
				{
					char *title = argv[0];
					if (!title)
						title = "";
					worksheet_write_string(d->ws, row_num, COLUMN_TITLE, title, format);	
					break;
				}

			case COLUMN_COUNT:
				{
					char *count = argv[1];
					if (!count)
						count = "";
					worksheet_write_number(d->ws, row_num,  COLUMN_COUNT, 
							atoi(count), format);	
					break;
				}				

			case COLUMN_KOD:
				{
					char *kod = argv[2];
					if (!kod)
						kod = "";	
					worksheet_write_string(d->ws, row_num,  COLUMN_COUNT, 
							kod, format);	
					break;
				}				
				

			default: break;
		}
		
	}
	
	d->line++;
	
	return 0;
}

static char * 
make_zakaz(){
	char *file = "zakaz.xlsx";
	remove(file);
	
	lxw_workbook  *wb  = workbook_new_from_template(file, "template.xlsx");	
	lxw_format *format = workbook_add_format(wb);	
	lxw_worksheet *ws  = workbook_get_worksheet_by_name(wb, "Лист1");

	struct make_excel_d d = {wb, ws, format, STARTLINE_ZAKAZ, 0};

	sqlite_connect_execute_function(
			"SELECT title, count, kod FROM zakazi", 
			"zakaz.db", &d, make_excel_cb);

	workbook_close(wb);

	return file;
}

static char * 
make_akt(){
	char *file = "akt.xlsx";
	remove(file);
	
	lxw_workbook  *wb  = workbook_new_from_template(file, "template_akt.xlsx");	
	lxw_format *format = workbook_add_format(wb);	
	lxw_worksheet *ws  = workbook_get_worksheet_by_name(wb, "Лист1");

	struct make_excel_d d = {wb, ws, format, STARTLINE_AKT, 0};

	sqlite_connect_execute_function(
			"SELECT title, count, kod FROM zakazi", 
			"zakaz.db", &d, make_excel_cb);

	workbook_close(wb);

	return file;
}
#endif /* ifndef EXCEL_H */
