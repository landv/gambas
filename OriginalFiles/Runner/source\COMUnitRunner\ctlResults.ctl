VERSION 5.00
Object = "{5E9E78A0-531B-11CF-91F6-C2863C385E30}#1.0#0"; "MSFLXGRD.OCX"
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.UserControl ctlResults 
   ClientHeight    =   3270
   ClientLeft      =   0
   ClientTop       =   0
   ClientWidth     =   8955
   ScaleHeight     =   3270
   ScaleWidth      =   8955
   Begin MSComctlLib.StatusBar sbStatus 
      Align           =   2  'Align Bottom
      Height          =   375
      Left            =   0
      TabIndex        =   0
      Top             =   2895
      Width           =   8955
      _ExtentX        =   15796
      _ExtentY        =   661
      _Version        =   393216
      BeginProperty Panels {8E3867A5-8586-11D1-B16A-00C0F0283628} 
         NumPanels       =   4
         BeginProperty Panel1 {8E3867AB-8586-11D1-B16A-00C0F0283628} 
            AutoSize        =   1
            Object.Width           =   8043
            Text            =   "Test:"
            TextSave        =   "Test:"
         EndProperty
         BeginProperty Panel2 {8E3867AB-8586-11D1-B16A-00C0F0283628} 
            Text            =   "Runs:"
            TextSave        =   "Runs:"
         EndProperty
         BeginProperty Panel3 {8E3867AB-8586-11D1-B16A-00C0F0283628} 
            Text            =   "Errors:"
            TextSave        =   "Errors:"
         EndProperty
         BeginProperty Panel4 {8E3867AB-8586-11D1-B16A-00C0F0283628} 
            Text            =   "Failures:"
            TextSave        =   "Failures:"
         EndProperty
      EndProperty
   End
   Begin MSFlexGridLib.MSFlexGrid grdFailures 
      Height          =   2175
      Left            =   0
      TabIndex        =   1
      Top             =   720
      Width           =   8895
      _ExtentX        =   15690
      _ExtentY        =   3836
      _Version        =   393216
      ScrollTrack     =   -1  'True
      SelectionMode   =   1
      AllowUserResizing=   1
   End
   Begin MSComctlLib.ProgressBar pbProgress 
      Height          =   375
      Left            =   1320
      TabIndex        =   2
      Top             =   0
      Width           =   7575
      _ExtentX        =   13361
      _ExtentY        =   661
      _Version        =   393216
      Appearance      =   1
   End
   Begin VB.Label Label2 
      Caption         =   "Errors and Failures:"
      Height          =   255
      Left            =   0
      TabIndex        =   4
      Top             =   480
      Width           =   1455
   End
   Begin VB.Label Label1 
      Caption         =   "Progress:"
      Height          =   255
      Left            =   0
      TabIndex        =   3
      Top             =   105
      Width           =   855
   End
End
Attribute VB_Name = "ctlResults"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

' Member Variables
Private WithEvents m_oTestResult As TestResult
Attribute m_oTestResult.VB_VarHelpID = -1

' Class constructor
Private Sub UserControl_Initialize()
    Reset
End Sub

' Class destructor
Private Sub UserControl_Terminate()
    Set m_oTestResult = Nothing
End Sub

Public Property Set TestResult(oTestResult As TestResult)
    Set m_oTestResult = oTestResult
End Property

' Event Handling for TestResult
Private Sub m_oTestResult_AfterAddError(oError As TestError)
    UpdateErrorsCount
    AddError oError
    DoEvents
End Sub

Private Sub m_oTestResult_AfterAddFailure(oError As TestError)
    UpdateFailuresCount
    AddFailure oError
    DoEvents
End Sub

Private Sub m_oTestResult_AfterEndTest()
    UpdateRunsCount
    UpdateProgressBar
    DoEvents
    
    If (m_oTestResult.WasSuccessful) Then
        Call SendMessageLong(pbProgress.hwnd, PBM_SETBARCOLOR, 0&, RGB(0, 255, 0))
    Else
        Call SendMessageLong(pbProgress.hwnd, PBM_SETBARCOLOR, 0&, RGB(255, 0, 0))
    End If
    
    If (m_oTestResult.RunTests = pbProgress.Max) Then
        sbStatus.Panels(1).Text = "Test: "
    End If
End Sub

Private Sub m_oTestResult_AfterStartTest(oTestCase As ITestCase)
    sbStatus.Panels(1).Text = "Test Case: " & oTestCase.Name
    DoEvents
End Sub

' Reset Controls
Public Sub Reset(Optional oTest As ITest)
    ResetCounters
    ResetProgressBar oTest
    ResetFailureList
End Sub

' Reset Controls Methods
Private Sub ResetCounters()
    sbStatus.Panels(1).Text = "Test: "
    sbStatus.Panels(2).Text = "Runs: 0"
    sbStatus.Panels(3).Text = "Errors: 0"
    sbStatus.Panels(4).Text = "Failures: 0"
End Sub

Private Sub ResetProgressBar(oTest As ITest)
    Call SendMessageLong(pbProgress.hwnd, PBM_SETBARCOLOR, 0&, CLR_DEFAULT)
    pbProgress.Value = 0
    pbProgress.Max = 1
    If Not (oTest Is Nothing) Then
        pbProgress.Max = IIf(oTest.CountTestCases > 0, oTest.CountTestCases, 1)
    End If
End Sub

Private Sub ResetFailureList()
    grdFailures.Clear
    grdFailures.Rows = 1
    grdFailures.Cols = 3
    SetColWidth
    grdFailures.TextMatrix(0, 0) = "Type"
    grdFailures.TextMatrix(0, 1) = "Test"
    grdFailures.TextMatrix(0, 2) = "Description"
End Sub

' Event Handlers
Private Sub UserControl_Resize()
    pbProgress.Width = PosInt(Width - pbProgress.Left)
    grdFailures.Width = PosInt(Width - grdFailures.Left)
    
    grdFailures.Height = PosInt((Height - grdFailures.Top) - pbProgress.Height)
    SetColWidth
End Sub

Private Sub grdFailures_DblClick()
    If (grdFailures.RowSel > 0 And grdFailures.ColSel > 0) Then
        MsgBox "Test: " & grdFailures.TextMatrix(grdFailures.RowSel, 1) & vbCrLf & _
            "Description: " & grdFailures.TextMatrix(grdFailures.RowSel, 2)
    End If
End Sub

Private Sub SetColWidth()
    If (grdFailures.Cols = 3) Then
        grdFailures.Redraw = False
        grdFailures.ColWidth(0) = grdFailures.Width * 0.1
        grdFailures.ColWidth(1) = grdFailures.Width * 0.3
        grdFailures.ColWidth(2) = PosInt(grdFailures.Width - grdFailures.ColWidth(1) - grdFailures.ColWidth(0) - 98)
        grdFailures.Redraw = True
    End If
End Sub

' Update Display Methods
Private Sub UpdateProgressBar()
    pbProgress.Value = m_oTestResult.RunTests
    pbProgress.Refresh
End Sub

Private Sub UpdateRunsCount()
    sbStatus.Panels(2).Text = "Runs: " & CStr(m_oTestResult.RunTests) & " / " & CStr(pbProgress.Max)
End Sub

Private Sub UpdateErrorsCount()
    sbStatus.Panels(3).Text = "Errors: " & CStr(m_oTestResult.Errors.Count)
End Sub

Private Sub UpdateFailuresCount()
    sbStatus.Panels(4).Text = "Failures: " & CStr(m_oTestResult.Failures.Count)
End Sub

'Add error to list
Private Sub AddError(oError As TestError)
    Dim lCurrentRow As Long
    
    grdFailures.Rows = grdFailures.Rows + 1
    lCurrentRow = grdFailures.Rows - 1
    grdFailures.TextMatrix(lCurrentRow, 0) = "Error"
    grdFailures.TextMatrix(lCurrentRow, 1) = TestName(oError)
    grdFailures.TextMatrix(lCurrentRow, 2) = oError.Source & " (" & Trim(oError.ErrNumber) & "): " & oError.Description
End Sub

'Add failure to list
Private Sub AddFailure(oError As TestError)
    Dim lCurrentRow As Long
    
    grdFailures.Rows = grdFailures.Rows + 1
    lCurrentRow = grdFailures.Rows - 1
    grdFailures.TextMatrix(lCurrentRow, 0) = "Failure"
    grdFailures.TextMatrix(lCurrentRow, 1) = TestName(oError)
    grdFailures.TextMatrix(lCurrentRow, 2) = oError.Description
End Sub

Private Function TestName(oError As TestError) As String
    If (oError.TestCase Is Nothing) Then
        TestName = ""
    Else
        TestName = TypeName(oError.TestCase.TestContainer) & "." & oError.TestCase.Name
    End If
End Function
