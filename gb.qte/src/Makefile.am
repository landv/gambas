COMPONENT = gb.qte
include $(top_srcdir)/component.am
include $(top_srcdir)/gb.qte.am

SUBDIRS = share ext

INCLUDES = @QTE_INC@ -I$(top_srcdir)/share -I$(top_srcdir)/src/share

gblib_LTLIBRARIES = gb.qte.la

gb_qte_la_LIBADD = @QTE_LIB@
gb_qte_la_LDFLAGS = -module @LD_FLAGS@
gb_qte_la_CXXFLAGS = -fno-rtti -DQWS -DGB_QT_COMPONENT $(AM_CXXFLAGS)

gb_qte_la_SOURCES = \
 gb.qt.h main.h main_moc.cpp main.cpp \
 CFont.h CFont.cpp \
 CScreen.h CScreen.cpp \
 CWidget.h CWidget_moc.cpp CWidget.cpp \
 CWindow.h CWindow_moc.cpp CWindow.cpp \
 CButton.h CButton_moc.cpp CButton.cpp \
 CContainer.h CContainer_moc.cpp CContainer.cpp \
 CLabel.h CLabel_moc.cpp CLabel.cpp \
 CListBox.h CListBox_moc.cpp CListBox.cpp \
 CTextBox.h CTextBox_moc.cpp CTextBox.cpp \
 CPictureBox.h CPictureBox_moc.cpp CPictureBox.cpp \
 CMenu.h CMenu_moc.cpp CMenu.cpp \
 CMouse.h CMouse_moc.cpp CMouse.cpp \
 CKey.h CKey_moc.cpp CKey.cpp \
 CColor.h CColor_moc.cpp CColor.cpp \
 CConst.h CConst.cpp \
 CCheckBox.h CCheckBox_moc.cpp CCheckBox.cpp \
 CFrame.h CFrame_moc.cpp CFrame.cpp \
 CPanel.h CPanel_moc.cpp CPanel.cpp \
 CRadioButton.h CRadioButton_moc.cpp CRadioButton.cpp \
 CTreeView.h CTreeView_moc.cpp CTreeView.cpp \
 CGridView.h CGridView_moc.cpp CGridView.cpp \
 CTextArea.h CTextArea_moc.cpp CTextArea.cpp \
 CTabStrip.h CTabStrip_moc.cpp CTabStrip.cpp \
 CDialog.h CDialog_moc.cpp CDialog.cpp \
 CPicture.h CPicture_moc.cpp CPicture.cpp \
 CImage.h CImage_moc.cpp CImage.cpp \
 CClipboard.h CClipboard_moc.cpp CClipboard.cpp \
 CDraw.h CDraw.cpp \
 CWatch.h CWatch_moc.cpp CWatch.cpp \
 CScrollView.h CScrollView_moc.cpp CScrollView.cpp \
 CDrawingArea.h CDrawingArea_moc.cpp CDrawingArea.cpp \
 CProgress.h CProgress_moc.cpp CProgress.cpp \
 CMessage.h CMessage_moc.cpp CMessage.cpp \
 CIconView.h CIconView_moc.cpp CIconView.cpp \
 CSlider.h CSlider_moc.cpp CSlider.cpp \
 CScrollBar.h CScrollBar_moc.cpp CScrollBar.cpp \
 CMovieBox.h CMovieBox_moc.cpp CMovieBox.cpp \
 CSpinBox.h CSpinBox_moc.cpp CSpinBox.cpp \
 CSplitter.h CSplitter_moc.cpp CSplitter.cpp \
 CWatcher.h CWatcher_moc.cpp CWatcher.cpp
