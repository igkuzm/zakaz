/**
 * File              : data_view.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 05.04.2023
 * Last Modified Date: 12.04.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef DATA_VIEW_H
#define DATA_VIEW_H

#include <gtk/gtk.h>
#include "zakaz.h"
#include "zakazi_view.h"
#include "support.h"
#include "interface.h"


static GtkTreeStore *DataViewStrore;

enum {
  DATA_COLUMN_TITLE,
  DATA_COLUMN_KOD,
  DATA_ID,
  DATA_PARENT,
  N_COLUMNS,
};

static GtkTreeStore *
data_view_table_model_new(){
	GtkTreeStore *store = gtk_tree_store_new(N_COLUMNS, 
			G_TYPE_STRING, //title 
			G_TYPE_STRING, //kod 
			G_TYPE_INT,    //id
			G_TYPE_INT     //parent
	);

	return store;
}

static void 
data_view_add_to_store(GtkTreeStore *store, struct data *data, GtkTreeIter *iter, GtkTreeIter *parent){
	gtk_tree_store_append(store, iter, parent);
	gtk_tree_store_set(store, iter, 
			DATA_COLUMN_TITLE, data->title, 
			DATA_COLUMN_KOD,   data->kod, 
			DATA_ID, data->id, 
			DATA_PARENT, data->parent, 
	-1);

}

static int 
data_view_fill_store(void *user_data, struct data *data){

	GtkTreeIter *parent = user_data;

	GtkTreeIter iter;
	data_view_add_to_store(DataViewStrore, data, &iter, parent);

	//if (data->id)
		get_data(data->id, &iter, data_view_fill_store);

	return 0;
}

static void
data_view_store_update(const char *search, GtkTreeStore *store){
	gtk_tree_store_clear(store);

	get_data(0, NULL, data_view_fill_store);
	
	/*if (search) {*/
		/*stroybat_data_get(DATABASE, datatype, search, NULL, gtroybat_fill_items_list_with_items);*/
		
	/*} else {*/
		/*stroybat_data_get_for_parent(DATABASE, datatype, 0, NULL, gtroybat_fill_items_list_with_items);*/
	/*}*/
}

static void 
data_view_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer userdata){
	g_print("Row activated\n");
	GObject *delegate = userdata;	

	GtkTreeModel *model = gtk_tree_view_get_model(treeview);
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter(model, &iter, path)) {
		int id;
		gtk_tree_model_get(model, &iter, DATA_ID, &id, -1); 
		
		if (!id) {
			g_print("No data in row\n");
			return;
		}
		
		if (gtk_tree_model_iter_has_child(model, &iter)){
			gtk_tree_view_expand_row(treeview, path, FALSE);
		} else {
			char *title, *kod, *SQL;
			gtk_tree_model_get(model, &iter, DATA_COLUMN_TITLE, &title, -1); 
			gtk_tree_model_get(model, &iter, DATA_COLUMN_KOD, &kod, -1); 

			/* find if there is such kod in zakazi */
			char count[10] = {0};
			SQL = STR("SELECT count FROM zakazi WHERE kod = '%s'", kod); 
			fprintf(stderr, "SQL: %s\n", SQL);
			sqlite_connect_get_string(SQL, DATABASE, count);
			fprintf(stderr, "COUNT S: %s\n", count);
				
			int c = atoi(count);
			fprintf(stderr, "COUNT C: %d\n", c);
			if (c){
				//increase count
				SQL = STR("UPDATE zakazi SET count = %d WHERE kod = '%s'", ++c,  kod); 
				fprintf(stderr, "SQL: %s\n", SQL);
				sqlite_connect_execute(SQL, DATABASE);
			} else {
				// create new
				if (add_zakaz(title, 1, kod) == 0){
					g_print("can't add zakaz: %s: %s\n", title, kod);
					return;	
				}
			}

			zakazi_update(delegate);
		}
	}
}

/*void data_view_list_changed(GtkWidget *widget, gpointer user_data){*/
	/*GtkTreeStore *store = user_data;*/

	/*GtkEntryBuffer *buffer = gtk_entry_get_buffer(GTK_ENTRY(widget));*/
	/*const char *search = gtk_entry_buffer_get_text(buffer);*/
	/*g_print("Search has changed to: %s\n", search);*/

	/*if (strlen(search) > 2) {*/
		/*gstroybat_items_list_view_store_update(search, store, datatype);	*/
	/*}*/

	/*if (strlen(search) == 0) {*/
		/*search = NULL;*/
		/*gstroybat_items_list_view_store_update(search, store, datatype);	*/
	/*}*/
/*}*/


static void 
data_view_new(GtkWidget *mainWindow){

	GtkWidget * treeView = lookup_widget(mainWindow, "dataTreeView");
	if (!treeView){
		g_print("Error! Can't find dataTreeView\n");
		return;
	}	

	/*GtkWidget *search = gtk_entry_new();*/
	/*gtk_entry_set_placeholder_text(GTK_ENTRY(search), "Поиск: название");*/
	/*g_signal_connect (search, "changed", G_CALLBACK (gstroybat_smeta_items_list_changed), itemsListViewStore);*/
	/*g_signal_connect (search, "insert-at-cursor", G_CALLBACK (gstroybat_smeta_items_list_changed), itemsListViewStore);	*/

	//gtk_tree_view_set_search_entry(GTK_TREE_VIEW(treeView), search);
	//gtk_tree_view_set_enable_search(GTK_TREE_VIEW(treeView), true);
	
	DataViewStrore = data_view_table_model_new();
	data_view_store_update(NULL, DataViewStrore);

	gtk_tree_view_set_model(GTK_TREE_VIEW(treeView), GTK_TREE_MODEL(DataViewStrore));
	g_signal_connect(treeView, "row-activated", (GCallback) data_view_row_activated, mainWindow);

	const char *column_titles[] = {"Наименование", "Код"};

	int i;
	for (i = 0; i < 2; ++i) {
		
		GtkCellRenderer	*renderer = gtk_cell_renderer_text_new();
		g_object_set(renderer, "wrap-mode", PANGO_WRAP_WORD, NULL);
		g_object_set(renderer, "wrap-width", 300, NULL);	
		g_object_set_data(G_OBJECT(renderer), "column_number", GUINT_TO_POINTER(i));
		
		GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(column_titles[i], renderer, "text", i,  NULL);
		switch (i) {
			case DATA_COLUMN_TITLE:
				{
					gtk_cell_renderer_set_fixed_size(renderer, -1, 40);
					g_object_set(column, "expand", TRUE, NULL);	
					break;
				}			
			default: break;
		}
		g_object_set(column, "resizable", TRUE, NULL);	
		
		gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), column);	
	}

	gtk_widget_show(treeView);
}

#endif /* ifndef DATA_VIEW_H */
