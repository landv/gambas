COMPONENT = gb.qte
include $(top_srcdir)/component.am
include $(top_srcdir)/gb.qte.am

SUBDIRS = share ext

INCLUDES = @QTE_INC@ -I$(top_srcdir)/src/share

gblib_LTLIBRARIES = gb.qte.la

gb_qte_la_LIBADD = @QTE_LIB@
gb_qte_la_LDFLAGS = -module @LD_FLAGS@
gb_qte_la_CXXFLAGS = -fno-rtti -DQWS -DGB_QT_COMPONENT $(AM_CXXFLAGS)

gb_qte_la_SOURCES = \
 gb.qt.h main.h moc_main.cpp main.cpp \
 CFont.h CFont.cpp \
 CScreen.h CScreen.cpp \
 CWidget.h moc_CWidget.cpp CWidget.cpp \
 CWindow.h moc_CWindow.cpp CWindow.cpp \
 CButton.h moc_CButton.cpp CButton.cpp \
 CContainer.h moc_CContainer.cpp CContainer.cpp \
 CLabel.h moc_CLabel.cpp CLabel.cpp \
 CListBox.h moc_CListBox.cpp CListBox.cpp \
 CTextBox.h moc_CTextBox.cpp CTextBox.cpp \
 CPictureBox.h moc_CPictureBox.cpp CPictureBox.cpp \
 CMenu.h moc_CMenu.cpp CMenu.cpp \
 CMouse.h moc_CMouse.cpp CMouse.cpp \
 CKey.h moc_CKey.cpp CKey.cpp \
 CColor.h moc_CColor.cpp CColor.cpp \
 CConst.h CConst.cpp \
 CCheckBox.h moc_CCheckBox.cpp CCheckBox.cpp \
 CFrame.h moc_CFrame.cpp CFrame.cpp \
 CPanel.h moc_CPanel.cpp CPanel.cpp \
 CRadioButton.h moc_CRadioButton.cpp CRadioButton.cpp \
 CTreeView.h moc_CTreeView.cpp CTreeView.cpp \
 CGridView.h moc_CGridView.cpp CGridView.cpp \
 CTextArea.h moc_CTextArea.cpp CTextArea.cpp \
 CTabStrip.h moc_CTabStrip.cpp CTabStrip.cpp \
 CDialog.h moc_CDialog.cpp CDialog.cpp \
 CPicture.h moc_CPicture.cpp CPicture.cpp \
 CImage.h moc_CImage.cpp CImage.cpp \
 CDrawing.h moc_CDrawing.cpp CDrawing.cpp \
 CClipboard.h moc_CClipboard.cpp CClipboard.cpp \
 CDraw.h CDraw.cpp \
 CWatch.h moc_CWatch.cpp CWatch.cpp \
 CScrollView.h moc_CScrollView.cpp CScrollView.cpp \
 CDrawingArea.h moc_CDrawingArea.cpp CDrawingArea.cpp \
 CProgress.h moc_CProgress.cpp CProgress.cpp \
 CMessage.h moc_CMessage.cpp CMessage.cpp \
 CPrinter.h moc_CPrinter.cpp CPrinter.cpp \
 CIconView.h moc_CIconView.cpp CIconView.cpp \
 CSlider.h moc_CSlider.cpp CSlider.cpp \
 CScrollBar.h moc_CScrollBar.cpp CScrollBar.cpp \
 CMovieBox.h moc_CMovieBox.cpp CMovieBox.cpp \
 CSpinBox.h moc_CSpinBox.cpp CSpinBox.cpp \
 CSplitter.h moc_CSplitter.cpp CSplitter.cpp \
 CWatcher.h moc_CWatcher.cpp CWatcher.cpp
 
