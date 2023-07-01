/**
 * File              : zakazi_view.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 05.04.2023
 * Last Modified Date: 13.04.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef ZAKAZI_VIEW_h
#define ZAKAZI_VIEW_h

#include <gtk/gtk.h>
#include "zakaz.h"
#include "support.h"

enum {
  ZAKAZI_COLUMN_NUMBER,
  ZAKAZI_COLUMN_TITLE,
  ZAKAZI_COLUMN_COUNT,
  ZAKAZI_COLUMN_KOD,
  ZAKAZI_POINTER,
  ZAKAZI_N_COLUMNS
};

static GtkListStore *
zakazi_table_model_new(){
	GtkListStore *store = gtk_list_store_new(ZAKAZI_N_COLUMNS, 
			G_TYPE_UINT,   // number
			G_TYPE_STRING, // title
			G_TYPE_UINT,   // count
			G_TYPE_STRING, // kod
			G_TYPE_POINTER
	);

	return store;
}

static void 
zakazi_store_add(GtkListStore *store, struct zakaz * zakaz){
	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 
			ZAKAZI_COLUMN_NUMBER, zakaz->n,
			ZAKAZI_COLUMN_TITLE, zakaz->title,
			ZAKAZI_COLUMN_COUNT, zakaz->c,
			ZAKAZI_COLUMN_KOD,   zakaz->kod,
			ZAKAZI_POINTER,      zakaz,
	-1);
}

static int 
zakazi_fill_table(void *userdata, struct zakaz * zakaz){
	
	GObject *delegate   = userdata;
	GtkListStore *store = g_object_get_data(delegate, "zakaziStore");	
	
	int *ptr         = g_object_get_data(delegate, "number"); 
	int number = ++ptr[0];
	g_object_set_data(delegate, "number", ptr);
		
	struct zakaz *z = malloc(sizeof(struct zakaz));
	if (!z){
		g_print("can't allocate struct zakaz\n");	
		return 0;
	}
	z->c = zakaz->c;
	z->id = zakaz->id;
	z->n = number;
	strcpy(z->title, zakaz->title);
	strcpy(z->kod, zakaz->kod);

	zakazi_store_add(store, z);

	return 0;
}

static gboolean 
zakazi_table_model_free(GtkTreeModel* model, GtkTreePath* path, 
		GtkTreeIter* iter, gpointer data) 
{
	struct zakaz * zakaz;
	gtk_tree_model_get(model, iter, ZAKAZI_POINTER, &zakaz, -1);	
	free(zakaz);
	return FALSE;
}

static void 
zakazi_update(GObject * delegate){
	g_print("Update zakazi\n");
	GtkListStore *store = g_object_get_data(delegate, "zakaziStore");	
	int *ptr            = g_object_get_data(delegate, "number"); 
	ptr[0] = 0;

	/* set remove button insensitive */
	//gtk_widget_set_sensitive(GTK_WIDGET(g_object_get_data(delegate, "petientRemoveButton")), FALSE);		

	/* set selected zakaz to NULL */
	g_object_set_data(delegate, "selectedZakaz", NULL);	

	/* clear store */
	gtk_tree_model_foreach (GTK_TREE_MODEL(store), 
			zakazi_table_model_free, NULL);
	gtk_list_store_clear(store);
	
	/* get list of zakazi */
	get_zakazi(delegate, zakazi_fill_table);
}

static void 
zakazi_row_activated(
		GtkTreeView *treeview, 
		GtkTreePath *path, 
		GtkTreeViewColumn *col, 
		gpointer userdata
		)
{
	g_print("Row activated\n");

	GObject *delegate = userdata;

	/* set remove button insensitive */
	//gtk_widget_set_sensitive(GTK_WIDGET(g_object_get_data(app, "petientRemoveButton")), FALSE);		

	/* set selected zakaz to NULL */
	g_object_set_data(delegate, "selectedZakaz", NULL);

	GtkTreeModel *model;
	GtkTreeIter   iter;

	model = gtk_tree_view_get_model(treeview);

	if (gtk_tree_model_get_iter(model, &iter, path)) {
		struct zakaz * zakaz;
		gtk_tree_model_get(model, &iter, ZAKAZI_POINTER, &zakaz, -1); 			
		
		if (zakaz){
			/* set remove button sensitive */
			//gtk_widget_set_sensitive(GTK_WIDGET(g_object_get_data(app, "petientRemoveButton")), TRUE);		
				
			/* set selected patient */
			g_object_set_data(delegate, "selectedZakaz", zakaz);
		}
	}
}

static void 
zakazi_table_cell_edited_callback (
		GtkCellRendererText *cell, 
		gchar *path_string, 
		gchar *new_text, 
		gpointer user_data
		)
{
	guint column_number = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(cell), "column_number"));
	g_print("EDITED path: %s, col: %d\n", path_string, column_number);
	
	//get application delegate
	GObject *delegate = g_object_get_data(G_OBJECT(cell), "delegate");

	// we HAVE TO use GtkTreeView within gpointer!
	//  otherwise we could not differntiate the model type!
	GtkTreeView  *treeview  = GTK_TREE_VIEW(user_data);
	GtkTreeModel *treeModel = gtk_tree_view_get_model(treeview);

	// we need to use GtkListStore, because GtkTreeModel does not
	//  provide an interface for cell changing.
	GtkListStore *model;   
	GtkTreeIter iter_rawModel;

	// check if we're working on the raw model or on a sorted version
	//  of it
	if(GTK_IS_LIST_STORE(treeModel)){
		// just use the model as is    
		model = GTK_LIST_STORE(treeModel);

		// retrieve the iterator for the cell, that should be changed
		gtk_tree_model_get_iter_from_string((GtkTreeModel*)model, &iter_rawModel, path_string);

	} else { // we're working on a sorted model   
		// We need to change to a usual model.
		GtkTreeModelSort *sortedModel = GTK_TREE_MODEL_SORT(treeModel);
		model = GTK_LIST_STORE(gtk_tree_model_sort_get_model(sortedModel));

		// get the iterator within the sorted model
		GtkTreeIter iter_sortedModel;
		gtk_tree_model_get_iter_from_string((GtkTreeModel*)sortedModel, &iter_sortedModel, path_string);  

		// convert the iterator to one of the raw model.
		// (Otherwise the wrong cell will change)
		gtk_tree_model_sort_convert_iter_to_child_iter(sortedModel, &iter_rawModel, &iter_sortedModel);
    }

	struct zakaz * zakaz;
	gtk_tree_model_get(treeModel, &iter_rawModel, ZAKAZI_POINTER, &zakaz, -1); 			
	if (!zakaz){
		g_print("error to get zakaz - NULL\n");
		return;
	}
	
	switch (column_number) {
		case ZAKAZI_COLUMN_COUNT:
			{
				int count;
				sscanf(new_text, "%d", &count);	
				zakaz->c = count;
				if (set_zakaz(zakaz)){
					g_print("error to update zakaz\n");
					break;
				}
				
				gtk_list_store_set(
						GTK_LIST_STORE(model), &iter_rawModel, 
						column_number, count, -1);
				break;
			}
		default: break;
	}
}

static void 
zakazi_ask_to_remove_responce(
		GtkDialog *dialog, 
		gint responce, 
		gpointer userdata
		)
{
	if (responce == GTK_RESPONSE_DELETE_EVENT) {
		g_print("Remove commited\n");

		GObject *delegate = userdata;
		struct zakaz * zakaz = 
				g_object_get_data(delegate, "zakazToRemove"); 
		
		if (!zakaz){
			g_print("zakaz is NULL\n");
			gtk_widget_destroy(GTK_WIDGET(dialog));
			return;
		}		

		if (remove_zakaz(zakaz)){
			g_print("can't remove: %s\n", zakaz->title);
		}
		
		zakazi_update(delegate);
	}
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

static void 
zakazi_ask_to_remove(GObject *delegate, struct zakaz * zakaz) {
	if (!zakaz){
		g_print("zakaz is NULL\n");
		return;
	}
	
	g_object_set_data(delegate, "zakazToRemove", zakaz);
	
	char title[BUFSIZ] = "";
	sprintf(title, 
			"Удалить из базы: %s?", 
			zakaz->title
	);
	
	/*GtkWidget * mainWindow = GTK_WIDGET(delegate); */
	GtkWidget *dialog = gtk_message_dialog_new(NULL,
			GTK_DIALOG_MODAL,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_NONE,
			"%s", title);
	gtk_window_set_title(GTK_WINDOW(dialog), "Удалить?");

	//add remove button
	GtkWidget *button = gtk_button_new_with_label("УДАЛИТЬ");
	//GtkStyleContext *context = gtk_widget_get_style_context(button);
	//gtk_style_context_add_class(context, "destructive-action");
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_DELETE_EVENT);
	
	//add cancel button
	gtk_dialog_add_button(GTK_DIALOG(dialog), "Отмена", GTK_RESPONSE_CANCEL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);
	g_signal_connect (dialog, "response", 
			G_CALLBACK (zakazi_ask_to_remove_responce), delegate);

	gtk_widget_show_all(dialog);
}

static void
zakazi_on_remove_clicked(gpointer user_data){
	
	/* get treeView */
	GtkWidget * mainWindow = 
			lookup_widget(GTK_WIDGET(user_data), "mainWindow");
	if (!mainWindow){
		g_print("Error! Can't find mainWindow\n");
		return;
	}	
	GObject *delegate = G_OBJECT(mainWindow);	
	
	/* get treeView */
	GtkWidget * treeView = 
			lookup_widget(GTK_WIDGET(delegate), "zakaziTreeView");
	if (!treeView){
		g_print("Error! Can't find zakaziTreeView\n");
		return;
	}

	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeView));
	if (!selection){
		g_print("Error! Can't get selection\n");
		return;
	}
	
	GtkTreeModel *model = g_object_get_data(delegate, "zakaziStore"); 
	if (!model){
		g_print("Error! Can't get model\n");
		return;
	}	

	GList *rows = 
			gtk_tree_selection_get_selected_rows(selection, &model);
	if (!rows){
		g_print("Error! Can't get rows\n");
		return;
	}	

	GtkTreePath *path = rows->data;
	if (!path){
		g_print("Error! Can't get path\n");
		return;
	}	

	GtkTreeIter iter;
	if (gtk_tree_model_get_iter(model, &iter, path)) {
		struct zakaz * zakaz;
		gtk_tree_model_get(model, &iter, ZAKAZI_POINTER, &zakaz, -1); 			
		
		if (zakaz){
			zakazi_ask_to_remove(delegate, zakaz);
		}
	}
}

static void 
zakazi_ask_to_clear_responce(
		GtkDialog *dialog, 
		gint responce, 
		gpointer userdata
		)
{
	if (responce == GTK_RESPONSE_DELETE_EVENT) {
		g_print("Clear commited\n");

		GObject *delegate = userdata;
		
		if (sqlite_connect_execute("DELETE FROM zakazi", DATABASE)){
			g_print("can't clear xakazi\n");
		}
		
		zakazi_update(delegate);
	}
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

static void 
zakazi_ask_to_clear(GObject *delegate) {
	
	char title[BUFSIZ] = "Удалить все записи из базы:?";
	
	/*GtkWidget * mainWindow = GTK_WIDGET(delegate); */
	GtkWidget *dialog = gtk_message_dialog_new(NULL,
			GTK_DIALOG_MODAL,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_NONE,
			"%s", title);
	gtk_window_set_title(GTK_WINDOW(dialog), "Удалить?");

	//add remove button
	GtkWidget *button = gtk_button_new_with_label("УДАЛИТЬ");
	//GtkStyleContext *context = gtk_widget_get_style_context(button);
	//gtk_style_context_add_class(context, "destructive-action");
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_DELETE_EVENT);
	
	//add cancel button
	gtk_dialog_add_button(GTK_DIALOG(dialog), "Отмена", GTK_RESPONSE_CANCEL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);
	g_signal_connect (dialog, "response", 
			G_CALLBACK (zakazi_ask_to_clear_responce), delegate);

	gtk_widget_show_all(dialog);
}

static void
zakazi_on_clear_clicked(gpointer user_data){
	GtkWidget * mainWindow = 
			lookup_widget(GTK_WIDGET(user_data), "mainWindow");
	if (!mainWindow){
		g_print("Error! Can't find mainWindow\n");
		return;
	}	
	GObject *delegate = G_OBJECT(mainWindow);		

	zakazi_ask_to_clear(delegate);	
}

static GtkWidget *
zakazi_new(GtkWidget *mainWindow){
	/* set delegate */
	GObject *delegate = G_OBJECT(mainWindow);

	/* get treeView */
	GtkWidget * treeView = lookup_widget(mainWindow, "zakaziTreeView");
	if (!treeView){
		g_print("Error! Can't find zakaziTreeView\n");
		return NULL;
	}

	/* create new model */
	GtkListStore *store = zakazi_table_model_new();
	g_object_set_data(delegate, "zakaziStore", store);
	
	int *number = malloc(sizeof(int));
	if (!number){
		g_print("Error! Can't allocate memory\n");
		return NULL;
	}
	*number = 0;
	g_object_set_data(delegate, "number", number);

	zakazi_update(delegate);

	/* set tree model for view */
	//GtkWidget *view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeView), GTK_TREE_MODEL(store));

	g_signal_connect(treeView, "row-activated", 
			(GCallback) zakazi_row_activated, delegate);

	const char *column_titles[] = {
		"№ п/п", 
		"Наименование", 
		"Количество", 
		"Код" 
	};
	
	/* fill tableView */
	int i;
	for (i = 0; i < ZAKAZI_N_COLUMNS -1; ++i) {
		
		GtkCellRenderer	*renderer = gtk_cell_renderer_text_new();
		g_object_set(renderer, "editable", FALSE, NULL);
		g_object_set(renderer, "wrap-mode", PANGO_WRAP_WORD, NULL);
		g_object_set(renderer, "wrap-width", 60, NULL);	
		
		g_signal_connect(renderer, "edited", 
				(GCallback) zakazi_table_cell_edited_callback, treeView);
		
		g_object_set_data(G_OBJECT(renderer), "column_number", GUINT_TO_POINTER(i));
		g_object_set_data(G_OBJECT(renderer), "delegate", delegate);
		
		GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
				column_titles[i], renderer, "text", i,  NULL);
		switch (i) {
			case ZAKAZI_COLUMN_TITLE: 
				{
					gtk_cell_renderer_set_fixed_size(renderer, -1, 40);
					g_object_set(column, "expand", TRUE, NULL);	
					g_object_set(renderer, "wrap-width", 300, NULL);	
					break;
				}
			case ZAKAZI_COLUMN_COUNT:
				{
					g_object_set(renderer, "editable", TRUE, NULL);
					break;
				}	
			default: break;
		}
		g_object_set(column, "resizable", TRUE, NULL);	
		
		gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), column);	
	}

	gtk_widget_show(treeView);
	return treeView;	
}

#endif /* ifndef ZAKAZI_VIEW_h */
