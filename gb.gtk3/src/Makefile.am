COMPONENT = gb.gtk3
include $(top_srcdir)/component.am

##SUBDIRS = . @GTKOPENGL_DIR@

gblib_LTLIBRARIES = gb.gtk3.la

gb_gtk3_la_LIBADD = @GTK3_LIB@
gb_gtk3_la_LDFLAGS = -module @LD_FLAGS@ @GTK3_LDFLAGS@
gb_gtk3_la_CPPFLAGS = @GTK3_INC@ -I$(top_srcdir)/share -DGTK3 -DGTK_DISABLE_SINGLE_INCLUDES -DGSEAL_ENABLE

gb_gtk3_la_SOURCES = \
	gmemory.h gmemory.cpp gtag.h ggambastag.h gshare.h \
	gb.gtk.h widgets.h font-parser.h font-parser.cpp \
	gtools.cpp gcolor.h \
	gstock.cpp gfont.cpp gpicture.cpp \
	gdesktop.cpp gmessage.cpp \
	gcontrol.cpp gcontainer.cpp gseparator.cpp \
	gbutton.cpp gslider.cpp glabel.cpp gsignals.cpp \
	gpicturebox.cpp gcombobox.cpp gtextbox.cpp gtextarea.cpp \
	gspinbox.cpp \
	gframe.cpp gtabstrip.cpp gscrollview.cpp \
	gmenu.cpp gtrayicon.cpp gmainwindow.cpp \
	gtree.h gtree.cpp \
	watcher.h watcher.cpp \
	CStock.cpp CStock.h \
	CConst.h CConst.cpp CColor.h CColor.cpp \
	CFont.h CFont.cpp \
	CKey.h CKey.cpp \
	CScreen.h CScreen.cpp \
	CStyle.h CStyle.cpp \
	CMessage.h CMessage.cpp CDialog.h CDialog.cpp \
	CDraw.h CDraw.cpp \
	cpaint_impl.h cpaint_impl.cpp \
	CImage.h CImage.cpp \
	CPicture.h CPicture.cpp \
	CClipboard.h CClipboard.cpp \
	CMouse.h CMouse.cpp \
	CWatcher.h CWatcher.cpp \
	CWidget.h CWidget.cpp CContainer.h  CContainer.cpp \
	CSeparator.h CSeparator.cpp \
	CDrawingArea.h CDrawingArea.cpp \
	CLabel.h CLabel.cpp \
	CSlider.h CSlider.cpp \
	CButton.h CButton.cpp \
	CPictureBox.h CPictureBox.cpp \
	CTextBox.h CTextBox.cpp \
	CTextArea.h CTextArea.cpp \
	CSpinBox.h CSpinBox.cpp \
	CFrame.h CFrame.cpp \
	CTabStrip.h CTabStrip.cpp \
	CScrollView.h CScrollView.cpp \
	CMenu.h CMenu.cpp CTrayIcon.h CTrayIcon.cpp CWindow.h CWindow.cpp \
	cprinter.h cprinter.cpp \
	csvgimage.h csvgimage.cpp \
	main.h main.cpp \
	gkey.h gkey.cpp \
	gcursor.h gcursor.cpp \
	gmouse.h gmouse.cpp \
	gdesktop.h \
	gstock.h \
	gpicture.h \
	gfont.h \
	gmessage.h \
	gdialog.h \
	gcontrol.h \
	gseparator.h \
	gtrayicon.h \
	gplugin.h \
	glabel.h \
	gbutton.h \
	gmoviebox.h \
	gpicturebox.h \
	gspinbox.h \
	gcombobox.h \
	gtextbox.h \
	gtextarea.h \
	gslider.h \
	gscrollbar.h \
	gcontainer.h \
	gscrollview.h \
	gdrawingarea.h gdrawingarea.cpp \
	gtabstrip.h \
	gframe.h \
	gmenu.h \
	gmainwindow.h \
	gapplication.h gapplication.cpp \
	gclipboard.h \
	gdrag.h gdrag.cpp \
	gtools.h kentities.h \
	gprinter.h gprinter.cpp \
	gglarea.h gglarea.cpp \
	x11.h x11.c \
	sm/bonobo-macros.h \
	sm/gnome-macros.h \
	sm/gnome-uidefs.h \
	sm/libgnomeui.h \
	sm/libgnomeuiP.h \
	sm/gnome-client.h \
	sm/gnome-client.c \
	sm/gnome-ice.h \
	sm/gnome-ice.c \
	sm/gnome-marshal.h \
	sm/gnome-marshal.c \
	sm/gnometypebuiltins.h \
	sm/gnometypebuiltins.c \
	sm/sm.h
