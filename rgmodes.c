/*
    Replay Gain Control plugin for DeaDBeeF Player
    Copyright (C) 2014 Christian Boxdörfer

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <deadbeef/deadbeef.h>
#include <deadbeef/gtkui_api.h>

#define trace(...) { fprintf(stderr, __VA_ARGS__); }

#define     CONFSTR_ORDER_LINEAR            "rg_control.order_linear"
#define     CONFSTR_ORDER_SHUFFLE           "rg_control.order_shuffle"
#define     CONFSTR_ORDER_SHUFFLE_ALBUMS    "rg_control.order_shuffle_albums"
#define     CONFSTR_ORDER_RANDOM            "rg_control.order_random"

DB_functions_t *deadbeef;
static DB_misc_t plugin;
static ddb_gtkui_t *gtkui_plugin;

enum RG_MODE { NONE = 0, TRACK = 1, ALBUM = 2 };

static int CONFIG_ORDER_LINEAR = NONE;
static int CONFIG_ORDER_SHUFFLE = NONE;
static int CONFIG_ORDER_SHUFFLE_ALBUMS = NONE;
static int CONFIG_ORDER_RANDOM = NONE;

static gboolean
on_button_config (void *userdata);

static void
load_config (void)
{
    deadbeef->conf_lock ();
    CONFIG_ORDER_LINEAR = deadbeef->conf_get_int (CONFSTR_ORDER_LINEAR,                  ALBUM);
    CONFIG_ORDER_SHUFFLE = deadbeef->conf_get_int (CONFSTR_ORDER_SHUFFLE,                TRACK);
    CONFIG_ORDER_SHUFFLE_ALBUMS = deadbeef->conf_get_int (CONFSTR_ORDER_SHUFFLE_ALBUMS,  ALBUM);
    CONFIG_ORDER_RANDOM = deadbeef->conf_get_int (CONFSTR_ORDER_RANDOM,                  TRACK);
    deadbeef->conf_unlock ();
}

static void
save_config (void)
{
    deadbeef->conf_set_int (CONFSTR_ORDER_LINEAR,           CONFIG_ORDER_LINEAR);
    deadbeef->conf_set_int (CONFSTR_ORDER_SHUFFLE,          CONFIG_ORDER_SHUFFLE);
    deadbeef->conf_set_int (CONFSTR_ORDER_SHUFFLE_ALBUMS,   CONFIG_ORDER_SHUFFLE_ALBUMS);
    deadbeef->conf_set_int (CONFSTR_ORDER_RANDOM,           CONFIG_ORDER_RANDOM);
}

void
replaygain_control_set_replaygain (int replaygain_mode)
{
    deadbeef->conf_set_int ("replaygain_mode", replaygain_mode);
    deadbeef->sendmessage (DB_EV_CONFIGCHANGED, 0, 0, 0);
}


static int
replaygain_control_action_callback(DB_plugin_action_t *action, int ctx) {
    on_button_config (NULL);
    return 0;
}

static DB_plugin_action_t replaygain_control_action = {
    .title = "Edit/Advanced Replay Gain Settings",
    .name = "replaygain_control_conf",
    .flags = DB_ACTION_COMMON | DB_ACTION_ADD_MENU,
    .callback2 = replaygain_control_action_callback,
    .next = NULL,
};

static DB_plugin_action_t *
replaygain_control_getactions(DB_playItem_t *it) {
    return &replaygain_control_action;
}

void
replaygain_control_apply_settings (int playorder)
{
    switch (playorder) {
        case PLAYBACK_ORDER_LINEAR:
            replaygain_control_set_replaygain (CONFIG_ORDER_LINEAR);
            break;
        case PLAYBACK_ORDER_SHUFFLE_TRACKS:
            replaygain_control_set_replaygain (CONFIG_ORDER_SHUFFLE);
            break;
        case PLAYBACK_ORDER_SHUFFLE_ALBUMS:
            replaygain_control_set_replaygain (CONFIG_ORDER_SHUFFLE_ALBUMS);
            break;
        case PLAYBACK_ORDER_RANDOM:
            replaygain_control_set_replaygain (CONFIG_ORDER_RANDOM);
            break;
        default:
            break;
    }
}

gboolean
on_button_config (void *userdata)
{
    GtkWidget *replaygain_control_properties;
    GtkWidget *config_dialog;
    GtkWidget *vbox01;
    GtkWidget *valign[19];
    GtkWidget *replaygain_control_frame;
    GtkWidget *table;
    GtkWidget *default_label;
    GtkWidget *none_label;
    GtkWidget *track_label;
    GtkWidget *album_label;
    GtkWidget *random_label;
    GtkWidget *shuffle_tracks_label;
    GtkWidget *shuffle_albums_label;

    GtkWidget *order_linear_none;
    GtkWidget *order_linear_tracks;
    GtkWidget *order_linear_albums;

    GtkWidget *order_shuffle_tracks_none;
    GtkWidget *order_shuffle_tracks_track;
    GtkWidget *order_shuffle_tracks_album;

    GtkWidget *order_shuffle_albums_none;
    GtkWidget *order_shuffle_albums_track;
    GtkWidget *order_shuffle_albums_album;

    GtkWidget *order_random_none;
    GtkWidget *order_random_track;
    GtkWidget *order_random_album;

    GtkWidget *dialog_action_area13;
    GtkWidget *applybutton1;
    GtkWidget *cancelbutton1;
    GtkWidget *okbutton1;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    replaygain_control_properties = gtk_dialog_new ();
    gtk_window_set_title (GTK_WINDOW (replaygain_control_properties), "Relay Gain Properties");
    gtk_window_set_type_hint (GTK_WINDOW (replaygain_control_properties), GDK_WINDOW_TYPE_HINT_DIALOG);

    config_dialog = gtk_dialog_get_content_area (GTK_DIALOG (replaygain_control_properties));
    gtk_widget_show (config_dialog);

    vbox01 = gtk_vbox_new (FALSE, 8);
    gtk_widget_show (vbox01);
    gtk_box_pack_start (GTK_BOX (config_dialog), vbox01, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox01), 12);

    replaygain_control_frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type ((GtkFrame *)replaygain_control_frame, GTK_SHADOW_IN);
    gtk_widget_show (replaygain_control_frame);
    gtk_box_pack_start (GTK_BOX (vbox01), replaygain_control_frame, TRUE, FALSE, 0);

    table = gtk_table_new (5, 4, TRUE);
    gtk_widget_show (table);
    gtk_container_add (GTK_CONTAINER (replaygain_control_frame), table);
    gtk_table_set_col_spacings ((GtkTable *) table, 2);
    gtk_container_set_border_width (GTK_CONTAINER (table), 2);

    for (int i = 0; i < 4; i++) {
        valign[i] = gtk_alignment_new(1, 0.5, 0, 0);
        gtk_table_attach_defaults ((GtkTable *) table, valign[i], 0, 1, 1 + i, 2 + i);
        gtk_widget_show (valign[i]);
    }

    default_label = gtk_label_new ("Default:");
    gtk_widget_show (default_label);
    gtk_container_add(GTK_CONTAINER(valign[0]), default_label);

    shuffle_tracks_label = gtk_label_new ("Shuffle (tracks):");
    gtk_widget_show (shuffle_tracks_label);
    gtk_container_add(GTK_CONTAINER(valign[1]), shuffle_tracks_label);

    shuffle_albums_label = gtk_label_new ("Shuffle (albums):");
    gtk_widget_show (shuffle_albums_label);
    gtk_container_add(GTK_CONTAINER(valign[2]), shuffle_albums_label);

    random_label = gtk_label_new ("Random:");
    gtk_widget_show (random_label);
    gtk_container_add(GTK_CONTAINER(valign[3]), random_label);

    for (int i = 0; i < 15; i++) {
        valign[i+4] = gtk_alignment_new(0.5, 0.5, 0, 0);
        gtk_table_attach_defaults ((GtkTable *) table, valign[i+4], 1 + (i % 3), 2 + (i % 3), 0 + (i / 3), 1 + (i / 3));
        gtk_widget_show (valign[i+4]);
    }

    none_label = gtk_label_new ("None");
    gtk_widget_show (none_label);
    gtk_container_add(GTK_CONTAINER(valign[4]), none_label);

    track_label = gtk_label_new ("Track");
    gtk_widget_show (track_label);
    gtk_container_add(GTK_CONTAINER(valign[5]), track_label);

    album_label = gtk_label_new ("Album");
    gtk_widget_show (album_label);
    gtk_container_add(GTK_CONTAINER(valign[6]), album_label);

    // linear mode
    order_linear_none = gtk_radio_button_new (NULL);
    gtk_widget_show (order_linear_none);
    gtk_container_add(GTK_CONTAINER(valign[7]), order_linear_none);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (order_linear_none), FALSE);

    order_linear_tracks = gtk_radio_button_new_from_widget ((GtkRadioButton *)order_linear_none);
    gtk_widget_show (order_linear_tracks);
    gtk_container_add(GTK_CONTAINER(valign[8]), order_linear_tracks);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (order_linear_tracks), FALSE);

    order_linear_albums = gtk_radio_button_new_from_widget ((GtkRadioButton *)order_linear_tracks);
    gtk_widget_show (order_linear_albums);
    gtk_container_add(GTK_CONTAINER(valign[9]), order_linear_albums);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (order_linear_albums), FALSE);

    // shuffle tracks
    order_shuffle_tracks_none = gtk_radio_button_new (NULL);
    gtk_widget_show (order_shuffle_tracks_none);
    gtk_container_add(GTK_CONTAINER(valign[10]), order_shuffle_tracks_none);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (order_shuffle_tracks_none), FALSE);

    order_shuffle_tracks_track = gtk_radio_button_new_from_widget ((GtkRadioButton *)order_shuffle_tracks_none);
    gtk_widget_show (order_shuffle_tracks_track);
    gtk_container_add(GTK_CONTAINER(valign[11]), order_shuffle_tracks_track);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (order_shuffle_tracks_track), FALSE);

    order_shuffle_tracks_album = gtk_radio_button_new_from_widget ((GtkRadioButton *)order_shuffle_tracks_track);
    gtk_widget_show (order_shuffle_tracks_album);
    gtk_container_add(GTK_CONTAINER(valign[12]), order_shuffle_tracks_album);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (order_shuffle_tracks_album), FALSE);

    // shuffle albums
    order_shuffle_albums_none = gtk_radio_button_new (NULL);
    gtk_widget_show (order_shuffle_albums_none);
    gtk_container_add(GTK_CONTAINER(valign[13]), order_shuffle_albums_none);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (order_shuffle_albums_none), FALSE);

    order_shuffle_albums_track = gtk_radio_button_new_from_widget ((GtkRadioButton *)order_shuffle_albums_none);
    gtk_widget_show (order_shuffle_albums_track);
    gtk_container_add(GTK_CONTAINER(valign[14]), order_shuffle_albums_track);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (order_shuffle_albums_track), FALSE);

    order_shuffle_albums_album = gtk_radio_button_new_from_widget ((GtkRadioButton *)order_shuffle_albums_track);
    gtk_widget_show (order_shuffle_albums_album);
    gtk_container_add(GTK_CONTAINER(valign[15]), order_shuffle_albums_album);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (order_shuffle_albums_album), FALSE);

    // random mode
    order_random_none = gtk_radio_button_new (NULL);
    gtk_widget_show (order_random_none);
    gtk_container_add(GTK_CONTAINER(valign[16]), order_random_none);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (order_random_none), FALSE);

    order_random_track = gtk_radio_button_new_from_widget ((GtkRadioButton *)order_random_none);
    gtk_widget_show (order_random_track);
    gtk_container_add(GTK_CONTAINER(valign[17]), order_random_track);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (order_random_track), FALSE);

    order_random_album = gtk_radio_button_new_from_widget ((GtkRadioButton *)order_random_track);
    gtk_widget_show (order_random_album);
    gtk_container_add(GTK_CONTAINER(valign[18]), order_random_album);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (order_random_album), FALSE);

    dialog_action_area13 = gtk_dialog_get_action_area (GTK_DIALOG (replaygain_control_properties));
    gtk_widget_show (dialog_action_area13);
    gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area13), GTK_BUTTONBOX_END);

    applybutton1 = gtk_button_new_from_stock ("gtk-apply");
    gtk_widget_show (applybutton1);
    gtk_dialog_add_action_widget (GTK_DIALOG (replaygain_control_properties), applybutton1, GTK_RESPONSE_APPLY);
    gtk_widget_set_can_default (applybutton1, TRUE);

    cancelbutton1 = gtk_button_new_from_stock ("gtk-cancel");
    gtk_widget_show (cancelbutton1);
    gtk_dialog_add_action_widget (GTK_DIALOG (replaygain_control_properties), cancelbutton1, GTK_RESPONSE_CANCEL);
    gtk_widget_set_can_default (cancelbutton1, TRUE);

    okbutton1 = gtk_button_new_from_stock ("gtk-ok");
    gtk_widget_show (okbutton1);
    gtk_dialog_add_action_widget (GTK_DIALOG (replaygain_control_properties), okbutton1, GTK_RESPONSE_OK);
    gtk_widget_set_can_default (okbutton1, TRUE);

    switch (CONFIG_ORDER_LINEAR) {
    case NONE:
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (order_linear_none), TRUE);
        break;
    case TRACK:
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (order_linear_tracks), TRUE);
        break;
    case ALBUM:
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (order_linear_albums), TRUE);
        break;
    }

    switch (CONFIG_ORDER_SHUFFLE) {
    case NONE:
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (order_shuffle_tracks_none), TRUE);
        break;
    case TRACK:
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (order_shuffle_tracks_track), TRUE);
        break;
    case ALBUM:
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (order_shuffle_tracks_album), TRUE);
        break;
    }

    switch (CONFIG_ORDER_SHUFFLE_ALBUMS) {
    case NONE:
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (order_shuffle_albums_none), TRUE);
        break;
    case TRACK:
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (order_shuffle_albums_track), TRUE);
        break;
    case ALBUM:
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (order_shuffle_albums_album), TRUE);
        break;
    }

    switch (CONFIG_ORDER_RANDOM) {
    case NONE:
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (order_random_none), TRUE);
        break;
    case TRACK:
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (order_random_track), TRUE);
        break;
    case ALBUM:
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (order_random_album), TRUE);
        break;
    }

    for (;;) {
        int response = gtk_dialog_run (GTK_DIALOG (replaygain_control_properties));
        if (response == GTK_RESPONSE_OK || response == GTK_RESPONSE_APPLY) {
            if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (order_linear_none)) == TRUE) {
                CONFIG_ORDER_LINEAR = NONE;
            }
            else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (order_linear_tracks)) == TRUE) {
                CONFIG_ORDER_LINEAR = TRACK;
            }
            else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (order_linear_albums)) == TRUE) {
                CONFIG_ORDER_LINEAR = ALBUM;
            }

            if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (order_shuffle_tracks_none)) == TRUE) {
                CONFIG_ORDER_SHUFFLE = NONE;
            }
            else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (order_shuffle_tracks_track)) == TRUE) {
                CONFIG_ORDER_SHUFFLE = TRACK;
            }
            else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (order_shuffle_tracks_album)) == TRUE) {
                CONFIG_ORDER_SHUFFLE = ALBUM;
            }

            if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (order_shuffle_albums_none)) == TRUE) {
                CONFIG_ORDER_SHUFFLE_ALBUMS = NONE;
            }
            else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (order_shuffle_albums_track)) == TRUE) {
                CONFIG_ORDER_SHUFFLE_ALBUMS = TRACK;
            }
            else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (order_shuffle_albums_album)) == TRUE) {
                CONFIG_ORDER_SHUFFLE_ALBUMS = ALBUM;
            }

            if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (order_random_none)) == TRUE) {
                CONFIG_ORDER_RANDOM = NONE;
            }
            else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (order_random_track)) == TRUE) {
                CONFIG_ORDER_RANDOM = TRACK;
            }
            else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (order_random_album)) == TRUE) {
                CONFIG_ORDER_RANDOM = ALBUM;
            }

            save_config ();

            int playorder = deadbeef->conf_get_int ("playback.order", 0);
            replaygain_control_apply_settings (playorder);

            deadbeef->sendmessage (DB_EV_CONFIGCHANGED, 0, 0, 0);
        }
        if (response == GTK_RESPONSE_APPLY) {
            continue;
        }
        break;
    }
    gtk_widget_destroy (replaygain_control_properties);
#pragma GCC diagnostic pop
    return FALSE;
}

static int playorder_old = -1;

static int
replaygain_control_message (uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2)
{
    int playorder;
    switch (id) {
        case DB_EV_CONFIGCHANGED:
            playorder = deadbeef->conf_get_int ("playback.order", 0);
            if (playorder_old == playorder) {
                break;
            }
            else {
                playorder_old = playorder;
            }
            replaygain_control_apply_settings (playorder);
            break;
    }
    return 0;
}

int
replaygain_control_start (void)
{
    load_config ();
    return 0;
}

int
replaygain_control_connect() {
    gtkui_plugin = (ddb_gtkui_t *)deadbeef->plug_get_for_id (DDB_GTKUI_PLUGIN_ID);
    if (!gtkui_plugin) {
        fprintf (stderr, "replaygain control: can't find gtkui plugin\n");
        return -1;
    }
    return 0;
}

static DB_misc_t plugin = {
    .plugin.type = DB_PLUGIN_MISC,
    .plugin.api_vmajor = 1,
    .plugin.api_vminor = 5,
    .plugin.version_major = 1,
    .plugin.version_minor = 0,
#if GTK_CHECK_VERSION(3,0,0)
    .plugin.id = "replaygain_control_gtk3",
    .plugin.name = "Replay Gain Control GTK3 UI",
#else
    .plugin.id = "replaygain_control_gtk2",
    .plugin.name = "Replay Gain Control GTK2 UI",
#endif
    .plugin.descr = "Replay Gain Control plugin",
    .plugin.copyright =
        "Replay Gain Control for DeaDBeeF Player\n"
        "Copyright (C) 2014 Christian Boxdörfer\n"
        "\n"
        "This program is free software; you can redistribute it and/or\n"
        "modify it under the terms of the GNU General Public License\n"
        "as published by the Free Software Foundation; either version 2\n"
        "of the License, or (at your option) any later version.\n"
        "\n"
        "This program is distributed in the hope that it will be useful,\n"
        "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
        "GNU General Public License for more details.\n"
        "\n"
        "You should have received a copy of the GNU General Public License\n"
        "along with this program; if not, write to the Free Software\n"
        "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n",
    .plugin.website = "",
    .plugin.get_actions = replaygain_control_getactions,
    .plugin.start = replaygain_control_start,
    .plugin.connect = replaygain_control_connect,
    .plugin.message = replaygain_control_message,
};

DB_plugin_t *
#if GTK_CHECK_VERSION(3,0,0)
ddb_misc_replaygain_control_GTK3_load (DB_functions_t *api) {
#else
ddb_misc_replaygain_control_GTK2_load (DB_functions_t *api) {
#endif
    deadbeef = api;
    return DB_PLUGIN(&plugin);
}
