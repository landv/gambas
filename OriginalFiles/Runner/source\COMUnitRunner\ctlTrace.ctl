VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.UserControl ctlTrace 
   ClientHeight    =   3660
   ClientLeft      =   0
   ClientTop       =   0
   ClientWidth     =   6270
   ScaleHeight     =   3660
   ScaleWidth      =   6270
   Begin MSComctlLib.ImageList imlIcons 
      Left            =   4680
      Top             =   1080
      _ExtentX        =   1005
      _ExtentY        =   1005
      BackColor       =   -2147483643
      ImageWidth      =   16
      ImageHeight     =   16
      MaskColor       =   12632256
      _Version        =   393216
      BeginProperty Images {2C247F25-8591-11D1-B16A-00C0F0283628} 
         NumListImages   =   4
         BeginProperty ListImage1 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "ctlTrace.ctx":0000
            Key             =   ""
         EndProperty
         BeginProperty ListImage2 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "ctlTrace.ctx":0352
            Key             =   ""
         EndProperty
         BeginProperty ListImage3 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "ctlTrace.ctx":06A4
            Key             =   ""
         EndProperty
         BeginProperty ListImage4 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "ctlTrace.ctx":09F6
            Key             =   ""
         EndProperty
      EndProperty
   End
   Begin MSComctlLib.TreeView tvTrace 
      Height          =   3375
      Left            =   0
      TabIndex        =   0
      Top             =   0
      Width           =   4455
      _ExtentX        =   7858
      _ExtentY        =   5953
      _Version        =   393217
      LabelEdit       =   1
      Style           =   7
      ImageList       =   "imlIcons"
      Appearance      =   1
   End
End
Attribute VB_Name = "ctlTrace"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

' The UnitTrace control is used to trace the progress and status of the executing test cases.
' The control displays all trace messages associated with each executing test case and groups
' them in a tree.  Test cases that execute successfully are identified with a green ball.
' Test cases that contain failures receive a yellow ball, and test cases with errors receive
' a red ball.  The UnitTrace control is useful if detailed information on the running tests is
' required.

' Constants
Const g_lOKImage = 1
Const g_lFailureImage = 2
Const g_lErrorImage = 3

' Member variables
Private WithEvents m_oTestResult As TestResult
Attribute m_oTestResult.VB_VarHelpID = -1
Private m_oTestCase As ITestCase
Private m_objRoot As Node

' Class destructor
Private Sub Class_Terminate()
    Set m_oTestResult = Nothing
    Set m_oTestCase = Nothing
End Sub

' Set the TestResult to be used for capturing the results of running the tests.
Public Property Set TestResult(oTestResult As TestResult)
    Set m_oTestResult = oTestResult
End Property

Private Sub UserControl_Resize()
    tvTrace.Width = ScaleWidth
    tvTrace.Height = ScaleHeight
End Sub

' Resets the UnitTrace control clearing the results of the last test run.
Public Sub Reset()
    tvTrace.Nodes.Clear
    
    Set m_objRoot = tvTrace.Nodes.Add(Text:="Tests", Image:=4)
    m_objRoot.Expanded = True
End Sub

Private Sub m_oTestResult_AfterStartTest(oTestCase As ITestCase)
    Dim oNode As Node
    Set oNode = tvTrace.Nodes.Add(m_objRoot.Index, tvwChild, TestName(oTestCase), TestName(oTestCase), g_lOKImage)
    tvTrace.Nodes.Add oNode.Index, tvwChild, , "Start Test Case", g_lOKImage
    Set m_oTestCase = oTestCase
    DoEvents
End Sub

Private Sub m_oTestResult_AfterEndTest()
    Dim oNode As Node
    Set oNode = tvTrace.Nodes(TestName(m_oTestCase))
    tvTrace.Nodes.Add oNode.Index, tvwChild, , "End Test Case", g_lOKImage
    
    ' change top node icon depending on trace events
    Dim oChildNode As Node, lImage As Long
    Set oChildNode = oNode.Child
    While Not (oChildNode Is Nothing)
        If (CLng(oChildNode.Image) > lImage) Then
            lImage = CLng(oChildNode.Image)
        End If
        Set oChildNode = oChildNode.Next
    Wend
    oNode.Image = lImage
    DoEvents
End Sub

Private Sub m_oTestResult_AfterAddError(oError As TestError)
    Dim oNode As Node
    Set oNode = tvTrace.Nodes(TestName(oError.TestCase))
    tvTrace.Nodes.Add oNode.Index, tvwChild, , "Error: " & GetErrorMsg(oError), g_lErrorImage
    DoEvents
End Sub

Private Sub m_oTestResult_AfterAddFailure(oError As TestError)
    Dim oNode As Node
    Set oNode = tvTrace.Nodes(TestName(oError.TestCase))
    tvTrace.Nodes.Add oNode.Index, tvwChild, , "Failure: " & oError.Description, g_lFailureImage
    DoEvents
End Sub

Private Sub m_oTestResult_AfterAddTrace(sMessage As String)
    Dim oNode As Node
    Set oNode = tvTrace.Nodes(TestName(m_oTestCase))
    tvTrace.Nodes.Add oNode.Index, tvwChild, , "Trace: " & sMessage, g_lOKImage
    DoEvents
End Sub

Private Function GetErrorMsg(oError As TestError)
    GetErrorMsg = oError.Source & " (" & Trim(oError.ErrNumber) & "): " & oError.Description
End Function

Private Function TestName(oTestCase As TestCase) As String
    If (oTestCase Is Nothing) Then
        TestName = ""
    Else
        TestName = TypeName(oTestCase.TestContainer) & "." & oTestCase.Name
    End If
End Function

