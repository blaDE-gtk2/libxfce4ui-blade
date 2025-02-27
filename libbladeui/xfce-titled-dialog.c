/*
 * Copyright (c) 2006 Benedikt Meurer <benny@xfce.org>.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

/**
 * SECTION:xfce-titled-dialog
 * @title: XfceTitledDialog
 * @short_description: A titled dialog window
 * @stability: Stable
 * @include: libbladeui/libbladeui.h
 *
 * #XfceTitledDialog is a titled dialog window supporting an optional
 * subtitle and mixed or pixbuf buttons.
 **/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#include <gdk/gdkkeysyms.h>

#include <libbladeui/xfce-heading.h>
#include <libbladeui/xfce-titled-dialog.h>
#include <libbladeui/libbladeui-private.h>
#include <libbladeui/libbladeui-alias.h>
#include <libbladeui/xfce-gtk-extensions.h>


#define XFCE_TITLED_DIALOG_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), XFCE_TYPE_TITLED_DIALOG, XfceTitledDialogPrivate))



/* Property identifiers */
enum
{
  PROP_0,
  PROP_SUBTITLE,
};



static void xfce_titled_dialog_finalize       (GObject                *object);
static void xfce_titled_dialog_get_property   (GObject                *object,
                                               guint                   prop_id,
                                               GValue                 *value,
                                               GParamSpec             *pspec);
static void xfce_titled_dialog_set_property   (GObject                *object,
                                               guint                   prop_id,
                                               const GValue           *value,
                                               GParamSpec             *pspec);
static void xfce_titled_dialog_close          (GtkDialog              *dialog);
static void xfce_titled_dialog_update_heading (XfceTitledDialog       *titled_dialog);



struct _XfceTitledDialogPrivate
{
  GtkWidget *heading;
  gchar     *subtitle;
};



G_DEFINE_TYPE (XfceTitledDialog, xfce_titled_dialog, GTK_TYPE_DIALOG)



static void
xfce_titled_dialog_class_init (XfceTitledDialogClass *klass)
{
  GtkDialogClass *gtkdialog_class;
  GtkBindingSet  *binding_set;
  GObjectClass   *gobject_class;

  /* add our private data to the class */
  g_type_class_add_private (klass, sizeof (XfceTitledDialogPrivate));

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = xfce_titled_dialog_finalize;
  gobject_class->get_property = xfce_titled_dialog_get_property;
  gobject_class->set_property = xfce_titled_dialog_set_property;

  gtkdialog_class = GTK_DIALOG_CLASS (klass);
  gtkdialog_class->close = xfce_titled_dialog_close;

  /**
   * XfceTitledDialog:subtitle:
   *
   * The subtitle displayed below the main dialog title.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_SUBTITLE,
                                   g_param_spec_string ("subtitle",
                                                        "subtitle",
                                                        "subtitle",
                                                        NULL,
                                                        G_PARAM_READWRITE
                                                        | G_PARAM_STATIC_STRINGS));

  /* connect additional key bindings to the GtkDialog::close action signal */
  binding_set = gtk_binding_set_by_class (klass);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_w, GDK_CONTROL_MASK, "close", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_W, GDK_CONTROL_MASK, "close", 0);
}



static void
xfce_titled_dialog_init (XfceTitledDialog *titled_dialog)
{
  GtkWidget *line;
  GtkWidget *vbox;
  GtkWidget *content_area;

  /* connect the private data */
  titled_dialog->priv = XFCE_TITLED_DIALOG_GET_PRIVATE (titled_dialog);

  /* remove the main dialog box from the window */
  content_area = gtk_dialog_get_content_area (GTK_DIALOG (titled_dialog));
  g_object_ref (G_OBJECT (content_area));
  gtk_container_remove (GTK_CONTAINER (titled_dialog), content_area);

  /* add a new vbox w/o border to the main window */
#if GTK_CHECK_VERSION (3, 0, 0)
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
#else
  vbox = gtk_vbox_new (FALSE, 0);
#endif
  gtk_container_add (GTK_CONTAINER (titled_dialog), vbox);
  gtk_widget_show (vbox);

  /* add the heading to the window */
  titled_dialog->priv->heading = g_object_new (XFCE_TYPE_HEADING, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), titled_dialog->priv->heading, FALSE, FALSE, 0);
  gtk_widget_show (titled_dialog->priv->heading);

  /* add the separator between header and content */
#if GTK_CHECK_VERSION (3, 0, 0)
  line = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
#else
  line = gtk_hseparator_new ();
#endif
  gtk_box_pack_start (GTK_BOX (vbox), line, FALSE, FALSE, 0);
  gtk_widget_show (line);

  /* add the main dialog box to the new vbox */
  gtk_box_pack_start (GTK_BOX (vbox), content_area, TRUE, TRUE, 0);
  g_object_unref (G_OBJECT(content_area));

  /* make sure to update the heading whenever one of the relevant window properties changes */
  g_signal_connect (G_OBJECT (titled_dialog), "notify::icon", G_CALLBACK (xfce_titled_dialog_update_heading), NULL);
  g_signal_connect (G_OBJECT (titled_dialog), "notify::icon-name", G_CALLBACK (xfce_titled_dialog_update_heading), NULL);
  g_signal_connect (G_OBJECT (titled_dialog), "notify::title", G_CALLBACK (xfce_titled_dialog_update_heading), NULL);
}




static void
xfce_titled_dialog_finalize (GObject *object)
{
  XfceTitledDialog *titled_dialog = XFCE_TITLED_DIALOG (object);

  /* release the subtitle */
  g_free (titled_dialog->priv->subtitle);

  (*G_OBJECT_CLASS (xfce_titled_dialog_parent_class)->finalize) (object);
}



static void
xfce_titled_dialog_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  XfceTitledDialog *titled_dialog = XFCE_TITLED_DIALOG (object);

  switch (prop_id)
    {
    case PROP_SUBTITLE:
      g_value_set_string (value, xfce_titled_dialog_get_subtitle (titled_dialog));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
xfce_titled_dialog_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  XfceTitledDialog *titled_dialog = XFCE_TITLED_DIALOG (object);

  switch (prop_id)
    {
    case PROP_SUBTITLE:
      xfce_titled_dialog_set_subtitle (titled_dialog, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
xfce_titled_dialog_close (GtkDialog *dialog)
{
  GdkEvent *event;

  /* verify that the dialog is realized */
  if (G_LIKELY (gtk_widget_get_realized (GTK_WIDGET (dialog))))
    {
      /* send a delete event to the dialog */
      event = gdk_event_new (GDK_DELETE);
      event->any.window = g_object_ref (gtk_widget_get_window(GTK_WIDGET (dialog)));
      event->any.send_event = TRUE;
      gtk_main_do_event (event);
      gdk_event_free (event);
    }
}



static void
xfce_titled_dialog_update_heading (XfceTitledDialog *titled_dialog)
{
  /* update the heading properties using the window property values */
  _xfce_heading_set_icon (XFCE_HEADING (titled_dialog->priv->heading), gtk_window_get_icon (GTK_WINDOW (titled_dialog)));
  _xfce_heading_set_icon_name (XFCE_HEADING (titled_dialog->priv->heading), gtk_window_get_icon_name (GTK_WINDOW (titled_dialog)));
  _xfce_heading_set_title (XFCE_HEADING (titled_dialog->priv->heading), gtk_window_get_title (GTK_WINDOW (titled_dialog)));
}



/**
 * xfce_titled_dialog_new:
 *
 * Allocates a new #XfceTitledDialog instance.
 *
 * Return value: the newly allocated #XfceTitledDialog.
 **/
GtkWidget*
xfce_titled_dialog_new (void)
{
  return g_object_new (XFCE_TYPE_TITLED_DIALOG, NULL);
}



/**
 * xfce_titled_dialog_new_with_buttons:
 * @title             : (allow-none): title of the dialog, or %NULL.
 * @parent            : (allow-none): transient parent window of the dialog, or %NULL.
 * @flags             : from #GtkDialogFlags.
 * @first_button_text : (allow-none): stock ID or text to go in first, or %NULL.
 * @...               : response ID for the first button, then additional buttons, ending with %NULL.
 *
 * See the documentation of gtk_dialog_new_with_buttons() for details about the
 * parameters and the returned dialog.
 *
 * Return value: the newly allocated #XfceTitledDialog.
 **/
GtkWidget*
xfce_titled_dialog_new_with_buttons (const gchar    *title,
                                     GtkWindow      *parent,
                                     GtkDialogFlags  flags,
                                     const gchar    *first_button_text,
                                     ...)
{
  const gchar *button_text;
  GtkWidget   *dialog;
  va_list      args;
  gint         response_id;

  /* allocate the dialog */
  dialog = g_object_new (XFCE_TYPE_TITLED_DIALOG,
                         "destroy-with-parent", ((flags & GTK_DIALOG_DESTROY_WITH_PARENT) != 0),
                         "modal", ((flags & GTK_DIALOG_MODAL) != 0),
                         "title", title,
                         NULL);

  /* set the transient parent (if any) */
  if (G_LIKELY (parent != NULL))
    gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);

  /* add all additional buttons */
  va_start (args, first_button_text);
  for (button_text = first_button_text; button_text != NULL; )
    {
      response_id = va_arg (args, gint);
      gtk_dialog_add_button (GTK_DIALOG (dialog), button_text, response_id);
      button_text = va_arg (args, const gchar *);
    }
  va_end (args);

  return dialog;
}



/**
 * xfce_titled_dialog_new_with_mixed_buttons:
 * @title                  :  (allow-none):title of the dialog, or %NULL.
 * @parent                 : (allow-none): transient parent window of the dialog, or %NULL.
 * @flags                  : from #GtkDialogFlags.
 * @first_button_icon_name : icon name to go in first, or "" for no icon.
 * @first_button_text      : (allow-none): text to go in first, or %NULL.
 * @...                    : response ID for the first button, then additional buttons, ending with %NULL.
 *
 * Creates an #XfceTitledDialog using xfce_gtk_button_new_mixed. This allows
 * the buttons to use an optional named or stock icon.
 *
 * Return value: the newly allocated #XfceTitledDialog.
 *
 * Since: 4.14
 *
 **/
GtkWidget*
xfce_titled_dialog_new_with_mixed_buttons (const gchar    *title,
                                           GtkWindow      *parent,
                                           GtkDialogFlags  flags,
                                           const gchar    *first_button_icon_name,
                                           const gchar    *first_button_text,
                                           ...)
{
  const gchar *icon_name;
  const gchar *button_text;
  GtkWidget   *dialog;
  va_list      args;
  gint         response_id;

  /* allocate the dialog */
  dialog = g_object_new (XFCE_TYPE_TITLED_DIALOG,
                         "destroy-with-parent", ((flags & GTK_DIALOG_DESTROY_WITH_PARENT) != 0),
                         "modal", ((flags & GTK_DIALOG_MODAL) != 0),
                         "title", title,
                         NULL);

  /* set the transient parent (if any) */
  if (G_LIKELY (parent != NULL))
    gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);

  /* add all additional buttons */
  icon_name = first_button_icon_name;
  button_text = first_button_text;
  va_start (args, first_button_text);

  while (icon_name != NULL)
    {
      GtkWidget *button;

      /* response id comes after button text */
      response_id = va_arg (args, gint);

      /* build our button and add it */
      button = xfce_gtk_button_new_mixed (icon_name, button_text);
      gtk_widget_set_can_default (button, TRUE);

      gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, response_id);
      gtk_widget_show (button);

      /* this is to pickup for the next button.
       * The pattern is icon_name, button text
       */
      icon_name = va_arg (args, const gchar *);
      if (icon_name)
        {
          button_text = va_arg (args, const gchar *);
        }
    }
  va_end (args);

  return dialog;
}




/**
 * xfce_titled_dialog_get_subtitle:
 * @titled_dialog : a #XfceTitledDialog.
 *
 * Returns the subtitle of the @titled_dialog, or %NULL
 * if no subtitle is displayed in the @titled_dialog.
 *
 * Return value: the subtitle of @titled_dialog, or %NULL.
 **/
const gchar*
xfce_titled_dialog_get_subtitle (XfceTitledDialog *titled_dialog)
{
  g_return_val_if_fail (XFCE_IS_TITLED_DIALOG (titled_dialog), NULL);
  return titled_dialog->priv->subtitle;
}



/**
 * xfce_titled_dialog_set_subtitle:
 * @titled_dialog : a #XfceTitledDialog.
 * @subtitle      : the new subtitle for the @titled_dialog, or %NULL.
 *
 * Sets the subtitle displayed by @titled_dialog to @subtitle; if
 * @subtitle is %NULL no subtitle will be displayed by the @titled_dialog.
 **/
void
xfce_titled_dialog_set_subtitle (XfceTitledDialog *titled_dialog,
                                 const gchar      *subtitle)
{
  g_return_if_fail (XFCE_IS_TITLED_DIALOG (titled_dialog));
  g_return_if_fail (subtitle == NULL || g_utf8_validate (subtitle, -1, NULL));

  /* release the previous subtitle */
  g_free (titled_dialog->priv->subtitle);

  /* activate the new subtitle */
  titled_dialog->priv->subtitle = g_strdup (subtitle);

  /* update the subtitle for the heading */
  _xfce_heading_set_subtitle (XFCE_HEADING (titled_dialog->priv->heading), subtitle);

  /* notify listeners */
  g_object_notify (G_OBJECT (titled_dialog), "subtitle");
}



#define __XFCE_TITLED_DIALOG_C__
#include <libbladeui/libbladeui-aliasdef.c>
