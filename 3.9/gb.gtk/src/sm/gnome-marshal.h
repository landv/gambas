
#ifndef ___gnome_marshal_MARSHAL_H__
#define ___gnome_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* BOOLEAN:INT,ENUM,BOOLEAN,ENUM,BOOLEAN (./gnome-marshal.list:1) */
extern void _gnome_marshal_BOOLEAN__INT_ENUM_BOOLEAN_ENUM_BOOLEAN (GClosure     *closure,
                                                                   GValue       *return_value,
                                                                   guint         n_param_values,
                                                                   const GValue *param_values,
                                                                   gpointer      invocation_hint,
                                                                   gpointer      marshal_data);

/* BOOLEAN:INT,STRING (./gnome-marshal.list:2) */
extern void _gnome_marshal_BOOLEAN__INT_STRING (GClosure     *closure,
                                                GValue       *return_value,
                                                guint         n_param_values,
                                                const GValue *param_values,
                                                gpointer      invocation_hint,
                                                gpointer      marshal_data);

/* BOOLEAN:OBJECT (./gnome-marshal.list:3) */
extern void _gnome_marshal_BOOLEAN__OBJECT (GClosure     *closure,
                                            GValue       *return_value,
                                            guint         n_param_values,
                                            const GValue *param_values,
                                            gpointer      invocation_hint,
                                            gpointer      marshal_data);

/* BOOLEAN:VOID (./gnome-marshal.list:4) */
extern void _gnome_marshal_BOOLEAN__VOID (GClosure     *closure,
                                          GValue       *return_value,
                                          guint         n_param_values,
                                          const GValue *param_values,
                                          gpointer      invocation_hint,
                                          gpointer      marshal_data);

/* INT:VOID (./gnome-marshal.list:5) */
extern void _gnome_marshal_INT__VOID (GClosure     *closure,
                                      GValue       *return_value,
                                      guint         n_param_values,
                                      const GValue *param_values,
                                      gpointer      invocation_hint,
                                      gpointer      marshal_data);

/* VOID:ENUM,BOOLEAN (./gnome-marshal.list:6) */
extern void _gnome_marshal_VOID__ENUM_BOOLEAN (GClosure     *closure,
                                               GValue       *return_value,
                                               guint         n_param_values,
                                               const GValue *param_values,
                                               gpointer      invocation_hint,
                                               gpointer      marshal_data);

/* VOID:INT,BOXED (./gnome-marshal.list:7) */
extern void _gnome_marshal_VOID__INT_BOXED (GClosure     *closure,
                                            GValue       *return_value,
                                            guint         n_param_values,
                                            const GValue *param_values,
                                            gpointer      invocation_hint,
                                            gpointer      marshal_data);

/* VOID:OBJECT (./gnome-marshal.list:8) */
#define _gnome_marshal_VOID__OBJECT	g_cclosure_marshal_VOID__OBJECT

/* VOID:UINT,UINT,UINT,UINT (./gnome-marshal.list:9) */
extern void _gnome_marshal_VOID__UINT_UINT_UINT_UINT (GClosure     *closure,
                                                      GValue       *return_value,
                                                      guint         n_param_values,
                                                      const GValue *param_values,
                                                      gpointer      invocation_hint,
                                                      gpointer      marshal_data);

G_END_DECLS

#endif /* ___gnome_marshal_MARSHAL_H__ */

