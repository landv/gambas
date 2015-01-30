VERSION 5.00
Object = "{BDC217C8-ED16-11CD-956C-0000C04E4C0A}#1.1#0"; "TABCTL32.OCX"
Begin VB.UserControl UnitRunner 
   ClientHeight    =   6135
   ClientLeft      =   0
   ClientTop       =   0
   ClientWidth     =   9675
   ScaleHeight     =   6135
   ScaleWidth      =   9675
   ToolboxBitmap   =   "UnitRunner.ctx":0000
   Begin TabDlg.SSTab tabMain 
      Height          =   6075
      Left            =   0
      TabIndex        =   0
      Top             =   0
      Width           =   9615
      _ExtentX        =   16960
      _ExtentY        =   10716
      _Version        =   393216
      TabHeight       =   520
      TabCaption(0)   =   "Results"
      TabPicture(0)   =   "UnitRunner.ctx":0312
      Tab(0).ControlEnabled=   -1  'True
      Tab(0).Control(0)=   "ctlSelector1"
      Tab(0).Control(0).Enabled=   0   'False
      Tab(0).Control(1)=   "ctlResults1"
      Tab(0).Control(1).Enabled=   0   'False
      Tab(0).ControlCount=   2
      TabCaption(1)   =   "Setup"
      TabPicture(1)   =   "UnitRunner.ctx":032E
      Tab(1).ControlEnabled=   0   'False
      Tab(1).Control(0)=   "ctlLog1"
      Tab(1).Control(1)=   "ctlParameter1"
      Tab(1).ControlCount=   2
      TabCaption(2)   =   "Trace"
      TabPicture(2)   =   "UnitRunner.ctx":034A
      Tab(2).ControlEnabled=   0   'False
      Tab(2).Control(0)=   "ctlTrace1"
      Tab(2).ControlCount=   1
      Begin COMUnitRunner.ctlLog ctlLog1 
         Height          =   675
         Left            =   -74940
         TabIndex        =   5
         Top             =   4440
         Width           =   9435
         _ExtentX        =   16642
         _ExtentY        =   1191
      End
      Begin COMUnitRunner.ctlParameter ctlParameter1 
         Height          =   3975
         Left            =   -74940
         TabIndex        =   4
         Top             =   420
         Width           =   9435
         _ExtentX        =   13891
         _ExtentY        =   1508
      End
      Begin COMUnitRunner.ctlTrace ctlTrace1 
         Height          =   5595
         Left            =   -74940
         TabIndex        =   3
         Top             =   420
         Width           =   9495
         _ExtentX        =   16748
         _ExtentY        =   9869
      End
      Begin COMUnitRunner.ctlResults ctlResults1 
         Height          =   5235
         Left            =   60
         TabIndex        =   2
         Top             =   780
         Width           =   9495
         _ExtentX        =   16748
         _ExtentY        =   9234
      End
      Begin COMUnitRunner.ctlSelector ctlSelector1 
         Height          =   315
         Left            =   60
         TabIndex        =   1
         Top             =   420
         Width           =   9495
         _ExtentX        =   16748
         _ExtentY        =   556
      End
   End
End
Attribute VB_Name = "UnitRunner"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Option Explicit

' Member Variables
Private m_oTestResult As TestResult
Attribute m_oTestResult.VB_VarHelpID = -1

' Run tests in background if specified
Private Sub UserControl_Show()
    If (ParseCommand("h") = "h") Then
        Parent.Hide
        Run
        Unload Parent
    End If
End Sub

' Class destructor
Private Sub UserControl_Terminate()
    Set m_oTestResult = Nothing
End Sub

' Retrieve the TestSuite generated from the test cases that have been selected by the user.
Public Property Get TestSuite() As ITest
    Set TestSuite = ctlSelector1.CreateTestSuite
End Property

' Get the TestResult to be used for capturing the results of running the tests.
Public Property Get TestResult() As TestResult
    Set TestResult = m_oTestResult
End Property

' Set the TestResult to be used for capturing the results of running the tests.
Public Property Set TestResult(oTestResult As TestResult)
    Set m_oTestResult = oTestResult
End Property

' Add a TestContainer to the UnitRunner so that the user can select test cases from it to execute.
Public Function AddTestContainer(oTestContainer As ITestContainer)
    ctlSelector1.AddTestContainer oTestContainer
End Function

' Run the test cases that have been selected by the user using the specified TestResult.
Public Sub Run(Optional oTestResult As TestResult)
    If (oTestResult Is Nothing) Then
        Set m_oTestResult = New TestResult
    Else
        Set m_oTestResult = oTestResult
    End If
    SetTestResult
    
    ' retrieve parameters
    Set m_oTestResult.Parameters = ctlParameter1.TestParameters
    
    ' create the selected TestContainer and then run it
    Dim oTest As ITest
    Set oTest = ctlSelector1.CreateTestSuite
    
    ' reset the controls to use this test
    ResetControls oTest
    
    oTest.Run m_oTestResult
    
    ctlLog1.WriteToLog m_oTestResult
End Sub

' Distribute the test result to all controls
Private Sub SetTestResult()
    Set ctlResults1.TestResult = m_oTestResult
    Set ctlTrace1.TestResult = m_oTestResult
End Sub

' Resets the UnitRunner control clearing the results of the last test run.
Public Sub Reset()
    ResetControls
End Sub

Public Property Let ParameterFile(strFile As String)
    ctlParameter1.ParameterFile = strFile
End Property

Public Property Let LogFile(strFile As String)
    ctlLog1.LogFile = strFile
End Property

Public Property Let CommandLine(strCommand As String)
    gstrCommand = strCommand
End Property

Private Sub ResetControls(Optional oTest As ITest)
    ctlResults1.Reset oTest
    ctlTrace1.Reset
End Sub

' Specify the TestContainer to be selected by default when the UnitRunner starts up.
Public Sub SetDefaultTestContainer(oTestContainer As ITestContainer)
    ctlSelector1.SetDefaultTestContainer oTestContainer
End Sub

' Resize controls on selected tab page
Private Sub tabMain_Click(PreviousTab As Integer)
    UserControl_Resize
End Sub

Private Sub UserControl_Resize()
    tabMain.Move 0, 0, ScaleWidth, ScaleHeight
    If (tabMain.Tab = 0) Then
        ctlSelector1.Move 80, tabMain.TabHeight + 140, PosInt(ScaleWidth - 160), ctlSelector1.Height
        ctlResults1.Move 80, tabMain.TabHeight + 180 + ctlSelector1.Height, PosInt(ScaleWidth - 160), PosInt(tabMain.Height - ctlSelector1.Top - ctlSelector1.Height - 140)
    ElseIf (tabMain.Tab = 1) Then
        ctlParameter1.Move ctlParameter1.Left, ctlParameter1.Top, PosInt(ScaleWidth - 160), PosInt(tabMain.Height - ctlParameter1.Top - ctlLog1.Height - 140)
        ctlLog1.Move ctlLog1.Left, ctlParameter1.Top + ctlParameter1.Height, PosInt(ScaleWidth - 160)
    ElseIf (tabMain.Tab = 2) Then
        ctlTrace1.Move 80, tabMain.TabHeight + 140, PosInt(ScaleWidth - 160), PosInt(tabMain.Height - ctlTrace1.Top - 140)
    End If
End Sub
