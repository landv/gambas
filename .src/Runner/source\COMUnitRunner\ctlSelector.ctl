VERSION 5.00
Begin VB.UserControl ctlSelector 
   ClientHeight    =   330
   ClientLeft      =   0
   ClientTop       =   0
   ClientWidth     =   8910
   ScaleHeight     =   330
   ScaleWidth      =   8910
   Begin VB.ComboBox cmbTestContainer 
      Height          =   315
      Left            =   1320
      Style           =   2  'Dropdown List
      TabIndex        =   1
      Top             =   0
      Width           =   2895
   End
   Begin VB.ComboBox cmbTestCase 
      Height          =   315
      Left            =   5400
      Style           =   2  'Dropdown List
      TabIndex        =   0
      Top             =   0
      Width           =   3495
   End
   Begin VB.Label Label3 
      Caption         =   "Test Container:"
      Height          =   255
      Left            =   0
      TabIndex        =   3
      Top             =   75
      Width           =   1215
   End
   Begin VB.Label Label4 
      Caption         =   "Test Case:"
      Height          =   255
      Left            =   4440
      TabIndex        =   2
      Top             =   75
      Width           =   975
   End
End
Attribute VB_Name = "ctlSelector"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

' Constants
Private Const ALL_TEST_CASES = "All Test Cases"
Private Const ALL_TEST_CONTAINERS = "All Test Containers"

' Member Variables
Private m_colTestContainers As Collection

' Class constructor
Private Sub UserControl_Initialize()
    Set m_colTestContainers = New Collection
End Sub

' Class destructor
Private Sub UserControl_Terminate()
    Set m_colTestContainers = Nothing
End Sub

' Add a TestContainer to the UnitRunner so that the user can select test cases from it to execute.
Public Sub AddTestContainer(oTestContainer As ITestContainer)
    Dim sTestContainerName As String
    sTestContainerName = TypeName(oTestContainer)
    
    If (ExistsTestContainer(sTestContainerName)) Then
        MsgBox "TestContainer " & sTestContainerName & " has already been added."
    Else
        m_colTestContainers.Add oTestContainer, sTestContainerName
        cmbTestContainer.AddItem sTestContainerName
    
        If (cmbTestContainer.ListCount = 1) Then
            cmbTestContainer.Text = sTestContainerName
        ElseIf (cmbTestContainer.ListCount = 2) Then
            ' multiple test containers -- therefore allow all test containers to be run
            cmbTestContainer.AddItem ALL_TEST_CONTAINERS, 0
            cmbTestContainer.ListIndex = cmbTestContainer.NewIndex ' select it
        End If
    End If
End Sub

' Check if the TestContainer has already been added
Private Function ExistsTestContainer(sTestContainerName As String) As Boolean
    On Error Resume Next
    m_colTestContainers.Item sTestContainerName
    ExistsTestContainer = (Err.Number = 0)
End Function

' Retrieve the TestSuite generated from the test cases that have been selected by the user.
Public Function CreateTestSuite() As ITest
    Dim oTestSuite As TestSuite
    Set oTestSuite = New TestSuite

    If (cmbTestContainer.ListIndex <> -1) Then
        Dim sTestContainer As String
        sTestContainer = cmbTestContainer.List(cmbTestContainer.ListIndex)
        
        Dim oTestContainer As ITestContainer
        If (sTestContainer = ALL_TEST_CONTAINERS) Then
            For Each oTestContainer In m_colTestContainers
                oTestSuite.AddAllTestCases oTestContainer
            Next
        Else
            Set oTestContainer = m_colTestContainers(sTestContainer)
            
            Dim sTestCase As String
            sTestCase = cmbTestCase.List(cmbTestCase.ListIndex)
            
            If (sTestCase = ALL_TEST_CASES) Then
                oTestSuite.AddAllTestCases oTestContainer
            Else
                oTestSuite.AddNewTestCase sTestCase, oTestContainer
            End If
        End If
    End If
    Set CreateTestSuite = oTestSuite
End Function

' Specify the TestContainer to be selected by default when the UnitRunner starts up.
Public Sub SetDefaultTestContainer(oTestContainer As ITestContainer)
    On Error GoTo ErrorHandler
    cmbTestContainer.Text = TypeName(oTestContainer)
    Exit Sub
ErrorHandler:
    AddTestContainer oTestContainer
    cmbTestContainer.Text = TypeName(oTestContainer)
End Sub

' Event Handlers
Private Sub UserControl_Resize()
    cmbTestCase.Width = PosInt(Width - cmbTestCase.Left)
End Sub

' Update TestCase combo depending on selected TestContainer
Private Sub cmbTestContainer_Click()
    Dim sTestContainer As String
    sTestContainer = cmbTestContainer.List(cmbTestContainer.ListIndex)
        
    cmbTestCase.Clear
    cmbTestCase.AddItem ALL_TEST_CASES
    If (sTestContainer <> ALL_TEST_CONTAINERS) Then
        Dim oTestContainer As ITestContainer
        Set oTestContainer = m_colTestContainers.Item(sTestContainer)
        
        Dim arrTestCases() As Variant
        arrTestCases = oTestContainer.TestCaseNames
        
        Dim i As Integer
        For i = LBound(arrTestCases) To UBound(arrTestCases)
            cmbTestCase.AddItem arrTestCases(i)
        Next
    End If
    cmbTestCase.ListIndex = 0
End Sub

