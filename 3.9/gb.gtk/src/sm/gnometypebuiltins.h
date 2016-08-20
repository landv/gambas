


#ifndef __GNOMETYPEBUILTINS_H__
#define __GNOMETYPEBUILTINS_H__ 1

#include <glib-object.h>

G_BEGIN_DECLS


/* --- gnome-app-helper.h --- */
#define GNOME_TYPE_UI_INFO_TYPE gnome_ui_info_type_get_type()
GType gnome_ui_info_type_get_type (void);
#define GNOME_TYPE_UI_INFO_CONFIGURABLE_TYPES gnome_ui_info_configurable_types_get_type()
GType gnome_ui_info_configurable_types_get_type (void);
#define GNOME_TYPE_UI_PIXMAP_TYPE gnome_ui_pixmap_type_get_type()
GType gnome_ui_pixmap_type_get_type (void);

/* --- gnome-client.h --- */
#define GNOME_TYPE_INTERACT_STYLE gnome_interact_style_get_type()
GType gnome_interact_style_get_type (void);
#define GNOME_TYPE_DIALOG_TYPE gnome_dialog_type_get_type()
GType gnome_dialog_type_get_type (void);
#define GNOME_TYPE_SAVE_STYLE gnome_save_style_get_type()
GType gnome_save_style_get_type (void);
#define GNOME_TYPE_RESTART_STYLE gnome_restart_style_get_type()
GType gnome_restart_style_get_type (void);
#define GNOME_TYPE_CLIENT_STATE gnome_client_state_get_type()
GType gnome_client_state_get_type (void);
#define GNOME_TYPE_CLIENT_FLAGS gnome_client_flags_get_type()
GType gnome_client_flags_get_type (void);

/* --- gnome-dateedit.h --- */
#define GNOME_TYPE_DATE_EDIT_FLAGS gnome_date_edit_flags_get_type()
GType gnome_date_edit_flags_get_type (void);

/* --- gnome-druid-page-edge.h --- */
#define GNOME_TYPE_EDGE_POSITION gnome_edge_position_get_type()
GType gnome_edge_position_get_type (void);

/* --- gnome-font-picker.h --- */
#define GNOME_TYPE_FONT_PICKER_MODE gnome_font_picker_mode_get_type()
GType gnome_font_picker_mode_get_type (void);

/* --- gnome-icon-list.h --- */
#define GNOME_TYPE_ICON_LIST_MODE gnome_icon_list_mode_get_type()
GType gnome_icon_list_mode_get_type (void);

/* --- gnome-icon-lookup.h --- */
#define GNOME_TYPE_ICON_LOOKUP_FLAGS gnome_icon_lookup_flags_get_type()
GType gnome_icon_lookup_flags_get_type (void);
#define GNOME_TYPE_ICON_LOOKUP_RESULT_FLAGS gnome_icon_lookup_result_flags_get_type()
GType gnome_icon_lookup_result_flags_get_type (void);

/* --- gnome-mdi.h --- */
#define GNOME_TYPE_MDI_MODE gnome_mdi_mode_get_type()
GType gnome_mdi_mode_get_type (void);

/* --- gnome-password-dialog.h --- */
#define GNOME_TYPE_PASSWORD_DIALOG_REMEMBER gnome_password_dialog_remember_get_type()
GType gnome_password_dialog_remember_get_type (void);

/* --- gnome-thumbnail.h --- */
#define GNOME_TYPE_THUMBNAIL_SIZE gnome_thumbnail_size_get_type()
GType gnome_thumbnail_size_get_type (void);

/* --- gnome-types.h --- */
#define GNOME_TYPE_PREFERENCES_TYPE gnome_preferences_type_get_type()
GType gnome_preferences_type_get_type (void);
G_END_DECLS

#endif /* __GNOMETYPEBUILTINS_H__ */



