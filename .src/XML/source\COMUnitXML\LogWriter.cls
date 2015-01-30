VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "LogWriter"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Option Explicit

Public Function WriteToFile(objResult As TestResult, strFilename As String) As DOMDocument
    Dim objDocument As DOMDocument30
    Set objDocument = WriteToDocument(objResult)
    
    objDocument.save strFilename
    
    Set WriteToFile = objDocument
End Function

Public Function WriteToDocument(objResult As TestResult) As DOMDocument
    Dim objDocument As DOMDocument30
    Set objDocument = CreateXMLDocument
    
    Dim objRoot As IXMLDOMElement
    Set objRoot = objDocument.CreateElement("log")
    objRoot.appendChild CreateElement(objDocument, "timestamp", CStr(Now))
    objRoot.appendChild CreateElement(objDocument, "title", App.Title)
    objRoot.appendChild CreateElement(objDocument, "tests", objResult.RunTests)

    If Not (objResult.Parameters Is Nothing) Then
        Dim objWriter As New ParameterWriter
        objRoot.appendChild objWriter.WriteToNode(objDocument, objResult.Parameters)
    End If
    
    objRoot.appendChild CreateErrorsNode(objDocument, objResult.Errors)
    objRoot.appendChild CreateFailuresNode(objDocument, objResult.Failures)
    objDocument.documentElement.appendChild objRoot
    
    Set WriteToDocument = objDocument
End Function

Private Function CreateErrorsNode(objDocument As DOMDocument, colErrors As TestErrors) As IXMLDOMElement
    Dim objRoot As IXMLDOMElement, objNode As IXMLDOMElement
    
    Set objRoot = objDocument.CreateElement("errors")
    
    Dim objError As TestError
    For Each objError In colErrors
        Set objNode = objDocument.CreateElement("error")
        objNode.appendChild CreateElement(objDocument, "number", objError.ErrNumber)
        objNode.appendChild CreateElement(objDocument, "source", objError.Source)
        objNode.appendChild CreateElement(objDocument, "description", objError.Description)
        
        If Not (objError.TestCase Is Nothing) Then
            objNode.appendChild CreateElement(objDocument, "testcase", objError.TestCase.Name)
        End If
        
        objRoot.appendChild objNode
    Next
    
    Set CreateErrorsNode = objRoot
End Function

Private Function CreateFailuresNode(objDocument As DOMDocument, colFailures As TestErrors) As IXMLDOMElement
    Dim objRoot As IXMLDOMElement, objNode As IXMLDOMElement
    
    Set objRoot = objDocument.CreateElement("failures")
    
    Dim objFailure As TestError
    For Each objFailure In colFailures
        Set objNode = objDocument.CreateElement("failure")
        objNode.appendChild CreateElement(objDocument, "description", objFailure.Description)
        
        If Not (objFailure.TestCase Is Nothing) Then
            objNode.appendChild CreateElement(objDocument, "testcase", objFailure.TestCase.Name)
        End If
        
        objRoot.appendChild objNode
    Next
    
    Set CreateFailuresNode = objRoot
End Function
