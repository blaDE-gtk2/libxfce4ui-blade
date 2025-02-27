/*
 * Copyright (c) 2007 The Xfce Development Team
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

#if !defined (LIBBLADEUI_INSIDE_LIBBLADEUI_H) && !defined (LIBBLADEUI_COMPILATION)
#error "Only <libbladeui/libbladeui.h> can be included directly, this file is not part of the public API."
#endif

#ifndef __XFCE_GDK_EXTENSIONS_H__
#define __XFCE_GDK_EXTENSIONS_H__

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdk.h>

G_BEGIN_DECLS

GdkScreen    *xfce_gdk_screen_get_active   (gint *monitor_return);

GdkRectangle *xfce_gdk_screen_get_geometry (void);

G_END_DECLS

#endif /* !__XFCE_GDK_EXTENSIONS_H__ */
