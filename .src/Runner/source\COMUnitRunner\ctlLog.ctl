VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "COMDLG32.OCX"
Begin VB.UserControl ctlLog 
   ClientHeight    =   705
   ClientLeft      =   0
   ClientTop       =   0
   ClientWidth     =   7800
   ScaleHeight     =   705
   ScaleWidth      =   7800
   Begin VB.Frame fraMain 
      Caption         =   "COMUnit Log File"
      Height          =   675
      Left            =   0
      TabIndex        =   0
      Top             =   0
      Width           =   7755
      Begin VB.CommandButton cmdView 
         Caption         =   "&View"
         Height          =   315
         Left            =   6720
         TabIndex        =   4
         Top             =   240
         Width           =   940
      End
      Begin VB.TextBox txtLogFile 
         Height          =   315
         Left            =   1080
         TabIndex        =   2
         Top             =   240
         Width           =   5235
      End
      Begin VB.CommandButton cmdFileDlg 
         Caption         =   "..."
         Height          =   315
         Left            =   6360
         TabIndex        =   1
         Top             =   240
         Width           =   315
      End
      Begin VB.Label Label1 
         Caption         =   "Log File:"
         Height          =   255
         Left            =   180
         TabIndex        =   3
         Top             =   300
         Width           =   855
      End
   End
   Begin MSComDlg.CommonDialog cdlgFile 
      Left            =   1980
      Top             =   0
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
      CancelError     =   -1  'True
   End
End
Attribute VB_Name = "ctlLog"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

Private Sub UserControl_Show()
    txtLogFile.Text = ParseCommand("l")
End Sub

Public Property Let LogFile(strFile As String)
    txtLogFile.Text = strFile
End Property

Private Sub cmdFileDlg_Click()
    cdlgFile.FileName = txtLogFile.Text
    On Error GoTo CancelError
    cdlgFile.ShowOpen
    txtLogFile.Text = cdlgFile.FileName
    
CancelError:
End Sub

Public Sub WriteToLog(objResult As TestResult)
    If (txtLogFile.Text = vbNullString) Then
        Exit Sub
    End If
    
    On Error Resume Next
    Dim objWriter As New LogWriter
    objWriter.WriteToFile objResult, txtLogFile.Text
    
    If Err Then
        MsgBox "Unable to log the test results to the specified log file: " & _
            vbNewLine & txtLogFile.Text & vbNewLine & Err.Description
    End If
End Sub

Private Sub cmdView_Click()
    Dim strFile As String
    strFile = txtLogFile.Text
    If (InStr(0, strFile, "\") < 0) Then
        strFile = App.Path & "\" & strFile
    End If
    
    ' launch ie or notepad
    On Error Resume Next
    Shell "c:\winnt\notepad.exe" & strFile
    
    If (Err) Then
        MsgBox "Unable to view the specified log file: " & _
            vbNewLine & strFile & vbNewLine & Err.Description
    End If
End Sub

Private Sub UserControl_Resize()
    fraMain.Move 0, 0, ScaleWidth, ScaleHeight
    txtLogFile.Width = PosInt(fraMain.Width - txtLogFile.Left - cmdFileDlg.Width - cmdView.Width - 180)
    cmdView.Left = PosInt(fraMain.Width - cmdView.Width - 80)
    cmdFileDlg.Left = PosInt(fraMain.Width - cmdView.Width - cmdFileDlg.Width - 120)
End Sub
