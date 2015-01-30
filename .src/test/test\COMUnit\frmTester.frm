VERSION 5.00
Begin VB.Form frmTester 
   Caption         =   "Form1"
   ClientHeight    =   5430
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   9675
   Icon            =   "frmTester.frx":0000
   LinkTopic       =   "Form1"
   ScaleHeight     =   5430
   ScaleWidth      =   9675
   StartUpPosition =   3  'Windows Default
   Begin VB.TextBox txtTestCase 
      Height          =   285
      Left            =   1080
      Locked          =   -1  'True
      TabIndex        =   4
      Top             =   120
      Width           =   8535
   End
   Begin VB.CommandButton btnRunTest 
      Caption         =   "&Run Tests"
      Default         =   -1  'True
      Height          =   495
      Left            =   6960
      TabIndex        =   0
      Top             =   4800
      Width           =   1215
   End
   Begin VB.TextBox txtResults 
      Height          =   4215
      Left            =   120
      MultiLine       =   -1  'True
      ScrollBars      =   2  'Vertical
      TabIndex        =   2
      Top             =   480
      Width           =   9495
   End
   Begin VB.CommandButton btnClose 
      Cancel          =   -1  'True
      Caption         =   "&Close"
      Height          =   495
      Left            =   8400
      TabIndex        =   1
      Top             =   4800
      Width           =   1215
   End
   Begin VB.Label Label1 
      Caption         =   "Test Case:"
      Height          =   255
      Left            =   120
      TabIndex        =   3
      Top             =   120
      Width           =   1215
   End
End
Attribute VB_Name = "frmTester"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

' Member variables
Private WithEvents m_oTestResult As TestResult
Attribute m_oTestResult.VB_VarHelpID = -1
Private m_oTestSuite As TestSuite

Private Sub btnClose_Click()
    Unload Me
End Sub

Private Sub btnRunTest_Click()
    txtResults.Text = ""

    Set m_oTestSuite = New TestSuite
    Set m_oTestResult = New TestResult

    m_oTestSuite.AddAllTestCases New TCTestSuite
    m_oTestSuite.AddAllTestCases New TCTestResult
    m_oTestSuite.AddAllTestCases New TCTestParameters

'    Dim oTCTestCase As New TCTestCase
'    Dim oTCTestErrors As New TCTestErrors

    m_oTestSuite.Run m_oTestResult

End Sub

' Event Handling for TestResult
Private Sub m_oTestResult_AfterAddError(oError As TestError)
    txtResults.Text = txtResults.Text & GetTestCaseName(oError.TestCase) & ": " & oError.Description & vbCrLf
    DoEvents
End Sub

Private Sub m_oTestResult_AfterAddFailure(oError As TestError)
    txtResults.Text = txtResults.Text & GetTestCaseName(oError.TestCase) & ": " & oError.Description & vbCrLf
    DoEvents
End Sub

Private Sub m_oTestResult_AfterStartTest(oTestCase As ITestCase)
    txtTestCase.Text = GetTestCaseName(oTestCase)
    DoEvents
End Sub

Private Sub m_oTestResult_AfterEndTest()
    If (m_oTestResult.RunTests = m_oTestSuite.CountTestCases) Then
        If (m_oTestResult.WasSuccessful) Then
            MsgBox "All Tests Succeeded"
        Else
            MsgBox "Tests Complete.  " & CStr(m_oTestResult.Failures.Count) & " Failures, and " & CStr(m_oTestResult.Errors.Count) & " Errors were encountered.", vbExclamation
        End If
    End If
    txtTestCase.Text = ""
End Sub

Private Function GetTestCaseName(oTestCase As ITestCase)
    GetTestCaseName = TypeName(oTestCase.TestContainer) & ": " & oTestCase.Name
End Function
