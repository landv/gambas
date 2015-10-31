VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "TCTestParameters"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

' Interface declaration
Implements ITestContainer

' Fixture Member Variables
Private m_oTestParameters As TestParameters

' Return the name of the different test case methods in this test container
Public Property Get ITestContainer_TestCaseNames() As Variant()
    ITestContainer_TestCaseNames = Array("testTestParameters")
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
    Set m_oTestParameters = New TestParameters
End Sub

'Destroy the test fixture
Public Sub ITestContainer_TearDown()
    Set m_oTestParameters = Nothing
End Sub

Public Sub testTestParameters(oTestResult As TestResult)
    oTestResult.AssertEqualsLong 0, m_oTestParameters.Count, "Incorrect number of TestParameters"
    
    ' add a parameter to the collection
    Dim oParameter As TestParameter
    Set oParameter = m_oTestParameters.Add("param", "value")
    oTestResult.AssertEqualsString "param", oParameter.Name
    oTestResult.AssertEqualsVariant "value", oParameter.Value
    oTestResult.AssertEqualsLong 1, m_oTestParameters.Count
    
    ' add a parameter object
    Dim oParameter2 As New TestParameter
    m_oTestParameters.AddParameter oParameter2
    oTestResult.AssertEqualsLong 2, m_oTestParameters.Count
    
    ' retrieve specific parameter
    oTestResult.AssertExists m_oTestParameters("param"), "No parameter is returned"
    
    ' iterate through collection
    For Each oParameter2 In m_oTestParameters
        oTestResult.AssertExists oParameter2
    Next
    
    ' add subparameter
    oTestResult.AssertEqualsLong 0, oParameter.Parameters.Count
    oParameter.Parameters.Add "subparam", 2
    oTestResult.AssertEqualsLong 1, oParameter.Parameters.Count
End Sub

