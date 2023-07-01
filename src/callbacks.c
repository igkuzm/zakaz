/**
 * File              : callbacks.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 05.04.2023
 * Last Modified Date: 13.04.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "zakazi_view.h"
#include "data_view.h"
#include "excel.h"
#include "openfile.h"

void
on_remove_clicked                      (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
	zakazi_on_remove_clicked(toolbutton);
}


void
on_clear_clicked                       (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
	zakazi_on_clear_clicked(toolbutton);
}

void
on_zakaz_clicked                       (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
	char *file = make_zakaz();
	openfile(file);
}


void
on_akt_clicked                         (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
	char *file = make_akt();
	openfile(file);
}

