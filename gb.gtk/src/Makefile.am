COMPONENT = gb.gtk
include $(top_srcdir)/component.am

SUBDIRS = ext

INCLUDES = @GTK_INC@ -I$(top_srcdir)/share

gblib_LTLIBRARIES = gb.gtk.la

gb_gtk_la_LIBADD = @GTK_LIB@
gb_gtk_la_LDFLAGS = -module @LD_FLAGS@ @GTK_LDFLAGS@

gb_gtk_la_SOURCES = \
	gmemory.h gmemory.cpp gtag.h ggambastag.h gshare.h \
	gb.gtk.h widgets.h widgets_private.h gtree.h font-parser.h font-parser.cpp \
	gtree.cpp gtools.cpp gcolor.h \
	gstock.cpp gfont.cpp gpicture.cpp \
	gdraw.cpp gdesktop.cpp gmessage.cpp \
	gcontrol.cpp gcontainer.cpp gseparator.cpp \
	gbutton.cpp gprogressbar.cpp gslider.cpp glabel.cpp gsignals.cpp \
	gpicturebox.cpp gcombobox.cpp gtextbox.cpp gtextarea.cpp glistbox.cpp \
	gspinbox.cpp ggridview.cpp gtreeview.cpp giconview.cpp \
	gframe.cpp gsplitter.cpp gtabstrip.cpp gscrollview.cpp \
	gmenu.cpp gtrayicon.cpp gmainwindow.cpp \
	watcher.h watcher.cpp \
	CStock.cpp CStock.h \
	CConst.h CConst.cpp CColor.h CColor.cpp \
	CFont.h CFont.cpp \
	CKey.h CKey.cpp \
	CScreen.h CScreen.cpp \
	CMessage.h CMessage.cpp CDialog.h CDialog.cpp \
	CDraw.h CDraw.cpp CPicture.h CPicture.cpp \
	CClipboard.h CClipboard.cpp \
	CMouse.h CMouse.cpp \
	CWatcher.h CWatcher.cpp \
	CWidget.h CWidget.cpp CContainer.h  CContainer.cpp \
	CSeparator.h CSeparator.cpp \
	CDrawingArea.h CDrawingArea.cpp \
	CLabel.h CLabel.cpp \
	CProgress.h CProgress.cpp \
	CSlider.h CSlider.cpp \
	CButton.h CButton.cpp \
	CPictureBox.h CPictureBox.cpp \
	CTextBox.h CTextBox.cpp \
	CTextArea.h CTextArea.cpp \
	CSpinBox.h CSpinBox.cpp \
	CListBox.h CListBox.cpp \
	tablerender.h tablerender.cpp CGridView.h CGridView.cpp \
	CTreeView.h CTreeView.cpp \
	CIconView.h CIconView.cpp \
	CFrame.h CFrame.cpp \
	CSplitter.h CSplitter.cpp \
	CTabStrip.h CTabStrip.cpp \
	CScrollView.h CScrollView.cpp \
	CMenu.h CMenu.cpp CTrayIcon.h CTrayIcon.cpp CWindow.h CWindow.cpp \
	main.h main.cpp \
	gkey.h \
	gcursor.h \
	gmouse.h gmouse.cpp \
	gdesktop.h \
	gdraw.h \
	gstock.h \
	gpicture.h \
	gfont.h \
	gmessage.h \
	gdialog.h \
	gcontrol.h \
	gseparator.h \
	gtrayicon.h \
	gplugin.h \
	gprogressbar.h \
	glabel.h \
	gbutton.h \
	gmoviebox.h \
	gpicturebox.h \
	ggridview.h \
	gtreeview.h \
	giconview.h \
	glistbox.h \
	gspinbox.h \
	gcombobox.h \
	gtextbox.h \
	gtextarea.h \
	gslider.h \
	gscrollbar.h \
	gcontainer.h \
	gsplitter.h \
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
	x11.h x11.c
