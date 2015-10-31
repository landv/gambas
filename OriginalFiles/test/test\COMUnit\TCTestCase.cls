VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "TCTestCase"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

' Interface declaration
Implements ITestContainer

' Fixture Member Variables
Private m_oTestSuite As TestSuite
Private m_oTestCase As TestCase

' Return the name of the different test case methods in this test container
Public Property Get ITestContainer_TestCaseNames() As Variant()

End Property

' Run the specified test case methods in this test container
Public Sub ITestContainer_RunTestCase(oTestCase As ITestCase, oTestResult As TestResult)
    On Error GoTo ErrorHandler
    InvokeHook Me, oTestCase.Name, INVOKE_FUNC, oTestResult
    Exit Sub
ErrorHandler:
    oTestResult.AddError Err.Number, Err.Source, Err.Description
End Sub

'Initialize the test fixture
Public Sub ITestContainer_Setup()
    Set m_oTestSuite = New TestSuite
End Sub

'Destroy the test fixture
Public Sub ITestContainer_TearDown()
    Set m_oTestSuite = Nothing
End Sub


