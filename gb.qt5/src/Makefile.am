COMPONENT = gb.qt5
include $(top_srcdir)/component.am
include $(top_srcdir)/gb.qt.am

SUBDIRS = . @QT5WEBKIT_DIR@ @QT5OPENGL_DIR@ @QT5EXT_DIR@

gblib_LTLIBRARIES = gb.qt5.la

gb_qt5_la_LIBADD = @THREAD_LIB@ @QT5_LIB@
gb_qt5_la_LDFLAGS = -module @LD_FLAGS@ @QT5_LDFLAGS@
gb_qt5_la_CXXFLAGS = @THREAD_INC@ -DGB_QT_COMPONENT $(AM_CXXFLAGS) -std=c++11
gb_qt5_la_CPPFLAGS = @QT5_INC@ -I$(top_srcdir)/share/

gb_qt5_la_SOURCES = \
 x11.h x11.c \
 desktop.h desktop.c \
 gb.qt.h main.h main_moc.cpp main.cpp \
 CFont.h CFont.cpp \
 CScreen.h CScreen.cpp \
 CStyle.h CStyle.cpp \
 CWidget.h CWidget_moc.cpp CWidget.cpp \
 CWindow.h CWindow_moc.cpp CWindow.cpp \
 CButton.h CButton_moc.cpp CButton.cpp \
 CContainer.h CContainer_moc.cpp CContainer.cpp \
 CLabel.h CLabel_moc.cpp CLabel.cpp \
 CTextBox.h CTextBox_moc.cpp CTextBox.cpp \
 CMenu.h CMenu_moc.cpp CMenu.cpp \
 CMouse.h CMouse_moc.cpp CMouse.cpp \
 CKey.h CKey_moc.cpp CKey.cpp \
 CColor.h CColor_moc.cpp CColor.cpp \
 CConst.h CConst.cpp \
 CCheckBox.h CCheckBox_moc.cpp CCheckBox.cpp \
 CFrame.h CFrame_moc.cpp CFrame.cpp \
 CPanel.h CPanel_moc.cpp CPanel.cpp \
 CRadioButton.h CRadioButton_moc.cpp CRadioButton.cpp \
 CTextArea.h CTextArea_moc.cpp CTextArea.cpp \
 CTabStrip.h CTabStrip_moc.cpp CTabStrip.cpp \
 CDialog.h CDialog_moc.cpp CDialog.cpp \
 CPicture.h CPicture_moc.cpp CPicture.cpp \
 CImage.h CImage_moc.cpp CImage.cpp \
 CClipboard.h CClipboard_moc.cpp CClipboard.cpp \
 CDraw.h CDraw.cpp \
 cpaint_impl.h cpaint_impl.cpp \
 CWatch.h CWatch_moc.cpp CWatch.cpp \
 CDrawingArea.h CDrawingArea_moc.cpp CDrawingArea.cpp \
 CSlider.h CSlider_moc.cpp CSlider.cpp \
 CScrollBar.h CScrollBar_moc.cpp CScrollBar.cpp \
 CMovieBox.h CMovieBox_moc.cpp CMovieBox.cpp \
 ctrayicon.h ctrayicon_moc.cpp ctrayicon.cpp \
 CWatcher.h CWatcher_moc.cpp CWatcher.cpp \
 cprinter.h cprinter_moc.cpp cprinter.cpp \
 csvgimage.h csvgimage_moc.cpp csvgimage.cpp \
 fix_breeze.h fix_breeze.cpp

