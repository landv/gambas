VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "COMDLG32.OCX"
Begin VB.UserControl ctlParameter 
   ClientHeight    =   3945
   ClientLeft      =   0
   ClientTop       =   0
   ClientWidth     =   7950
   ScaleHeight     =   3945
   ScaleWidth      =   7950
   Begin VB.Frame fraParameters 
      Caption         =   "COMUnit Parameters"
      Height          =   3915
      Left            =   0
      TabIndex        =   0
      Top             =   0
      Width           =   7935
      Begin MSComctlLib.TreeView tvParameters 
         Height          =   3075
         Left            =   120
         TabIndex        =   4
         Top             =   720
         Width           =   7755
         _ExtentX        =   13679
         _ExtentY        =   5424
         _Version        =   393217
         Style           =   7
         Appearance      =   1
      End
      Begin MSComDlg.CommonDialog dlgFile 
         Left            =   4740
         Top             =   180
         _ExtentX        =   847
         _ExtentY        =   847
         _Version        =   393216
         DefaultExt      =   ".xml"
         DialogTitle     =   "Select Parameter File to Load"
      End
      Begin VB.TextBox txtFilename 
         Height          =   330
         Left            =   1440
         TabIndex        =   3
         Top             =   300
         Width           =   5835
      End
      Begin VB.CommandButton cmdFile 
         Caption         =   "..."
         Height          =   345
         Left            =   7440
         TabIndex        =   2
         Top             =   300
         Width           =   375
      End
      Begin VB.Label Label1 
         Caption         =   "Parameter File:"
         Height          =   255
         Left            =   120
         TabIndex        =   1
         Top             =   360
         Width           =   1455
      End
   End
End
Attribute VB_Name = "ctlParameter"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

Private m_colParameters As TestParameters

Private Sub UserControl_Show()
    Dim strInitFile As String
    strInitFile = ParseCommand("p")
    If (strInitFile <> "") Then
        LoadParameters strInitFile
    End If
End Sub

Private Sub UserControl_Terminate()
    Set m_colParameters = Nothing
End Sub

Public Property Get TestParameters() As TestParameters
    Set TestParameters = m_colParameters
End Property

Public Property Let ParameterFile(strFile As String)
    LoadParameters strFile
End Property

Private Sub cmdFile_Click()
    dlgFile.DialogTitle = "Select the COMUnit Parameter file to use"
    dlgFile.DefaultExt = "xml"
    dlgFile.Filter = "COMUnit Parameter File (*.xml)|*.xml"
    dlgFile.FileName = txtFilename.Text
    On Error GoTo CancelError
    dlgFile.ShowOpen
    
    ' load parameters
    LoadParameters dlgFile.FileName

CancelError:
End Sub

Private Sub LoadParameters(strFile As String)
    On Error GoTo ErrorHandler
    Dim objReader As New ParameterReader
    Set m_colParameters = objReader.ReadFromFile(strFile)
    On Error GoTo 0
    
    If Not (m_colParameters Is Nothing) Then
        tvParameters.Nodes.Clear
        
        Dim objNode As Node
        Set objNode = tvParameters.Nodes.Add(Text:="Parameters")
        objNode.Expanded = True

        DisplayParameters m_colParameters, objNode
    End If
    
    txtFilename.Text = strFile
    Exit Sub
    
ErrorHandler:
    MsgBox "Unable to load the parameters from the specified file: " & _
        vbNewLine & strFile & vbNewLine & Err.Description
End Sub

Private Sub DisplayParameters(colParameters As TestParameters, objRoot As Node)
    Dim objParameter As TestParameter
    For Each objParameter In colParameters
        Dim strText As String
        If (TypeName(objParameter.Value) = "String") Then
            strText = objParameter.Name & " = '" & objParameter.Value & "'"
        Else
            strText = objParameter.Name & " = " & objParameter.Value
        End If
        
        Dim objNode As Node
        Set objNode = tvParameters.Nodes.Add(objRoot.Index, tvwChild, Text:=strText)
        
        If (objParameter.Parameters.Count > 0) Then
            DisplayParameters objParameter.Parameters, objNode
        End If
    Next
End Sub

Private Sub UserControl_Resize()
    fraParameters.Move 0, 0, ScaleWidth, ScaleHeight
    txtFilename.Width = fraParameters.Width - txtFilename.Left - cmdFile.Width - 180
    cmdFile.Left = ScaleWidth - cmdFile.Width - 100
    tvParameters.Move 100, 720, PosInt(ScaleWidth - 200), PosInt(ScaleHeight - 820)
End Sub
