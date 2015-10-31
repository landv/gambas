VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "TCTestResult"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

' Interface declaration
Implements ITestContainer

' Fixture Member Variables
Private WithEvents m_oTestResult As TestResult
Attribute m_oTestResult.VB_VarHelpID = -1
Private m_sEvent As String
Private m_sEventData As String

' Return TestFixture for the TestCase
Public Property Get ITestContainer_TestCaseNames() As Variant()
    ITestContainer_TestCaseNames = Array("testAddErrors", "testAddFailures", "testStartTest", "testEndTest", _
        "testAssert", "testAssertEqualsString", "testAssertEqualsLong", "testAssertEqualsDouble", _
        "testAssertEqualsVariant", "testAssertExists", "testAssertNotEmpty", "testAssertNotNull", _
        "testAddTrace", "testTestParameters", "testAssertError")
End Property

' Run the specified test case methods in this test container
Public Sub ITestContainer_RunTestCase(oTestCase As ITestCase, oTestResult As TestResult)
    On Error GoTo ErrorHandler
    InvokeHook Me, oTestCase.Name, INVOKE_FUNC, oTestResult
    Exit Sub
ErrorHandler:
    oTestResult.AddError Err.Number, Err.Source, Err.Description
End Sub

'Initialize the fixture
Public Sub ITestContainer_Setup()
    Set m_oTestResult = New TestResult
    m_sEvent = ""
    m_sEventData = ""
End Sub

'Destroy the fixture
Public Sub ITestContainer_TearDown()
    Set m_oTestResult = Nothing
End Sub

Public Sub testAddErrors(oTestResult As TestResult)

    m_oTestResult.AddError 10, "test1", "description1"
    m_oTestResult.AddError 11, "test2", "description2"
    
    oTestResult.AssertEqualsLong 2, m_oTestResult.Errors.Count, "Incorrect number of TestErrors"
    
    oTestResult.AssertEqualsLong 10, m_oTestResult.Errors(1).ErrNumber
    oTestResult.AssertEqualsString "test1", m_oTestResult.Errors(1).Source
    oTestResult.AssertEqualsString "description1", m_oTestResult.Errors(1).Description
    
    oTestResult.AssertEqualsLong 11, m_oTestResult.Errors(2).ErrNumber
    oTestResult.AssertEqualsString "test2", m_oTestResult.Errors(2).Source
    oTestResult.AssertEqualsString "description2", m_oTestResult.Errors(2).Description
    
    ' test that event was sent
    oTestResult.AssertEqualsString "AfterAddError", m_sEvent, "Incorrect event received."
End Sub

Public Sub testAddFailures(oTestResult As TestResult)
    
    m_oTestResult.AddFailure "description1"
    m_oTestResult.AddFailure "description2"
    
    oTestResult.AssertEqualsLong 2, m_oTestResult.Failures.Count, "Incorrect number of failures"
    
    oTestResult.AssertEqualsString "description1", m_oTestResult.Failures(1).Description
    oTestResult.AssertEqualsString "description2", m_oTestResult.Failures(2).Description
    
    ' test that event was sent
    oTestResult.AssertEqualsString "AfterAddFailure", m_sEvent, "Incorrect event received."
End Sub

Public Sub testStartTest(oTestResult As TestResult)

    m_oTestResult.StartTest New ITestCase
    
    ' test that event was sent
    oTestResult.AssertEqualsString "AfterStartTest", m_sEvent, "Incorrect event received."
End Sub

Public Sub testEndTest(oTestResult As TestResult)
    m_oTestResult.EndTest
    
    ' test that event was sent
    oTestResult.AssertEqualsString "AfterEndTest", m_sEvent, "Incorrect event received."
End Sub


Public Sub testAssert(oTestResult As TestResult)
    m_oTestResult.Assert True
    oTestResult.AssertEqualsLong 0, m_oTestResult.Failures.Count
    
    m_oTestResult.Assert False
    oTestResult.AssertEqualsLong 1, m_oTestResult.Failures.Count
End Sub

Public Sub testAssertEqualsString(oTestResult As TestResult)
    m_oTestResult.AssertEqualsString "hello", "hello"
    oTestResult.AssertEqualsLong 0, m_oTestResult.Failures.Count
    
    m_oTestResult.AssertEqualsString "hello", "goodbye"
    oTestResult.AssertEqualsLong 1, m_oTestResult.Failures.Count
End Sub

Public Sub testAssertEqualsLong(oTestResult As TestResult)
    m_oTestResult.AssertEqualsLong 1000, 1000
    oTestResult.AssertEqualsLong 0, m_oTestResult.Failures.Count
    
    m_oTestResult.AssertEqualsLong 1000, 1001
    oTestResult.AssertEqualsLong 1, m_oTestResult.Failures.Count
End Sub

Public Sub testAssertEqualsDouble(oTestResult As TestResult)
    m_oTestResult.AssertEqualsDouble 1.1, 1.1, 0.01
    oTestResult.AssertEqualsLong 0, m_oTestResult.Failures.Count
    
    m_oTestResult.AssertEqualsDouble 1.1, 1.2, 0.01
    oTestResult.AssertEqualsLong 1, m_oTestResult.Failures.Count
End Sub

Public Sub testAssertEqualsVariant(oTestResult As TestResult)
    Dim t1, t2
    t1 = "hello"
    t2 = "hello"
    
    m_oTestResult.AssertEqualsVariant t1, t2
    oTestResult.AssertEqualsLong 0, m_oTestResult.Failures.Count
    
    t2 = 50
    m_oTestResult.AssertEqualsVariant t1, t2
    oTestResult.AssertEqualsLong 1, m_oTestResult.Failures.Count
End Sub

Public Sub testAssertExists(oTestResult As TestResult)
    Dim t1 As Object, t2 As Object
    Set t1 = New Collection
    
    m_oTestResult.AssertExists t1
    oTestResult.AssertEqualsLong 0, m_oTestResult.Failures.Count
    
    m_oTestResult.AssertExists t2
    oTestResult.AssertEqualsLong 1, m_oTestResult.Failures.Count
End Sub

Public Sub testAssertNotEmpty(oTestResult As TestResult)
    Dim t1, t2
    t1 = "hello"
    
    m_oTestResult.AssertNotEmpty t1
    oTestResult.AssertEqualsLong 0, m_oTestResult.Failures.Count

    m_oTestResult.AssertNotEmpty t2
    oTestResult.AssertEqualsLong 1, m_oTestResult.Failures.Count
End Sub

Public Sub testAssertNotNull(oTestResult As TestResult)
    Dim t1
    t1 = "hello"
    
    m_oTestResult.AssertNotNull t1
    oTestResult.AssertEqualsLong 0, m_oTestResult.Failures.Count

    t1 = Null
    m_oTestResult.AssertNotNull t1
    oTestResult.AssertEqualsLong 1, m_oTestResult.Failures.Count
End Sub

Public Sub testAddTrace(oTestResult As TestResult)
    m_oTestResult.AddTrace "Trace test"
    oTestResult.AssertEqualsString "AfterAddTrace", m_sEvent
    oTestResult.AssertEqualsString "Trace test", m_sEventData
End Sub

Public Sub testTestParameters(oTestResult As TestResult)
    
    oTestResult.Assert m_oTestResult.Parameters Is Nothing
    
    Dim colParameters As New TestParameters
    Set m_oTestResult.Parameters = colParameters
    oTestResult.AssertExists m_oTestResult.Parameters
End Sub

Public Sub testAssertError(oTestResult As TestResult)
    
    Dim lError As Long
    lError = vbObjectError + 1
    
    On Error Resume Next
    Err.Raise lError
    
    m_oTestResult.AssertEqualsError Err
    oTestResult.AssertEqualsLong 0, m_oTestResult.Failures.Count
    
    m_oTestResult.AssertEqualsError Err, lError
    oTestResult.AssertEqualsLong 0, m_oTestResult.Failures.Count
    
    m_oTestResult.AssertEqualsError Err, lError + 1
    oTestResult.AssertEqualsLong 1, m_oTestResult.Failures.Count
    
    Err.Clear
    m_oTestResult.AssertEqualsError Err
    oTestResult.AssertEqualsLong 2, m_oTestResult.Failures.Count
End Sub

' Test Event Handling for TestResult
Private Sub m_oTestResult_AfterAddError(oError As TestError)
    m_sEvent = "AfterAddError"
End Sub

Private Sub m_oTestResult_AfterAddFailure(oError As TestError)
    m_sEvent = "AfterAddFailure"
End Sub

Private Sub m_oTestResult_AfterStartTest(oTestCase As ITestCase)
    m_sEvent = "AfterStartTest"
End Sub

Private Sub m_oTestResult_AfterEndTest()
    m_sEvent = "AfterEndTest"
End Sub

Private Sub m_oTestResult_AfterAddTrace(sMessage As String)
    m_sEvent = "AfterAddTrace"
    m_sEventData = sMessage
End Sub

