VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "TCTestSuite"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

' Interface declaration
Implements ITestContainer

' Fixture Member Variables
Private m_oTestSuite As TestSuite
Private m_oTestCase As ITestCase

' Return the name of the different test case methods in this test container
Public Property Get ITestContainer_TestCaseNames() As Variant()
    ITestContainer_TestCaseNames = Array("testNewTestCase", "testAddNewTestCase", _
        "testAddAllTestCases", "testRun", "testFAILSetup", "testFAILTearDown")
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
    Set m_oTestCase = Nothing
End Sub

Public Sub testNewTestCase(oTestResult As TestResult)
    Set m_oTestCase = m_oTestSuite.NewTestCase("testNewTestCase", Me)
    
    oTestResult.AssertEqualsString "testNewTestCase", m_oTestCase.Name, "TestCase name is incorrect"
    oTestResult.Assert (Me Is m_oTestCase.TestContainer), "TestCase TestContainer is incorrect"
End Sub

Public Sub testAddNewTestCase(oTestResult As TestResult)
    Set m_oTestCase = m_oTestSuite.AddNewTestCase("testNewTestCase", Me)
    
    oTestResult.AssertEqualsString "testNewTestCase", m_oTestCase.Name, "TestCase name is incorrect"
    oTestResult.Assert (Me Is m_oTestCase.TestContainer), "TestCase TestContainer is incorrect"
    oTestResult.AssertEqualsLong m_oTestSuite.CountTestCases, 1, "CountTestCases is incorrect"
End Sub

Public Sub testAddAllTestCases(oTestResult As TestResult)
    m_oTestSuite.AddAllTestCases Me
    oTestResult.AssertEqualsLong UBound(ITestContainer_TestCaseNames) + 1, m_oTestSuite.CountTestCases, "CountTestCases is incorrect"
End Sub

Public Sub testRun(oTestResult As TestResult)
    m_oTestSuite.AddTest New Test1
    m_oTestSuite.AddTest New Test2
    oTestResult.AssertEqualsLong 2, m_oTestSuite.CountTestCases, "CountTestCases is incorrect"
    
    Dim oNewTestResult As New TestResult
    m_oTestSuite.Run oNewTestResult
    oTestResult.AssertEqualsLong 2, oNewTestResult.RunTests, "Number of run tests is incorrect"
    oTestResult.AssertEqualsLong 1, oNewTestResult.Failures.Count, "Number of failures is incorrect"
    oTestResult.AssertEqualsLong 0, oNewTestResult.Errors.Count, "Number of errors is incorrect"
    oTestResult.Assert Not (oNewTestResult.WasSuccessful), "Test should have succeeded"
End Sub

Public Sub testFAILSetup(oTestResult As TestResult)
    Dim oTestContainer As New testFAILSetup
    Dim oSuite As New TestSuite
    Dim oResult As New TestResult
    
    oSuite.AddAllTestCases oTestContainer
    oSuite.Run oResult
    
    oTestResult.AssertEqualsLong 1, oResult.RunTests, "Incorrect number of run tests"
    oTestResult.AssertEqualsLong 1, oResult.Errors.Count, "Incorrect number of errors"
    oTestResult.AssertEqualsLong 0, oResult.Failures.Count, "Incorrect number of failures"
End Sub

Public Sub TestFailTearDown(oTestResult As TestResult)
    Dim oTestContainer As New TestFailTearDown
    Dim oSuite As New TestSuite
    Dim oResult As New TestResult
    
    oSuite.AddAllTestCases oTestContainer
    oSuite.Run oResult
    
    oTestResult.AssertEqualsLong 1, oResult.RunTests, "Incorrect number of run tests"
    oTestResult.AssertEqualsLong 1, oResult.Errors.Count, "Incorrect number of errors"
    oTestResult.AssertEqualsLong 1, oResult.Failures.Count, "Incorrect number of failures"
End Sub
